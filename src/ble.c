/*
 * ble.c
 *  Handles all the bluetooth related events.
 *  Checks for indication flags and sends the required information to the EFR connect app.
 *  Created on: Sep 30, 2021
 *      Author: mich1576
 */

#include <stdio.h>
#include "stdbool.h"
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "app_assert.h"
#include "ble.h"
#include "i2c.h"
#include "lcd.h"
#include "math.h"
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define SCAN_PASSIVE                  0                                         //set the macro to 0 for passive scanning
#define SCAN_INTERVAL                 80                                        //50ms (50*1.6)
#define SCAN_WINDOW                   40                                        //25ms (25*1.6)

// connection parameters
#define CONN_INTERVAL_MIN             60                                        // min. connection. interval 75/ 1.25 = 60 hex 3C
#define CONN_INTERVAL_MAX             60                                        // max. connection. interval 75/ 1.25 = 60 hex 3C
#define CONN_RESPONDER_LATENCY        0x03                                      // latency
#define CONN_TIMEOUT                  0x50                                      // timeout
#define CONN_MIN_CE_LENGTH            250                                       // min ce length
#define CONN_MAX_CE_LENGTH            250                                       // max ce length

sl_bt_msg_t *evt;                                                               //evt struct that stores data and header.
int temp;                                                                     //stores the value of temperature recorded from Si7021
int temp_client;                                                                //stores the temperature value received from server to display on client display
// BLE private data
ble_data_struct_t ble_data;

// function that returns a pointer to the
// BLE private data
ble_data_struct_t* getBleDataPtr() {
  return (&ble_data);
} // getBleDataPtr()


//******************************************GATTFLOAT32TOINT*************************************//
// Original code from Dan Walkes. I (Sluiter) fixed a sign extension bug with the mantissa.
// convert IEEE-11073 32-bit float to integer
int32_t gattFloat32ToInt(const uint8_t *value_start_little_endian)
{
    uint8_t signByte = 0;
    int32_t mantissa;
    // data format pointed at by value_start_little_endian is:
    // [0] = contains the flags byte
    // [3][2][1] = mantissa (24-bit 2’s complement)
    // [4] = exponent (8-bit 2’s complement)
    int8_t exponent = (int8_t)value_start_little_endian[4];
    // sign extend the mantissa value if the mantissa is negative
    if (value_start_little_endian[3] & 0x80) { // msb of [3] is the sign of the mantissa
        signByte = 0xFF;
    }
    mantissa = (int32_t) (value_start_little_endian[1] << 0) |
        (value_start_little_endian[2] << 8) |
        (value_start_little_endian[3] << 16) |
        (signByte << 24) ;
    // value = 10^exponent * mantissa, pow() returns a double type
    return (int32_t) (pow(10, exponent) * mantissa);
} // gattFloat32ToInt
//***********************************************************************************************//

// Parse advertisements looking for advertised Health Thermometer service
static uint8_t find_service_in_advertisement(uint8_t *data, uint8_t len)
{
  uint8_t ad_field_length;
  uint8_t ad_field_type;
  uint8_t i = 0;
  // Parse advertisement packet
  while (i < len) {
    ad_field_length = data[i];
    ad_field_type = data[i + 1];
    // Partial ($02) or complete ($03) list of 16-bit UUIDs
    if (ad_field_type == 0x02 || ad_field_type == 0x03) {
      // compare UUID to Health Thermometer service UUID
      if (memcmp(&data[i + 2], thermo_service, 2) == 0) {
        return 1;
      }
    }
    // advance to the next AD struct
    i = i + ad_field_length + 1;
  }
  return 0;
}

/**************************************************************************//**
 * Temperature Measurement characteristic indication confirmed.
 *****************************************************************************/
SL_WEAK void sl_bt_ht_temperature_measurement_indication_confirmed_cb(uint8_t connection)
{
  (void)connection;
}

void handle_ble_event(sl_bt_msg_t *evt) {

  sl_status_t retstat;                                                          //stores the return status of different bluetooth API functions
  uint8_t *char_value;

  switch (SL_BT_MSG_ID(evt->header)) {
  // ******************************************************
  // Events common to both Servers and Clients
  // ******************************************************
  // --------------------------------------------------------
  // This event indicates the device has started and the radio is ready.
  // Do not call any stack API commands before receiving this boot event!
  // Including starting BT stack soft timers!
  // --------------------------------------------------------
  // ATTRIBUTION NOTE:Some cases are handled in a manner the "SOC THERMOMETER" example handles them
      case sl_bt_evt_system_boot_id:
        // handle boot event

#if DEVICE_IS_BLE_SERVER
        displayInit();

        displayPrintf(DISPLAY_ROW_NAME, "%s", BLE_DEVICE_TYPE_STRING);              //SERVER
        displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A6");                                //ASSIGNMENT NUMBER

        //ID extracted from address
        retstat = sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);
        app_assert_status(retstat);

        // display address on LCD
        displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                                         ble_data.myAddress.addr[5],
                                         ble_data.myAddress.addr[4],
                                         ble_data.myAddress.addr[3],
                                         ble_data.myAddress.addr[2],
                                         ble_data.myAddress.addr[1],
                                         ble_data.myAddress.addr[0]);//SERVER ADDRESS

        //store ID in advertisingSetHandle
        ble_data.advertisingSetHandle = 0xff;

        // Create an advertising set.
        retstat = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
        app_assert_status(retstat);

        // Set advertising interval to 250ms.
        retstat = sl_bt_advertiser_set_timing(
            ble_data.advertisingSetHandle, // advertising set handle
            0x190,                                                              // min. adv. interval (milliseconds * 1.6) 250*1.6 = 400 hex 190
            0x190,                                                              // max. adv. interval (milliseconds * 1.6) 250*1.6 = 400 hex 190
            0,                                                                  // adv. duration
            0);                                                                 // max. num. adv. events
        app_assert_status(retstat);

        //Once the device is booted up start advertising
        retstat = sl_bt_advertiser_start(
            ble_data.advertisingSetHandle,
            sl_bt_advertiser_general_discoverable,
            sl_bt_advertiser_connectable_scannable);
        app_assert_status(retstat);

        ble_data.soft_timer_handle = evt->data.evt_system_soft_timer.handle;

        displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");

#else
        displayInit();
        displayPrintf(DISPLAY_ROW_NAME, "%s", BLE_DEVICE_TYPE_STRING);              //CLIENT
        displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");                       //connection not opened yet

        // display address on LCD
        displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                                         evt->data.evt_connection_opened.address.addr[5],
                                         evt->data.evt_connection_opened.address.addr[4],
                                         evt->data.evt_connection_opened.address.addr[3],
                                         evt->data.evt_connection_opened.address.addr[2],
                                         evt->data.evt_connection_opened.address.addr[1],
                                         evt->data.evt_connection_opened.address.addr[0]);//CLIENT ADDRESS




        // Set passive scanning on 1Mb PHY
        retstat = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, SCAN_PASSIVE);
        app_assert_status(retstat);

        // Set scan interval and scan window
        retstat = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, SCAN_INTERVAL, SCAN_WINDOW);
        app_assert_status(retstat);

        // Set the default connection parameters for subsequent connections
        retstat = sl_bt_connection_set_default_parameters(CONN_INTERVAL_MIN,    // min. connection. interval 75/ 1.25 = 60 hex 3C
                                                     CONN_INTERVAL_MAX,         // max. connection. interval 75/ 1.25 = 60 hex 3C
                                                     CONN_RESPONDER_LATENCY,    // latency
                                                     CONN_TIMEOUT,              // timeout
                                                     CONN_MIN_CE_LENGTH,        // min ce length
                                                     CONN_MAX_CE_LENGTH);       // max ce length
        app_assert_status(retstat);

        // Start scanning - looking for thermometer devices
        retstat = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
        app_assert_status_f(retstat,"Failed to start discovery #1\n");

#endif

        break;

      case sl_bt_evt_connection_opened_id:
        // handle open event

#if DEVICE_IS_BLE_SERVER
        //Once the device is connected to the app stop advertising
        retstat = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
        app_assert_status(retstat);

        //store the connection handle to send an indication for temperature
        ble_data.connection_handle = evt->data.evt_connection_opened.connection;

        //set the connection parameters
        retstat = sl_bt_connection_set_parameters  (
            ble_data.connection_handle,                                         // connection handle
            CONN_INTERVAL_MIN,                                                  // min. connection. interval 75/ 1.25 = 60 hex 3C
            CONN_INTERVAL_MAX,                                                  // max. connection. interval 75/ 1.25 = 60 hex 3C
            CONN_RESPONDER_LATENCY,                                             // latency
            CONN_TIMEOUT,                                                       // timeout
            CONN_MIN_CE_LENGTH,                                                 // min ce length
            CONN_MAX_CE_LENGTH);                                                // max ce length

        app_assert_status(retstat);
        //LOG_INFO("connection set parameters return status: %ld\n\r", retstat);

        displayPrintf(DISPLAY_ROW_CONNECTION,"Connected");

#else
         displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");                                      //connection not opened yet
         displayPrintf(DISPLAY_ROW_BTADDR2, "%02X:%02X:%02X:%02X:%02X:%02X",
                                                                           SERVER_BT_ADDRESS.addr[5],
                                                                           SERVER_BT_ADDRESS.addr[4],
                                                                           SERVER_BT_ADDRESS.addr[3],
                                                                           SERVER_BT_ADDRESS.addr[2],
                                                                           SERVER_BT_ADDRESS.addr[1],
                                                                           SERVER_BT_ADDRESS.addr[0]);   //SERVER ADDRESS
         //store the connection handle
         ble_data.connection_handle = evt->data.evt_connection_opened.connection;

#endif

        break;
      case sl_bt_evt_connection_closed_id:
        // handle close event

#if DEVICE_IS_BLE_SERVER
        // Restart advertising after client has disconnected.
        ble_data.i_am_a_bool_for_temp = false;
        retstat = sl_bt_advertiser_start(
            ble_data.advertisingSetHandle,
            sl_bt_advertiser_general_discoverable,
            sl_bt_advertiser_connectable_scannable);
        app_assert_status(retstat);

        displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");
        displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
#else
        displayPrintf(DISPLAY_ROW_NAME, "%s", BLE_DEVICE_TYPE_STRING);              //CLIENT
        displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");                       //connection not opened yet
        displayPrintf(DISPLAY_ROW_TEMPVALUE,"");                                    //TEMP ROW blank
        displayPrintf(DISPLAY_ROW_BTADDR2,"");                                      //SERVER ADDR blank

#endif
        break;

      case  sl_bt_evt_connection_parameters_id:
        //log all the connection parameters if any of them changes
       /* LOG_INFO("Connection params: connection=%d\n\r, interval=%d\n\r, latency=%d\n\r, timeout=%d\n\r, securitymode=%d\n\r",
                (int) (evt->data.evt_connection_parameters.connection),
                (int) (evt->data.evt_connection_parameters.interval*1.25),
                (int) (evt->data.evt_connection_parameters.latency),
                (int) (evt->data.evt_connection_parameters.timeout*10),
                (int) (evt->data.evt_connection_parameters.security_mode) );
        */
        break;

        //Events for Slave/Server
      case sl_bt_evt_gatt_server_characteristic_status_id:
        //This flag is raised every time the characteristics of the connection change
        if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) {
            //Indicates either that a local Client Characteristic Configuration descriptor (CCCD) was changed by the remote GATT client

            //check if the Characteristic client configuration has changed
            if(sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {
                //check if the indication is enabled
                if (sl_bt_gatt_disable != (sl_bt_gatt_client_config_flag_t) evt->data.evt_gatt_server_characteristic_status.client_config_flags){
                    ble_data.characteristic = evt->data.evt_gatt_server_characteristic_status.characteristic;
                    ble_data.i_am_a_bool_for_temp = true;
                    ble_data.i_am_a_bool_for_inflight = true; //checks if the indication is in flight
                }
                // confirmation of indication received from remove GATT client
                else if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {
                  sl_bt_ht_temperature_measurement_indication_confirmed_cb(
                    evt->data.evt_gatt_server_characteristic_status.connection);
                  ble_data.i_am_a_bool_for_inflight = false;
                }
                else{
                    displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
                    ble_data.i_am_a_bool_for_temp = false;    //checks if indication confirmation is received
                    //LOG_INFO("Indication disabled!!\n\r");
                }
            }
        }
        break;

      case sl_bt_evt_gatt_server_indication_timeout_id:
        //LOG_INFO("The connection timed out!!!\n\r");
        //Possible event from calling sl_bt_gatt_server_send_indication() -
        //i.e. we never received a confirmation for a previously transmitted indication.
        break;

      case sl_bt_evt_system_soft_timer_id:
        displayUpdate();
        break;

 //******************************EVENTS THAT CORRESPOND TO JUST THE CLIENT***********************************//
        // -------------------------------
        // This event is generated when an advertisement packet or a scan response
        // is received from a responder
        case sl_bt_evt_scanner_scan_report_id:
          // Parse advertisement packets
          if (evt->data.evt_scanner_scan_report.packet_type == 0) {
            // If a thermometer advertisement is found...
            if (find_service_in_advertisement(&(evt->data.evt_scanner_scan_report.data.data[0]),
                                              evt->data.evt_scanner_scan_report.data.len) != 0) {
              // then stop scanning for a while
              retstat = sl_bt_scanner_stop();
              app_assert_status(retstat);
              // and connect to that device
                retstat = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                           evt->data.evt_scanner_scan_report.address_type,
                                           sl_bt_gap_1m_phy,
                                           NULL);
                app_assert_status(retstat);
            }
          }
          break;

          // -------------------------------
          // This event is generated when a new service is discovered
         case sl_bt_evt_gatt_service_id:
           LOG_INFO("Ble conn_handle: %ld server connection handle: %ld Service handle: %ld\n\r",ble_data.connection_handle, evt->data.evt_gatt_service.connection, evt->data.evt_gatt_service.service );
            if (ble_data.connection_handle == evt->data.evt_gatt_service.connection) {
              // Save service handle for future reference
              ble_data.client_service_handle = evt->data.evt_gatt_service.service;
            }
          break;

            // -------------------------------
            // This event is generated when a new characteristic is discovered
         case sl_bt_evt_gatt_characteristic_id:
           LOG_INFO("Ble conn_handle: %ld server connection handle: %ld Characteristic handle: %ld\n\r",ble_data.connection_handle, evt->data.evt_gatt_service.connection,evt->data.evt_gatt_characteristic.characteristic);
              if (ble_data.connection_handle == evt->data.evt_gatt_service.connection) {
                // Save service handle for future reference
                ble_data.client_characteristic_handle = evt->data.evt_gatt_characteristic.characteristic;
              }
         break;

              // -------------------------------
              // This event is generated for various procedure completions, e.g. when a
              // write procedure is completed, or service discovery is completed
         case sl_bt_evt_gatt_procedure_completed_id:
             //check in discovery state machine
         break;

               // -------------------------------
               // This event is generated when a characteristic value was received e.g. an indication
         case sl_bt_evt_gatt_characteristic_value_id:
                 if (evt->data.evt_gatt_characteristic_value.value.len >= 5) {
                   char_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);

                   if (ble_data.connection_handle == evt->data.evt_gatt_characteristic_value.connection) {
                     temp_client= gattFloat32ToInt(char_value);
                     displayPrintf(DISPLAY_ROW_TEMPVALUE,"temp=%d", temp_client);
                     LOG_INFO("temp: %d\n\r", temp_client);
                   }
                 } else {
                   app_log_warning("Characteristic value too short: %d\n",
                                   evt->data.evt_gatt_characteristic_value.value.len);
                 }
         break;
  } // end - switch

} // handle_ble_event()


void sl_bt_ht_temperature_measurement_indication_changed_cb(uint8_t connection, uint16_t characteristic)
{

    // -------------------------------------------------------------------
    // Update our local GATT DB and send indication if enabled for the characteristic
    // -------------------------------------------------------------------

  if(ble_data.i_am_a_bool_for_temp == true){
      sl_status_t sc;                                                           //to store and check the return status

      uint8_t htm_temperature_buffer[5];                                        // Stores the temperature data in the Health Thermometer (HTM) format.
                                                                                // format of the buffer is: flags_byte + 4-bytes of IEEE-11073 32-bit float
      uint8_t *p = htm_temperature_buffer;                                      // Pointer to HTM temperature buffer needed for converting values to bitstream.
      uint32_t htm_temperature_flt;                                             // Stores the temperature data read from the sensor in the IEEE-11073 32-bit float format

      temp = store();

      uint8_t flags = 0x00;                                                     // HTM flags set as 0 for Celsius, no time stamp and no temperature type.

      // "bitstream" refers to the order of bytes and bits sent. byte[0] is sent first, followed by byte[1]...
      UINT8_TO_BITSTREAM(p, flags); // put the flags byte in first, "convert" is a strong word, it places the byte into the buffer

      // Convert sensor data to IEEE-11073 32-bit floating point format.
      htm_temperature_flt = UINT32_TO_FLOAT(temp*1000, -3);

      // Convert temperature to bitstream and place it in the htm_temperature_buffer
      UINT32_TO_BITSTREAM(p, htm_temperature_flt);

      sc = sl_bt_gatt_server_write_attribute_value( gattdb_temperature_measurement, // handle from gatt_db.h
                                                    0,                              // offset
                                                    5,                              // length
                                                    &htm_temperature_buffer[0]);    // pointer to buffer where data is


      if (sc != SL_STATUS_OK) {
      LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x", (unsigned int) sc);
      }

      //convert and store the temperature data for logging
      sc = sl_bt_gatt_server_send_indication(connection,                        //connection handle
                                             characteristic,                    //for what characteristic we are sending an indication
                                             5,                                 // length
                                             &htm_temperature_buffer[0]);       // pointer to buffer where data is);
      if (sc != SL_STATUS_OK) {
      LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x", (unsigned int) sc);
      }

      displayPrintf(DISPLAY_ROW_TEMPVALUE, "temp=%d", temp);
  }
  else{
      LOG_ERROR("Indication state has been changed in the middle of the state machine!!");
  }
}


