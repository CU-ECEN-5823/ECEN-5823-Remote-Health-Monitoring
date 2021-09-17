/*
 * timers.c
 *
 * Functions to initialize and set LETIMER0
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "main.h"
#include "src/timers.h"

#define ACTUAL_CLK_FREQ select_oscillator()     //get actual clock frequency
#define VALUE_TO_LOAD_COMP0 (LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000      //calculate value to load in COMP0

#define CLK_RES 61
#define MIN_WAIT 60
#define MAX_WAIT 3000001

//structure to define parameters for LETIMER
const LETIMER_Init_TypeDef LETIMER_INIT_STRUCT = {
    false,              /* Disable timer when initialization completes. */
    false,              /* Stop counter during debug halt. */
    true,               /* load COMP0 into CNT on underflow. */
    false,              /* Do not load COMP1 into COMP0 when REP0 reaches 0. */
    0,                  /* Idle value 0 for output 0. */
    0,                  /* Idle value 0 for output 1. */
    letimerUFOANone,    /* No action on underflow on output 0. */
    letimerUFOANone,    /* No action on underflow on output 1. */
    letimerRepeatFree,  /* Count until stopped by SW. */
    0                   /* Use default value as top Value. */
};

void mytimer_init() {

  LETIMER_Init(LETIMER0, &LETIMER_INIT_STRUCT);         //initialize LETIMER0 using values defined in the structure
  LETIMER_CompareSet(LETIMER0, 0, VALUE_TO_LOAD_COMP0); //Set value of COMP0
  LETIMER_Enable(LETIMER0, true);                       //Enable LETIMER0
  //LOG_INFO("Timer started\n\r");
}

//blocks (polls) at least us_wait microseconds, using LETIMER0 tick counts as a reference
void timerWaitUs(uint32_t us_wait) {
  uint32_t desired_tick, current_cnt;

  if((us_wait<=MIN_WAIT) | (us_wait>=MAX_WAIT)) {
      LOG_ERROR("TimerWait range\n\r");
  }

  desired_tick = (us_wait/CLK_RES);
  //LOG_INFO("TICKS=%ld\n\r", desired_tick);

  current_cnt = LETIMER_CounterGet(LETIMER0);
  //LOG_INFO("curr_cnt=%ld\n\r", current_cnt);

  if(current_cnt >= desired_tick) {
      while((LETIMER_CounterGet(LETIMER0)) != (current_cnt-desired_tick));
  }

  else {
      while((LETIMER_CounterGet(LETIMER0)) != (VALUE_TO_LOAD_COMP0-(desired_tick-current_cnt)));
  }

  //LOG_INFO("cnt=%d\n\r", LETIMER_CounterGet(LETIMER0));

  //LOG_INFO("Delay complete\n\r");

}


