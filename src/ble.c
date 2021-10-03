/*
 * ble.c
 *
 *  Created on: Sep 30, 2021
 *      Author: salon
 */

#include "src/ble.h"
#include "app.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// BLE private data
ble_data_struct_t ble_data;

connection_struct_t connection_data;

sl_status_t sc=0;

uint32_t adv_int=0x190;       //250 ms
uint16_t conn_int = 0x3c;     //75 ms
uint16_t slave_latency = 0x04;    //4
uint16_t spvsn_timeout = 0x50;   //800ms

uint8_t htm_temperature_buffer[5];
uint8_t *p = htm_temperature_buffer;
uint32_t htm_temperature_flt;
uint8_t flags = 0x00;

uint8_t char_stat_connection;
uint16_t char_stat_characteristic;

ble_data_struct_t * getBleDataPtr() {

  return (&ble_data);
}

void ble_SendTemp() {

  if(connection_data.connected == true) {
  float temperature_in_c = convertTemp();

  UINT8_TO_BITSTREAM(p, flags);

  htm_temperature_flt = UINT32_TO_FLOAT(temperature_in_c*1000, -3);

  UINT32_TO_BITSTREAM(p, htm_temperature_flt);

  sl_status_t sc = sl_bt_gatt_server_write_attribute_value(gattdb_temperature_measurement, 0, 5, &htm_temperature_buffer[0]);

  if(sc != SL_STATUS_OK) {
      LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
  }

  if (connection_data.indication == true) {
      sc = sl_bt_gatt_server_send_indication(char_stat_connection, char_stat_characteristic, 5, &htm_temperature_buffer[0]);
      if(sc != SL_STATUS_OK) {
            LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
        }
  }
  }

}

void handle_ble_event(sl_bt_msg_t *evt) {

  ble_data_struct_t *bleData = getBleDataPtr();

  switch(SL_BT_MSG_ID(evt->header)) {

    //for both server and client
    case sl_bt_evt_system_boot_id:


      /*Read the Bluetooth identity address used by the device, which can be a public
       * or random static device address.
       *
       * @param[out] address Bluetooth public address in little endian format
       * @param[out] type Address type   */
      sc = sl_bt_system_get_identity_address(&(bleData->myAddress), &(bleData->myAddressType));
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /*Create an advertising set. The handle of the created advertising set is
       * returned in response.
       *
       * @param[out] handle Advertising set handle  */
      sc = sl_bt_advertiser_create_set(&(bleData->advertisingSetHandle));
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_create_set() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      sc = sl_bt_advertiser_set_timing(bleData->advertisingSetHandle, adv_int, adv_int, 0, 0);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_set_timing() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      sc = sl_bt_advertiser_start(bleData->advertisingSetHandle, sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      break;

    case sl_bt_evt_connection_opened_id:

      connection_data.connected = true;
      LOG_INFO("connection open event\n\r");
      sc = sl_bt_advertiser_stop(bleData->advertisingSetHandle);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /*PACKSTRUCT( struct sl_bt_evt_connection_opened_s
      {
       bd_addr address;
       uint8_t address_type;
       uint8_t master;
       uint8_t connection;
       uint8_t bonding;
       uint8_t advertiser;
      });*/
      sc = sl_bt_connection_set_parameters(evt->data.evt_connection_opened.connection, conn_int, conn_int, slave_latency, spvsn_timeout, 0xffff, 0xffff);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_connection_set_parameters() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }
      break;

    case sl_bt_evt_connection_closed_id:

      LOG_INFO("connection closed event, reason=%x", (uint32_t) evt->data.evt_connection_closed.reason);
      connection_data.connected = false;

      sc = sl_bt_advertiser_start(bleData->advertisingSetHandle, sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      break;

    case sl_bt_evt_connection_parameters_id:

      /*PACKSTRUCT( struct sl_bt_evt_connection_parameters_s
      {
        uint8_t  connection;
        uint16_t interval;
        uint16_t latency;
        uint16_t timeout;
        */
      LOG_INFO("Connection params: connection=%d, interval=%d, latency=%d, timeout=%d, securitymode=%d\n\r",
              (int) (evt->data.evt_connection_parameters.connection),
              (int) (evt->data.evt_connection_parameters.interval*1.25),
              (int) (evt->data.evt_connection_parameters.latency),
              (int) (evt->data.evt_connection_parameters.timeout*10),
              (int) (evt->data.evt_connection_parameters.security_mode) );
      //log parameters value
      break;

    case sl_bt_evt_system_external_signal_id:

      break;

      //for servers
    case sl_bt_evt_gatt_server_characteristic_status_id:

      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) {
      char_stat_connection = evt->data.evt_gatt_server_characteristic_status.connection;
      char_stat_characteristic = evt->data.evt_gatt_server_characteristic_status.characteristic;

      //LOG_INFO("client_config = %d\n\r", (unsigned int)evt->data.evt_gatt_server_characteristic_status.client_config_flags);
      LOG_INFO("charac=%d, conn=%d\n\r",char_stat_characteristic, char_stat_connection);

      if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

          connection_data.indication = !connection_data.indication;
      /*if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0) {
          connection_data.indication = false;
      }
      else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == 2) {
          connection_data.indication = true;
      }*/

      }
      //track indication bool
      }
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:

      LOG_INFO("server indication timeout\n\r");
      connection_data.indication = false;
      break;

      //for clients

  }
}
