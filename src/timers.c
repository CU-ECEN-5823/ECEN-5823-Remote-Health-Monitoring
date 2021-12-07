/*
 * timers.c
<<<<<<< HEAD
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
#include "src/timers.h"

//resolution of LETIMER clock tick

#if (LOWEST_ENERGY_MODE < 3)
#define CLK_RES 61
#define MIN_WAIT 61

#else
#define CLK_RES 1000
#define MIN_WAIT 1000
#endif

#define MAX_WAIT 6000000  //maximum wait time possible

//structure to define parameters for LETIMER
const LETIMER_Init_TypeDef LETIMER_INIT_STRUCT = {
    false,              /* Disable timer when initialization completes. */
    true,              /* Allow counter to run during debug halt. */
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

  //enable underflow interrupt of timer peripheral
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

}

//interrupt based delay of at least us_wait microseconds, using LETIMER0 tick counts as a reference
void timerWaitUs_interrupt(uint32_t us_wait) {
  uint16_t desired_tick, current_cnt, required_cnt;

  //check function argument range
  if((us_wait<(uint32_t)MIN_WAIT) || (us_wait>(uint32_t)MAX_WAIT)) {
      LOG_ERROR("TimerWait range\n\r");

      //clamp wait time value
      if(us_wait < (uint32_t)MIN_WAIT) {
          us_wait = MIN_WAIT;
      }

      else if(us_wait > (uint32_t)MAX_WAIT) {
          us_wait = MAX_WAIT;
      }
  }

  desired_tick = (us_wait/CLK_RES);           //calculate required timer ticks

  current_cnt = LETIMER_CounterGet(LETIMER0); //get current LETIMER counter value

  required_cnt = current_cnt-desired_tick;    //get required

  //handle roll-over case
  if(required_cnt > VALUE_TO_LOAD_COMP0) {

      //if required counter value is more than current counter value; wait till counter value is 0,
      //    recalculate remaining required ticks and poll for that much time
      required_cnt = VALUE_TO_LOAD_COMP0 - (0xFFFF - required_cnt);
  }

  LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);

  LETIMER_CompareSet(LETIMER0, 1, required_cnt); //Set value of COMP1

  //enable COMP1 interrupt of timer peripheral
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);

  LETIMER0->IEN |= LETIMER_IEN_COMP1;

}


//blocks (polls) at least us_wait microseconds, using LETIMER0 tick counts as a reference
void timerWaitUs_polled(uint32_t us_wait) {
  uint16_t desired_tick, current_cnt, required_cnt;

  //check function argument range
  /*if((us_wait<(uint32_t)MIN_WAIT) | (us_wait>(uint32_t)MAX_WAIT)) {
      LOG_ERROR("TimerWait range\n\r");

      //clamp wait time value
      if(us_wait < (uint16_t)MIN_WAIT) {
          us_wait = MIN_WAIT;
      }

      else if(us_wait > (uint16_t)MAX_WAIT) {
          us_wait = MAX_WAIT;
      }
  }*/

  desired_tick = (us_wait/CLK_RES);           //calculate required timer ticks

  current_cnt = LETIMER_CounterGet(LETIMER0); //get current LETIMER counter value

  required_cnt = current_cnt-desired_tick;    //get required number of timer ticks

  //wait time value wrap around case
  /* if(required_cnt >= (uint16_t)VALUE_TO_LOAD_COMP0) {
      required_cnt = 0xFFFF - required_cnt;

      required_cnt = VALUE_TO_LOAD_COMP0 - required_cnt;
  }*/

  //poll until required time period passes
  if(current_cnt >= desired_tick) {
      while((LETIMER_CounterGet(LETIMER0)) != (required_cnt));
  }

  //handle roll-over case
  else {
      //if required counter value is more than current counter value; wait till counter value is 0,
      //    recalculate remaining required ticks and poll for that much time
      while((LETIMER_CounterGet(LETIMER0)) != (uint32_t)(VALUE_TO_LOAD_COMP0-(desired_tick-current_cnt)));
  }

}


=======
 * This file contains function prototype for timer initialization and interrupt settings.
 *  Created on: Sep 9, 2021
 *      Author: mich1576
 */

/*********************************************************INCLUDES****************************************************************/
#include "timers.h"

/*******************************************************GLOBAL VARIABLES*********************************************************/
//Logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

/****************************************************FUNCTION DEFINITION**********************************************************/
//initialize LETIMER0, set the compare register values based on the energy mode settings and set the correct settings for interrupt
void initLETIMER0 (void){
  LETIMER_Init_TypeDef timer_instance=
      {
          .enable         = true,                                               //Don't start counting when initialization completes

          .debugRun       = true,                                               //Counter will not keep running during debug halt

          .comp0Top       = true,                                               //Load COMP0 register into CNT when counter underflows

          .bufTop         = false,                                              //Don't load COMP1 into COMP0 when REP0 reaches 0

          .out0Pol        = 0,                                                  //Idle value for output 0
          .out1Pol        = 0,                                                  //Idle value for output 1
          .ufoa0          = letimerUFOANone,                                    //No output action
          .ufoa1          = letimerUFOANone,                                    //No output action
          .repMode        = letimerRepeatFree,                                  //Repeat until stopped

          .topValue   = 0,                                                      //Use default top Value
        };

  LETIMER_Init(LETIMER0, &timer_instance);

#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))
    LETIMER_CompareSet(LETIMER0, 0, VALUE_TO_LOAD_LFXO_COMP0);
    LETIMER_IntClear(LETIMER0, LETIMER_IF_UF);                                  //clear the underflow flag
    LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);                                //enable underflow in interruot enable register

#elif (LOWEST_ENERGY_MODE == 3)
    LETIMER_CompareSet(LETIMER0, 0, VALUE_TO_LOAD_ULFRCO_COMP0);
  LETIMER_IntClear(LETIMER0, LETIMER_IF_UF);                                    //clear the underflow flag
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);                                  //enable underflow in interruot enable register

#endif

}

//Takes input in microseconds and provides required amount of delay
void timerWaitUs_polled(uint32_t us_wait){

  uint32_t difference, current, store;

#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))
    uint32_t us_each_tick   = US_EACH_TICK_LFXO ;
    uint32_t wait_for_ticks = (us_wait/us_each_tick);

    store = VALUE_TO_LOAD_LFXO_COMP0;
    if(wait_for_ticks> VALUE_TO_LOAD_LFXO_COMP0){
        LOG_ERROR("Invalid wait input\n\r");
    }
#elif (LOWEST_ENERGY_MODE == 3)

    uint32_t us_each_tick   = US_EACH_TICK_ULFRCO ;                             //Using the ULFRCO and tick for that
    uint32_t wait_for_ticks = (us_wait/us_each_tick);                           //calculate the number of ticks required for the same

    store = VALUE_TO_LOAD_ULFRCO_COMP0;
    if(wait_for_ticks> VALUE_TO_LOAD_ULFRCO_COMP0){                             //if the value for ticks exceeds the range provide error message
        LOG_ERROR("Invalid wait input\n\r");
    }
#endif

    else{
            current = LETIMER_CounterGet (LETIMER0);                            //take the current value of the timer
            if( current > wait_for_ticks ){                                     //check if the required number of ticks are greater than current counter value
                difference = current - wait_for_ticks;                          //take the difference between current timer counter value and the required ticks
                while(current>=difference){                                     //poll until required ticks are obtained
                   current = LETIMER_CounterGet (LETIMER0);
                }
            }
            else{

                difference = store - (wait_for_ticks - current);                //count until the counter reaches zero and calculate remaining number of ticks
                 while(current != difference){
                     current = LETIMER_CounterGet (LETIMER0);
                 }
            }
        }
}

//Takes input in microseconds and provides required amount of delay
void timerWaitUs_irq(uint32_t us_wait){

  uint32_t difference, current, store;

#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))

    uint32_t us_each_tick   = US_EACH_TICK_LFXO ;
    uint32_t wait_for_ticks = (us_wait/us_each_tick);


    store = VALUE_TO_LOAD_LFXO_COMP0;                                           //store the overflow value
    if(wait_for_ticks> VALUE_TO_LOAD_LFXO_COMP0){                               //if the value for ticks exceeds the range provide error message
        LOG_ERROR("Invalid wait input\n\r");
    }

#elif (LOWEST_ENERGY_MODE == 3)

    uint32_t us_each_tick   = US_EACH_TICK_ULFRCO ;                             //Using the ULFRCO and tick for that
    uint32_t wait_for_ticks = (us_wait/us_each_tick);                           //calculate the number of ticks required for the same

    store = VALUE_TO_LOAD_ULFRCO_COMP0;                                         //store the overflow value
    if(wait_for_ticks> VALUE_TO_LOAD_ULFRCO_COMP0){                             //if the value for ticks exceeds the range provide error message
        LOG_ERROR("Invalid wait input\n\r");
    }
#endif

    else{
            current = LETIMER_CounterGet (LETIMER0);                            //take the current value of the timer
            if( current > wait_for_ticks ){                                     //check if the required number of ticks are greater than current counter value
                difference= current - wait_for_ticks;                           //take the difference between current timer counter value and the required ticks
            }
            else{
                LOG_ERROR("rollover");
                difference = store - (wait_for_ticks - current);                //count until the counter reaches zero and calculate remaining number of ticks
            }
            LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);                      //to avoid any timer related issues caused by BLE external signal
            LETIMER_CompareSet(LETIMER0, 1, difference);                        //Set COMP1 register to calculated value
            LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);                     //Enable COMP1 interrupt
    }
}
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
