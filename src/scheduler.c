/*
 * scheduler.c
<<<<<<< HEAD
 *
 *  Created on: Sep 15, 2021
 *      Author: salon
 */

=======
 * Schedules event for Underflow, COMP1 and I2C. Traverses through different states for server in temperature state machine
 * and the same for client in discovery state machine.
 *  Created on: Sep 15, 2021
 *      Author: mich1576
 */

/*********************************************************INCLUDES****************************************************************/
#include "scheduler.h"
#include "ble.h"
#include "app_assert.h"
#include "lcd.h"
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

<<<<<<< HEAD
#include "app.h"
#include "src/scheduler.h"
#include "pulse_oximeter.h"

uint32_t MyEvent;

sl_status_t rc=0;
int pulse_data_count=0;

uint8_t read_device_mode[2] = {0x02, 0x00};  //send this command to begin the communication with pulse oximeter

#if !DEVICE_IS_BLE_SERVER
// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

// button state service UUID defined by Bluetooth SIG
// 00000001-38c8-433e-87ec-652a2d136289
static const uint8_t button_service[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00 };
// button state characteristic UUID defined by Bluetooth SIG
// 00000002-38c8-433e-87ec-652a2d136289
static const uint8_t button_char[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00 };

//gesture sensor service uuid
//d38b2e9b-966c-4426-bda0-9c017e23bb35
static const uint8_t gesture_service[16] = { 0x35, 0xbb, 0x23, 0x7e, 0x01, 0x9c, 0xa0, 0xbd, 0x26, 0x44, 0x6c, 0x96, 0x9b, 0x2e, 0x8b, 0xd3 };
//gesture sensor characteristic uuid
//e38b2e9b-966c-4426-bda0-9c017e23bb35
static const uint8_t gesture_char[16] = { 0x35, 0xbb, 0x23, 0x7e, 0x01, 0x9c, 0xa0, 0xbd, 0x26, 0x44, 0x6c, 0x96, 0x9b, 0x2e, 0x8b, 0xe3 };

//pulse oximeter service uuid
//ee63a26e-8809-4038-8c7c-341ed4042818
static const uint8_t oximeter_service[16] = { 0x18, 0x28, 0x04, 0xd4, 0x1e, 0x34, 0x7c, 0x8c, 0x38, 0x40, 0x09, 0x88, 0x6e, 0xa2, 0x63, 0xee};
//pulse oximeter characteristic uuid
//fe63a26e-8809-4038-8c7c-341ed4042818
static const uint8_t oximeter_char[16] = { 0x18, 0x28, 0x04, 0xd4, 0x1e, 0x34, 0x7c, 0x8c, 0x38, 0x40, 0x09, 0x88, 0x6e, 0xa2, 0x63, 0xfe};


#endif

//enum for interrupt based events
enum {
  evt_NoEvent=0,
  evt_TimerUF,
  evt_COMP1,
  evt_TransferDone,
  evt_ButtonPressed,
  evt_ButtonReleased,
  evt_GestureInt,
};

//enum to define scheduler events
typedef enum uint32_t {
  state0_idle,
  state1_timer_wait,
  state2_write_cmd,
  state3_write_wait,
  state4_read,
  state0_idle_client,
  state0_get_button_service,
  state0_get_gesture_service,
  state0_get_oximeter_service,
  state1_get_temp_char,
  state1_get_button_char,
  state1_get_gesture_char,
  state1_get_oximeter_char,
  state2_set_temp_ind,
  state2_set_button_ind,
  state2_set_gesture_ind,
  state2_set_oximeter_ind,
  state3_all_set,
  state4_wait_for_close,
  state0_gesture_wait,
  state1_gesture,
  //server state machine
  state_no_gesture,
  //for oximeter state machine
  state_pulse_sensor_init,
  state_wait_10ms,
  state_wait_1s,
  state_read_return_check,
  state_set_output_mode,
  state_setFifoThreshold,
  state_agcAlgoControl,
  state_max30101Control,
  state_maximFastAlgoControl,
  state_readAlgoSamples,
  state_wait_before_reading,
  state_read_sensor_hub_status,
  state_numSamplesOutFifo,
  state_read_fill_array,
  state_pulse_done,
  MY_NUM_STATES,
}my_state;

// scheduler routine to set a scheduler event
void schedulerSetEventUF() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_TimerUF);

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventCOMP1() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_COMP1);

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventTransferDone() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_TransferDone);

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventButtonPressed() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_ButtonPressed);

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventButtonReleased() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_ButtonReleased);

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetGestureEvent() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_GestureInt);

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

#if DEVICE_IS_BLE_SERVER

void handle_gesture() {

  ble_data_struct_t *bleData = getBleDataPtr();

  if ( isGestureAvailable() ) {
      LOG_INFO("Is gesture available?\n\r");

      switch ( readGesture() ) {

        case DIR_UP:
          //LOG_INFO("DOWN\n\r");
          bleData->gesture_value = 0x04;
          displayPrintf(DISPLAY_ROW_9, "Gesture = DOWN");
          disableGestureSensor();
          bleData->gesture_on = false;
          displayPrintf(DISPLAY_ROW_ACTION, "Gesture sensor OFF");
          //LOG_INFO("Sending down gesture\n\r");
          ble_SendGestureState(0x04);

          break;

        case DIR_DOWN:
          //LOG_INFO("UP\n\r");
          bleData->gesture_value = 0x03;
          displayPrintf(DISPLAY_ROW_9, "Gesture = UP");
          //LOG_INFO("Sending up gesture\n\r");
          ble_SendGestureState(0x03);


          break;

        case DIR_LEFT:
          //LOG_INFO("LEFT\n\r");
          bleData->gesture_value = 0x01;
          displayPrintf(DISPLAY_ROW_9, "Gesture = LEFT");
          //LOG_INFO("Sending left gesture\n\r");
          ble_SendGestureState(0x01);


          break;

        case DIR_RIGHT:
          //LOG_INFO("RIGHT\n\r");
          bleData->gesture_value = 0x02;
          displayPrintf(DISPLAY_ROW_9, "Gesture = RIGHT");
          // LOG_INFO("Sending right gesture\n\r");
          ble_SendGestureState(0x02);


          break;

        case DIR_NEAR:
          //LOG_INFO("NEAR\n\r");
          bleData->gesture_value = 0x05;
          displayPrintf(DISPLAY_ROW_9, "Gesture = NEAR");
          // LOG_INFO("Sending near gesture\n\r");
          ble_SendGestureState(0x05);


          break;

        case DIR_FAR:
          //LOG_INFO("FAR\n\r");
          bleData->gesture_value = 0x06;
          displayPrintf(DISPLAY_ROW_9, "Gesture = FAR");

          //LOG_INFO("Sending far gesture\n\r");
          ble_SendGestureState(0x06);


          break;

        default:
          //LOG_INFO("NONE");
          bleData->gesture_value = 0x00;
          displayPrintf(DISPLAY_ROW_9, "Gesture = NONE");
          ble_SendGestureState(0x00);
      }
  }
}

void gesture_state_machine(sl_bt_msg_t *evt) {

  my_state currentState;
  static my_state nextState = state0_gesture_wait;

  currentState = nextState;     //set current state of the process

  switch(currentState) {

    case state0_gesture_wait:

      nextState = state0_gesture_wait;          //default

      //check for underflow event
      if(evt->data.evt_system_external_signal.extsignals == evt_GestureInt) {

          LOG_INFO("GestureInt event\n\r");

          handle_gesture();

          nextState = state1_gesture;
      }

      break;

    case state1_gesture:

      nextState = state0_gesture_wait;

      break;

    default:
      LOG_ERROR("Should not be here in gesture state machine\n\r");
  }

  return;

}

void oximeter_state_machine(sl_bt_msg_t *evt) {

  my_state currentState;
  static my_state nextState = state_pulse_sensor_init;
  ble_data_struct_t *bleData = getBleDataPtr();
  bool gesture_check =false;

    currentState = nextState;     //set current state of the process

    switch(currentState) {
            case state_pulse_sensor_init:

              //setting MFIO and RESET as output, reset is set and mfio is cleared
              pulse_oximeter_init_pins();

              //clear the reset in and set the MFIO pin
              turn_off_reset();

              //Set the MFIO pin
              turn_on_mfio();

              //wait 10ms
              timerWaitUs_interrupt(10000);

              nextState = state_wait_10ms;

              break;

            case state_wait_10ms:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                //set reset pin
                turn_on_reset();

                //wait for 1 second
                timerWaitUs_interrupt(1000000);

                nextState = state_wait_1s;
              }

              break;

            case state_wait_1s:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                //set MFIO pin as an interrupt
                set_MFIO_interrupt();

                //settings for putting the device into read mode
                I2C_pulse_write_polled(read_device_mode, 2);

                //wait for 10ms before performing a read
                timerWaitUs_interrupt(6000);

                nextState = state_read_return_check;
              }

              break;

            case state_read_return_check:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                  //perform read to check the return value
                  I2C_pulse_read_polled();

                  //perform read to check if it returns 0
                  read_return_check();

                  //set the output mode
                  set_output_mode_func();

                  //wait for 6ms between a write and a read
                  timerWaitUs_interrupt(6000);

                  nextState = state_set_output_mode;
              }

              break;

            case state_set_output_mode:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                 //perform read to check the return value
                  I2C_pulse_read_polled();

                  //perform read to check if it returns 0
                  read_return_check();

                  //set the fifo threshold
                  setFifoThreshold_func();

                  //wait for 6ms before performing a read
                  timerWaitUs_interrupt(6000);

                  nextState = state_setFifoThreshold;
              }

              break;

            case state_setFifoThreshold:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                  //LOG_INFO("In state_setFifoThreshold!!\n\r");

                  //wait for 100ms before performing red
                  //NOTE:This is set to match the timings as per the arduino code provided in Github
                  timerWaitUs_polled(100000);

                  //perform read to check the return value
                   I2C_pulse_read_polled();

                   //perform read to check if it returns 0
                   read_return_check();

                   //agc algo control commands
                   agcAlgoControl_func();

                   //wait for 6ms before performing a read
                   timerWaitUs_interrupt(6000);

                   nextState = state_agcAlgoControl;
              }

              break;

            case state_agcAlgoControl:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                  //LOG_INFO("In state_agcAlgoControl!!\n\r");

                  //wait for 100ms before performing red
                  //NOTE:This is set to match the timings as per the arduino code provided in Github
                  timerWaitUs_polled(100000);

                  //perform read to check the return value
                   I2C_pulse_read_polled();

                   //set the control for the sensor
                   max30101Control_func();

                   //wait for 6ms before performing a read
                   timerWaitUs_interrupt(6000);

                   nextState = state_max30101Control;

              }

              break;

            case state_max30101Control:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                  //LOG_INFO("In state_max30101Control!!\n\r");
                  //wait for 100ms before performing red
                  //NOTE:This is set to match the timings as per the arduino code provided in Github
                  timerWaitUs_polled(100000);

                  //perform read to check the return value
                   I2C_pulse_read_polled();

                   //maximum fast algo control
                   maximFastAlgoControl_func();

                   //wait for 6ms before performing a read
                   timerWaitUs_interrupt(6000);

                   nextState = state_maximFastAlgoControl;

              }

              break;

            case state_maximFastAlgoControl:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                  //LOG_INFO("In state_maximFastAlgoControl!!\n\r");
                  //wait for 100ms before performing red
                  //NOTE:This is set to match the timings as per the arduino code provided in Github
                  timerWaitUs_polled(100000);

                  //perform read to check the return value
                   I2C_pulse_read_polled();

                   //read the algo samples before taking the actual readings
                   readAlgoSamples_func();

                   //wait for 6ms before performing a read
                   timerWaitUs_interrupt(6000);

                   nextState = state_readAlgoSamples;

              }

              break;

            case state_readAlgoSamples:

              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){


                  //perform read to check the return value
                   I2C_pulse_read_polled();

                   //perform read to check if it returns 0
                   read_return_check();

                   //wait 6 seconds before taking the actual reading
                   timerWaitUs_interrupt(6000000);

                   nextState = state_wait_before_reading;

              }

              break;

            case state_wait_before_reading:
              LOG_INFO("In state_wait_before_reading\n\r");
              if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                  //read the sensor status
                   read_sensor_hub_status_func();

                   //wait 6 seconds before taking the actual reading
                   timerWaitUs_interrupt(6000);

                   nextState = state_read_sensor_hub_status;

              }

              else if(!bleData->oximeter_off){

                  LOG_INFO("In else if of state_wait_before_reading\n\r");
                  //read the sensor status
                   read_sensor_hub_status_func();

                   //wait 6 seconds before taking the actual reading
                   timerWaitUs_interrupt(6000);

                   nextState = state_read_sensor_hub_status;
              }

              break;

            case state_read_sensor_hub_status:

                  if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                        //read for checking the return value
                        I2C_pulse_read_polled();

                        //perform read to check if it returns 0
                        read_return_check();

                        //number of samples out of the fifo
                        numSamplesOutFifo_func();

                        //wair for 6ms before performing a read
                        timerWaitUs_interrupt(6000);

                         nextState = state_numSamplesOutFifo;
                  }

              break;

            case state_numSamplesOutFifo:

                if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                    //perform a read to check the return value
                    I2C_pulse_read_polled();

                    //perform read to check if it returns 0
                    read_return_check();

                    //start reading sesnor data by sending read commands
                    read_fill_array_func();

                    //wair for 6ms before performing a read
                    timerWaitUs_interrupt(6000);

                     nextState = state_read_fill_array;

                }

              break;

            case state_read_fill_array:

                if(evt->data.evt_system_external_signal.extsignals == evt_COMP1){

                    //read the actual data
                    I2C_pulse_read_polled();

                    //read 15 values to give sensor time to acquire appropriate values
                      if(pulse_data_count <1){

                          //stop after 15 counts
                          pulse_data_count = extract_data();

                          //read after every second
                          timerWaitUs_interrupt(1000000);

                          nextState = state_wait_before_reading;
                      }

                      else{
                          bleData->oximeter_off = true;
                          //gesture_check = enableGestureSensor(true);
//                          LOG_INFO("gesture_check:%d\n\r",gesture_check);
                          //displayPrintf(DISPLAY_ROW_ACTION, "Gesture sensor ON");
//                          turn_off_reset();
//                          GPIO_IntDisable((uint32_t)4);
                          pulse_data_count = 0;
                          bleData->gesture_value = 0x00;
                          bleData->gesture_on = true;
                          nextState = state_wait_before_reading;

                      }

                }

              break;

            case state_pulse_done:
              LOG_INFO("In state_pulse_done\n\r");
                  if(bleData->oximeter_off == true){
                      LOG_INFO("condition true!!\n\r");
//                      nextState = state_pulse_sensor_init;
                      nextState = state_wait_before_reading;
                  }
                  else{
                      LOG_INFO("condition false!!\n\r");
                      nextState = state_pulse_done;
                  }
              break;

            default:
              LOG_INFO("Something wrong!! In the default state of oximeter state machine");

              break;

      }
    return;
}


//for server only
//state machine to be executed
void temperature_state_machine(sl_bt_msg_t *evt) {

  my_state currentState;
  static my_state nextState = state0_idle;
  ble_data_struct_t *bleData = getBleDataPtr();

  if((SL_BT_MSG_ID(evt->header)==sl_bt_evt_system_external_signal_id)
      && (bleData->connected==true)
      && (bleData->indication==true)) {

      currentState = nextState;     //set current state of the process

      switch(currentState) {

        case state0_idle:
          nextState = state0_idle;          //default

          //check for underflow event
          if(evt->data.evt_system_external_signal.extsignals == evt_TimerUF) {

              //LOG_INFO("timerUF event\n\r");
              //enable temperature sensor
              //enable_sensor();

              //wait for 80ms for sensor to power on
              timerWaitUs_interrupt(80000);

              nextState = state1_timer_wait;
          }

          break;

        case state1_timer_wait:
          nextState = state1_timer_wait;    //default

          //check for COMP1 event after timerwait
          if(evt->data.evt_system_external_signal.extsignals == evt_COMP1) {

              //LOG_INFO("Comp1 event\n\r");

              //set the processor in EM1 energy mode
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

              //send write command to slave
              write_cmd();

              nextState = state2_write_cmd;
          }

          break;

        case state2_write_cmd:
          nextState = state2_write_cmd;     //default

          //check for I2C transfer complete event after writing command to slave
          if(evt->data.evt_system_external_signal.extsignals == evt_TransferDone) {

              //LOG_INFO("write transfer done\n\r");
              //remove processor from EM1 energy mode
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

              //wait 10.8ms for measurement of temperature
              timerWaitUs_interrupt(10800);

              nextState = state3_write_wait;
          }

          break;

        case state3_write_wait:
          nextState = state3_write_wait;    //default

          //check for COMP1 event after timerwait
          if(evt->data.evt_system_external_signal.extsignals == evt_COMP1) {

              //read data from sensor
              read_cmd();

              //set the processor in EM1 energy mode
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

              nextState = state4_read;
          }

          break;

        case state4_read:
          nextState = state4_read;          //default

          //check for I2C transfer complete event after reading data from slave
          if(evt->data.evt_system_external_signal.extsignals == evt_TransferDone) {

              //LOG_INFO("read transfer  done\n\r");
              //remove processor from EM1 energy mode
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

              //disable si7021 sensor
              //disable_sensor();

              //disable I2C interrupt
              NVIC_DisableIRQ(I2C0_IRQn);

              //log temperature value
              // LOG_INFO("Temp = %f C\n\r", convertTemp());

              //send temperature indication to client
              ble_SendTemp();
              bleData->gesture_value = 0x00;
                                        bleData->gesture_on = true;
                                        enableGestureSensor(true);
                                        displayPrintf(DISPLAY_ROW_ACTION, "Gesture sensor ON");

              nextState = state0_idle;
          }

          break;

        default:

          LOG_ERROR("Should not be here in state machine\n\r");

          break;
      }

  }
  return;
}


#else

void discovery_state_machine(sl_bt_msg_t *evt) {

  my_state currentState;
  static my_state nextState = state0_idle_client;

  ble_data_struct_t *bleData = getBleDataPtr();

  if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id) {
      nextState = state0_idle_client;
  }

  currentState = nextState;     //set current state of the process

  switch(currentState) {

    //stay in idle state
    case state0_idle_client:
      nextState = state0_idle_client;          //default

      //wait for connection open event
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id) {

          //LOG_INFO("Discovering services\n\r");

          //Discover primary services with the specified UUID in a remote GATT database.
          rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                            sizeof(thermo_service),
                                                            (const uint8_t*)thermo_service);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state1_get_temp_char;          //default
      }
      break;

      //got service from server
     case state1_get_temp_char:
       nextState = state1_get_temp_char;

       //wait for previous gatt command to be completed
       if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

           //LOG_INFO("Discovering characteristics\n\r");


           //Discover all characteristics of a GATT service in a remote GATT database
           // having the specified UUID
           rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                            bleData->service_handle,
                                                            sizeof(thermo_char),
                                                            (const uint8_t*)thermo_char);
           if(rc != SL_STATUS_OK) {
               LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 1 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
           }

           nextState = state2_set_temp_ind;
       }

       break;

       //got characteristic from server
        //enable indications for temperature service
      case state2_set_temp_ind:
        nextState = state2_set_temp_ind;

        //wait for previous gatt command to be completed
        if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

            //LOG_INFO("Enabling notifications\n\r");


            //enable indications sent from server
            rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                            bleData->char_handle,
                                                            sl_bt_gatt_indication);
            if(rc != SL_STATUS_OK) {
                LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
            }

            nextState = state0_get_button_service;
        }

        break;

      //discover button state service
    case state0_get_button_service:
      nextState = state0_get_button_service;          //default

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


          //LOG_INFO("Discovering services 2\n\r");

          rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                            sizeof(button_service),
                                                            (const uint8_t*)button_service);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 2 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state1_get_button_char;
      }

      break;

      //discover button state characteristics
    case state1_get_button_char:
      nextState = state1_get_button_char;

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


          //LOG_INFO("Discovering services 2\n\r");

          rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                           bleData->button_service_handle,
                                                           sizeof(button_char),
                                                           (const uint8_t*)button_char);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 2 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state2_set_button_ind;
      }

      break;

      //enble indications for button state service
     case state2_set_button_ind:
       nextState = state2_set_button_ind;

       //wait for previous gatt command to be completed
       if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


           //LOG_INFO("Enabling notifications 2\n\r");

           rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                           bleData->button_char_handle,
                                                           sl_bt_gatt_indication);

           if(rc != SL_STATUS_OK) {
               LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
           }

           bleData->button_indication = true;

           //displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
           nextState = state0_get_gesture_service;
       }

       break;

      //discover button state service
    case state0_get_gesture_service:
      nextState = state0_get_gesture_service;          //default

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {
      //if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id) {

          //LOG_INFO("Discovering services 3\n\r");

          rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                            sizeof(gesture_service),
                                                            (const uint8_t*)gesture_service);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 2 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state1_get_gesture_char;
      }

      break;

      //discover button state characteristics
    case state1_get_gesture_char:
      nextState = state1_get_gesture_char;

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


          //LOG_INFO("Discovering services 3\n\r");

          rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                           bleData->gesture_service_handle,
                                                           sizeof(gesture_char),
                                                           (const uint8_t*)gesture_char);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 2 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state2_set_gesture_ind;
      }

      break;

      //enble indications for button state service
     case state2_set_gesture_ind:
       nextState = state2_set_gesture_ind;

       //wait for previous gatt command to be completed
       if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


           //LOG_INFO("Enabling notifications 3\n\r");

           rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                           bleData->gesture_char_handle,
                                                           sl_bt_gatt_indication);

           if(rc != SL_STATUS_OK) {
               LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
           }

           bleData->gesture_indication = true;

           //displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
           nextState = state0_get_oximeter_service;
       }

       break;

      //discover button state service
    case state0_get_oximeter_service:
      nextState = state0_get_oximeter_service;          //default

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


          //LOG_INFO("Discovering services 4\n\r");

          rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                            sizeof(oximeter_service),
                                                            (const uint8_t*)oximeter_service);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() 2 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state1_get_oximeter_char;
      }

      break;

      //discover button state characteristics
    case state1_get_oximeter_char:
      nextState = state1_get_oximeter_char;

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


          //LOG_INFO("Discovering services 4\n\r");

          rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                           bleData->oximeter_service_handle,
                                                           sizeof(oximeter_char),
                                                           (const uint8_t*)oximeter_char);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() 2 returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state2_set_oximeter_ind;
      }

      break;

      //enble indications for button state service
    case state2_set_oximeter_ind:
      nextState = state2_set_oximeter_ind;

      //wait for previous gatt command to be completed
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {


          //LOG_INFO("Enabling notifications 4\n\r");

          rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                          bleData->oximeter_char_handle,
                                                          sl_bt_gatt_indication);

          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          bleData->oximeter_indication = true;

          displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
          nextState = state3_all_set;
      }

      break;

      //indication is set on from server
    case state3_all_set:
      nextState = state3_all_set;

      //gatt complete
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id) {

          nextState = state4_wait_for_close;
      }

      break;

      //state to wait for a connection close event
    case state4_wait_for_close:
      nextState = state4_wait_for_close;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id) {

          //go in idle state to wait for a connection open event
          nextState = state0_idle_client;
      }

      break;

    default:

      LOG_ERROR("Should not be here in state machine\n\r");

      break;

  }
}

#endif
=======
/****************************************************FUNCTION DEFINITION**********************************************************/
//set the sl_bt_external_signal to Underflow event.
void scheduler_evtUF () {
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL();              // turn off interrupts in NVIC
  //myEvents |= evtUF;                // set event to underflow event
  sl_bt_external_signal(evtUF);       // set the external signal function for underflow event
  CORE_EXIT_CRITICAL();               // re-enable interrupts in NVIC
}

//set the sl_bt_external_signal to COMP1 event
void scheduler_evtCOMP1 () {
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL();              // turn off interrupts in NVIC
  //myEvents |= evtCOMP1;             // set event to COMP1 event
  sl_bt_external_signal(evtCOMP1);    // set the external signal function for COMP1 event
  CORE_EXIT_CRITICAL();               // re-enable interrupts in NVIC
}

//set the sl_bt_external_signal to I2C event
void scheduler_evtI2C () {
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL();              // turn off interrupts in NVIC
  //myEvents |= evtI2C;                  // set event to I2C event
  sl_bt_external_signal(evtI2C);    // set the external signal function for I2C event
  CORE_EXIT_CRITICAL();               // re-enable interrupts in NVIC
}

/************************************************************SERVER STATE MACHINE*****************************************************************/
void temperature_state_machine (sl_bt_msg_t *evt) {

     state_t currentState;                                                      //variable to get the current state
     static state_t nextState = Idle;                                           //initializing next state as ideal
     currentState = nextState;                                                  //initialize current state as next state
     ble_data_struct_t *bleDataPtr = getBleDataPtr();

     switch (currentState) {                                                    //check the value of current state
       case Idle:                                                               //case when the device is in idle state
         nextState = Idle;                                                      //next state should be idle unless an event is encountered
         if (evt->data.evt_system_external_signal.extsignals == evtUF) {        //if an underflow event is encountered
             if(bleDataPtr->i_am_a_bool_for_temp == true){                               //checks if indication are enabled only then takes a measuremnt
             warmup();                                                          //turn the sensor on
             timerWaitUs_irq(80000);                                            //wait for power up of 7021
             nextState = Warmup ;                                               //set the next state to warmup
             }
          }
         break;


       case Warmup:                                                             //if the device is in warmup state
         nextState = Warmup;                                                    //next state should be warmup unless an event is encountered
         if (evt->data.evt_system_external_signal.extsignals == evtCOMP1) {                                                //if COMP1 event encountered
             LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);                   //disable COMP1 interrupt
             sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);         //sleep in EM1 while performing EM1 transaction
             I2C_write();                                                       //perform I2C write
             nextState = write;                                                 //set the next state to write
         }
         break;

       case write:                                                              //if the device is in write state
         nextState = write;                                                     //next state should be write until event is encountered
         if (evt->data.evt_system_external_signal.extsignals == evtI2C) {                                                  //if I2C event encountered
             NVIC_DisableIRQ(I2C0_IRQn);                                        //Disable I2C interrupt
             sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //go back to EM3 sleep state
             timerWaitUs_irq(10800);                                            //wait at least 10.8ms
             nextState = timerwait;                                             //set next state to timerwait
         }
         break;


       case timerwait:                                                          //if device in timerwait state
         nextState = timerwait;                                                 //next state should be timerwait until an event encountered
         if (evt->data.evt_system_external_signal.extsignals == evtCOMP1) {                                                //if COMP1 event encountered
             LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);                   //disable COMP1 interrupt
             sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);         //sleep in EM1 while performing EM1 transaction
             I2C_read();                                                        //perform I2C read
             nextState = read;                                                  //set next state to read
         }
         break;


       case read:                                                               //if device in read state
         nextState = read;                                                      //next state should be timerwait until an event encountered
         if (evt->data.evt_system_external_signal.extsignals == evtI2C) {                                                  //if I2C event encountered
             NVIC_DisableIRQ(I2C0_IRQn);                                        //Disable I2C interrupt
             //turnoff();                                                         //turn the sensor off
             sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //go back to EM3 sleep state
                         sl_bt_ht_temperature_measurement_indication_changed_cb(bleDataPtr->connection_handle,
                                                                                bleDataPtr->characteristic);
             nextState = Idle;                                                  //set next state to Idle
         }
         break;

       default:
         break;
     } // switch
} // server state_machine()


/************************************************************CLIENT STATE MACHINE*****************************************************************/
void discovery_state_machine (sl_bt_msg_t *evt){

      sl_status_t retstat;                                                          //stores the return status of different bluetooth API functions
      conn_state_t currentState;                                                    //variable to get the current state
      static conn_state_t nextState = opening;                                      //initializing next state as ideal
      currentState = nextState;                                                     //initialize current state as next state
      ble_data_struct_t *bleDataPtr = getBleDataPtr();

      switch(currentState){
        case opening:                                                                                   //wait in this state until open event is observed
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id){                              //connection observed
              retstat = sl_bt_gatt_discover_primary_services_by_uuid(bleDataPtr->connection_handle,     //start discovering for services
                                                                     sizeof(thermo_service),
                                                                     (const uint8_t*)thermo_service);
              app_assert_status(retstat);                                                               //check return status
              nextState = discover_services;                                                            //set the next state to discover services until a complete event is observed
          }
         break;

        case discover_services:                                                                         //wait in this state until service is discovered
         if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id){                        //completed event observed
              retstat = sl_bt_gatt_discover_characteristics_by_uuid(bleDataPtr->connection_handle,      //start discovering for characteristics
                                                                    bleDataPtr->client_service_handle,
                                                                     sizeof(thermo_char),
                                                                     (const uint8_t*)thermo_char);
              app_assert_status(retstat);                                                               //check return status
              nextState = discover_characteristics;                                                     //set the next state to discover characteristics until completed event is observed
          }
          break;

        case discover_characteristics:                                                                  //wait in this state until service is discovered
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id){                       //completed event observed
              // stop discovering
              sl_bt_scanner_stop();                                                                     //stop scanning for devices
              // enable indications
              retstat = sl_bt_gatt_set_characteristic_notification(bleDataPtr->connection_handle,       //send indication
                                                                        bleDataPtr->client_characteristic_handle,
                                                                        sl_bt_gatt_indication);
              app_assert_status(retstat);                                                               //check return value
              displayPrintf(DISPLAY_ROW_CONNECTION,"Handling Indications");                             //display on LED as handling connections
              nextState = enable_indication;                                                            //stay in enable indication state until a close event is received
          }
          break;

        case enable_indication:                                                                         //stay in enable indication state until a close event is received
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_characteristic_value_id){                      //value received
              retstat = sl_bt_gatt_send_characteristic_confirmation(bleDataPtr->connection_handle);     //send confirmation that value is received
              app_assert_status(retstat);                                                               //check return value
              nextState = enable_indication;                                                            //Indication still enabled
          }
          else if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id){                         //if connection closed
              nextState = scanning;                                                                     //go to scanning
          }
          break;

        case scanning:
              retstat = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);          //start scanning for devices again
              app_assert_status_f(retstat, "Failed to start discovery #2\n");                           //check return value
            nextState = opening;                                                                        //go to opening state again
          break;

        default:
          break;
      }//switch

} //client state machine
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
