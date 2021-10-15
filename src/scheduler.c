/*
 * scheduler.c
 *
 *  Created on: Sep 15, 2021
 *      Author: salon
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "src/scheduler.h"

uint32_t MyEvent;

sl_status_t rc=0;

// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

//enum for interrupt based events
enum {
  evt_NoEvent=0,
  evt_TimerUF,
  evt_COMP1,
  evt_TransferDone,
};

//enum to define scheduler events
typedef enum uint32_t {
  state0_idle,
  state1_timer_wait,
  state2_write_cmd,
  state3_write_wait,
  state4_read,
  state0_idle_client,
  state1,
  state2,
  state3,
  state4,
  MY_NUM_STATES,
}my_state;

// scheduler routine to set a scheduler event
void schedulerSetEventUF() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_TimerUF);

  // set the event in your data structure, this is a read-modify-write
  //MyEvent |= evt_TimerUF;

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventCOMP1() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_COMP1);
  // set the event in your data structure, this is a read-modify-write
  //MyEvent |= evt_COMP1;

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventTransferDone() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(evt_TransferDone);
  // set the event in your data structure, this is a read-modify-write
  //MyEvent |= evt_TransferDone;

  // exit critical section
  CORE_EXIT_CRITICAL();

} // schedulerSetEventXXX()

// scheduler routine to return 1 event to main()code and clear that event
uint32_t getNextEvent() {

  static uint32_t theEvent=evt_NoEvent;
  //determine 1 event to return to main() code, apply priorities etc.
  // clear the event in your data structure, this is a read-modify-write
  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  if(MyEvent & evt_TimerUF) {
      theEvent = evt_TimerUF;
      MyEvent ^= evt_TimerUF;
  }
  else if(MyEvent & evt_COMP1) {
      theEvent = evt_COMP1;
      MyEvent ^= evt_COMP1;
  }
  else if(MyEvent & evt_TransferDone) {
      theEvent = evt_TransferDone;
      MyEvent ^= evt_TransferDone;
  }

  // exit critical section
  CORE_EXIT_CRITICAL();

  return (theEvent);
} // getNextEvent()

#if DEVICE_IS_BLE_SERVER
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
              //LOG_INFO("Temp = %f C\n\r", convertTemp());

              //send temperature indication to client
              ble_SendTemp();

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

  bleData->client_event = SL_BT_MSG_ID(evt->header);

  currentState = nextState;     //set current state of the process

  switch(currentState) {

    case state0_idle_client:
      nextState = state0_idle_client;          //default
      LOG_INFO("In client idle\n\r");

      if(bleData->client_event == sl_bt_evt_connection_opened_id) {
          LOG_INFO("Connection opened\n\r");
          bleData->gatt_procedure = true;
          rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                            sizeof(thermo_service),
                                                            (const uint8_t*)thermo_service);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

          nextState = state1;
      }

      break;

    case state1:
      nextState = state1;
      LOG_INFO("In state1\n\r");

      if(bleData->client_event == sl_bt_evt_gatt_procedure_completed_id) {

          LOG_INFO("discovered a service\n\r");
          bleData->gatt_procedure = true;
          rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                           bleData->service_handle,
                                                           sizeof(thermo_char),
                                                           (const uint8_t*)thermo_char);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }


          nextState = state2;
      }

      break;

    case state2:
      nextState = state2;
      LOG_INFO("in state2\n\r");

      if(bleData->client_event == sl_bt_evt_gatt_procedure_completed_id) {
          LOG_INFO("Discovered characteristic\n\r");
          bleData->gatt_procedure = true;
          rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                          bleData->char_handle,
                                                          sl_bt_gatt_indication);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }


          displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
          nextState = state3;
      }

      break;

    case state3:
      nextState = state3;
      LOG_INFO("In state3\n\r");

      if(bleData->client_event == sl_bt_evt_gatt_characteristic_value_id) {
          LOG_INFO("Got an indication\n\r");
          rc = sl_bt_gatt_send_characteristic_confirmation(bleData->connection_handle);
          if(rc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)rc);
          }

      }

      if(bleData->client_event == sl_bt_evt_connection_closed_id) {
          LOG_INFO("Connection close event\n\r");
          nextState = state4;
      }

      break;

    case state4:
      nextState = state4;

      if(bleData->client_event == sl_bt_evt_connection_opened_id) {
          LOG_INFO("connection open event\n\r");
          nextState = state0_idle_client;
      }

      break;

    default:

      LOG_ERROR("Should not be here in state machine\n\r");

      break;

  }
}

#endif
