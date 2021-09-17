/*
 * oscillators.c
 *
 * Function to initialize and set oscillator type and frequencies
 *  for different energy modes
 *
 *  Created on: Sep 8, 2021
 *      Author: saloni
 */

#include "src/oscillators.h"
#include "app.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define LFXO_FREQ   32768   //frequency of low frequency crystal oscillator
#define LFXO_PRESCALER 2    //prescaler for low frequency oscillator

int select_oscillator() {

  return (LFXO_FREQ/LFXO_PRESCALER);

}

void oscillator_init() {

  //int lfa_clk=0, letimer_clk=0;   //variables to check clock frequencies
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);      //enable LFXO oscillator
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);   //select LFXO oscillator for low frequency peripheral branch
  CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_2);    //set prescaler for LETIMER0 peripheral
  CMU_ClockEnable(cmuClock_LETIMER0, true);           //enable clock for LETIMER0 peripheral
  CMU_ClockFreqGet(cmuClock_LFA);           //get clock frequency for low frequency branch
  CMU_ClockFreqGet(cmuClock_LETIMER0);  //get clock frequency for LETIMER
  //LOG_INFO("lfa_clk=%d,  letimer_clk=%d\n\r", (int32_t)lfa_clk, (int32_t)letimer_clk);   //log clock frequencies

}


