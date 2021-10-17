/*
 * scheduler.c
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
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

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
