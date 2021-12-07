/*
 * ble.h
<<<<<<< HEAD
 *
 *  Created on: Sep 30, 2021
 *      Author: salon
=======
 *  Handles all the bluetooth related events.
 *  Created on: Sep 30, 2021
 *      Author: mich1576
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

<<<<<<< HEAD
#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "src/i2c.h"
#include "app.h"

#include "em_letimer.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "main.h"

#define UINT8_TO_BITSTREAM(p, n)      { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n)     { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
    *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e)         (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

#define MAX_PTR 16
#define READ 0
#define WRITE 1
=======
#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }                     // use this for the flags byte, which you set = 0

#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                  *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }

#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };                                                                                //which is later converted to bitstream to send to the app.

// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e

// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition
<<<<<<< HEAD

//private structure for indication data
struct buffer_entry
{
  uint16_t charHandle;
  size_t bufferLength;
  uint8_t buffer[5];
};

typedef struct {

  // values that are common to servers and clients

  //server address
  bd_addr myAddress;
  uint8_t myAddressType;

  // values unique for server

  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle;
  //connection handle
  uint8_t connection_handle;
  //flag to check if bluetooth is connected
  bool    connected;
  //flag to check if HTM indication is on
  bool    indication;
  //flag to check if push button indication is on
  bool button_indication;

  bool gesture_indication;

  bool oximeter_indication;
  //flag to check if indication is in flight
  bool    indication_inFlight;
  //rollover count variable
  uint32_t rollover_cnt;
  //flag to check if push button is pressed
  bool button_pressed;
  bool pb1_button_pressed;
  //flag to check if server and client are bonded
  bool bonded;
  //variable to save bonding passkey
  uint32_t passkey;

  // values unique for client
  //temperature service handle
  uint32_t service_handle;
  //temperature characteristic handle
  uint16_t char_handle;
  //indication characteristic value from server
  uint8_t * char_value;
  //button service handle
  uint32_t button_service_handle;
  //button characteristic handle
  uint16_t button_char_handle;

  //gesture service handle
  uint32_t gesture_service_handle;
  //gesture characteristic handle
  uint16_t gesture_char_handle;
  //gesture value
  uint8_t gesture_value;
  //gesture on
  bool gesture_on;

  //button service handle
  uint32_t oximeter_service_handle;
  //button characteristic handle
  uint16_t oximeter_char_handle;
  //turn off oximeter sensor
  bool oximeter_off;


} ble_data_struct_t;

ble_data_struct_t* getBleDataPtr(void);

#if DEVICE_IS_BLE_SERVER

void ble_SendTemp();

void ble_SendButtonState(uint8_t value);

void ble_SendGestureState(uint8_t value);

void ble_SendOximeterState(uint8_t* pulse_data);

#endif

void handle_ble_event(sl_bt_msg_t *evt);

=======
typedef struct {
      // values that are common to servers and clients
      bd_addr myAddress;

      uint8_t myAddressType;

      // values unique for server
      uint8_t advertisingSetHandle;

      //store connection handle for sending the indication
      uint8_t connection_handle;

      //store the characteristic we are sending the indication for
      uint16_t characteristic;

      //soft timer handle
      uint8_t soft_timer_handle;

      //Rollover count
      uint8_t rollover_count;

      //milliseconds
      uint32_t milliseconds;

      //bool indication for temperature measurement characteristic
      bool i_am_a_bool_for_temp;
      // values unique for client

      //bool for checking if connection is open
      bool i_am_a_bool_for_inflight;

      //variable to store the client service handle
      uint32_t client_service_handle;

      //variable to store the client characteristic handle
      uint16_t client_characteristic_handle;

} ble_data_struct_t;

// function prototypes
/* function     : getBleDataPtr
 * params       : void
 * brief        : function to get the ble data struct rather than keeping it global and corrupting the data inside
 * return type  : ble_data_struct_t*
 */
ble_data_struct_t* getBleDataPtr(void);

/* function     : handle_ble_event
 * params       : sl_bt_msg_t *evt
 * brief        : takes different bluetooth events as input and and handles events based on different flags
 * return type  : void
 */
void handle_ble_event(sl_bt_msg_t *evt);

/* function     : sl_bt_ht_temperature_measurement_indication_confirmed_cb
 * params       : uint8_t connection
 * brief        : function that checks confirmatin of indication from the EFR connect
 * return type  : void
 */
void sl_bt_ht_temperature_measurement_indication_confirmed_cb(uint8_t connection);

/* function     : sl_bt_ht_temperature_measurement_indication_changed_cb
 * params       : uint8_t connection, uint16_t characteristic
 * brief        : function called if indication is enabled from the user
 * return type  : void
 */
void sl_bt_ht_temperature_measurement_indication_changed_cb(uint8_t connection, uint16_t characteristic);

/* function     : gattFloat32ToInt
 * params       : const uint8_t *value_start_little_endian
 * brief        : function to convert float data into integer
 * return type  : void
 */
int32_t gattFloat32ToInt(const uint8_t *value_start_little_endian);

>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
#endif /* SRC_BLE_H_ */
