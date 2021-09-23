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

enum {
  evt_NoEvent,
  evt_TimerUF,
  evt_COMP1,
  evt_TransferDone
};

//enum to define scheduler events
typedef enum uint32_t {
  state0_idle,
  state1_timer_wait,
  state2_write_cmd,
  state3_write_wait,
  state4_read,
  MY_NUM_STATES,
}my_state;

// scheduler routine to set a scheduler event
void schedulerSetEventUF() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // set the event in your data structure, this is a read-modify-write
  MyEvent = evt_TimerUF;

  // exit critical section
  CORE_EXIT_CRITICAL();

  //LOG_INFO("event UF\n\r");
} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventCOMP1() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // set the event in your data structure, this is a read-modify-write
  MyEvent = evt_COMP1;

  // exit critical section
  CORE_EXIT_CRITICAL();

  //LOG_INFO("event COMP1\n\r");
} // schedulerSetEventXXX()

// scheduler routine to set a scheduler event
void schedulerSetEventTransferDone() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // set the event in your data structure, this is a read-modify-write
  MyEvent = evt_TransferDone;

  // exit critical section
  CORE_EXIT_CRITICAL();

  //LOG_INFO("event transfer\n\r");
} // schedulerSetEventXXX()

// scheduler routine to return 1 event to main()code and clear that event
uint32_t getNextEvent() {

  uint32_t theEvent;
  //determine 1 event to return to main() code, apply priorities etc.
  theEvent = MyEvent;

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // clear the event in your data structure, this is a read-modify-write
  MyEvent = evt_NoEvent;
  // exit critical section
  CORE_EXIT_CRITICAL();

  //printf("theEvent=%ld\n\r",theEvent);

  return (theEvent);
} // getNextEvent()

void temperature_state_machine(uint32_t event) {

  //LOG_INFO("in FSM\n\r");

  my_state currentState;
  static my_state nextState = state0_idle;

  currentState = nextState;

  switch(currentState) {

    case state0_idle:
      nextState = state0_idle;          //default
      //LOG_INFO("In idle\n\r");

      if(event == evt_TimerUF) {
          LOG_INFO("got timer overflow\n\r");

          //enable temperature sensor
          enable_sensor();

          //wait for 80ms
          timerWaitUs_interrupt(80000);
          nextState = state1_timer_wait;
      }

      break;

    case state1_timer_wait:
      nextState = state1_timer_wait;    //default
      //LOG_INFO("in timer wait\n\r");

      if(event == evt_COMP1) {
          LOG_INFO("got comp1 after power on wait\n\r");

          write_cmd();

          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          nextState = state2_write_cmd;
      }

      break;

    case state2_write_cmd:
      nextState = state2_write_cmd;     //default
      //LOG_INFO("in write cmd\n\r");

      if(event == evt_TransferDone) {
          LOG_INFO("write command transfer done\n\r");

          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

          //wait 12ms for measurement
          timerWaitUs_interrupt(11000);

          nextState = state3_write_wait;
      }

      break;

    case state3_write_wait:
      nextState = state3_write_wait;    //default
      //LOG_INFO("in write wait\n\r");

      if(event == evt_COMP1) {
          LOG_INFO("got comp1 after write wait\n\r");

          read_cmd();

          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          nextState = state4_read;
      }

      break;

    case state4_read:
      nextState = state4_read;          //default
      //LOG_INFO("in read\n\r");

      if(event == evt_TransferDone) {
          LOG_INFO("read command transfer done\n\r");

          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

          disable_sensor();

          //disable I2C interrupt
          NVIC_DisableIRQ(I2C0_IRQn);

          //convert sensor data into temperature
          //sensor_temp = convertTemp();

          //log temperature value
          LOG_INFO("Temp = %f C\n\r", convertTemp());

          nextState = state0_idle;
      }

      break;

    default:

      LOG_ERROR("Should not be here in state machine\n\r");

      break;
  }
}
