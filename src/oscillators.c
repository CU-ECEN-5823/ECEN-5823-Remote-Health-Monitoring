/*
 * oscillators.c
<<<<<<< HEAD
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
#define ULFRCO_FREQ 1000    //frequency for ultra low frequency RC oscillator
#define ULFRCO_PRESCALER 1  //prescaler for ultra low frequency RC oscillator

int select_oscillator() {

  //get actual clock frequency for EM3 mode
  if(LOWEST_ENERGY_MODE == 3) {
      return (ULFRCO_FREQ/ULFRCO_PRESCALER);
  }

  //get actual frequency for other energy modes
  else {
      return (LFXO_FREQ/LFXO_PRESCALER);
  }

}

void oscillator_init() {

  //clock set for EM3 energy mode
  if(LOWEST_ENERGY_MODE == 3) {
      CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);    //enable ULFRCO oscillator
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO); //select ULFRCO oscillator for low frequency peripheral branch
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_1);    //set prescaler for LETIMER0 peripheral
      CMU_ClockEnable(cmuClock_LETIMER0, true);           //enable clock for LETIMER0 peripheral
      //CMU_ClockFreqGet(cmuClock_LFA);                     //get clock frequency for low frequency branch
      //CMU_ClockFreqGet(cmuClock_LETIMER0);                //get clock frequency for LETIMER
  }

  //clock set for EM0, EM1 and EM2 energy modes
  else {
      CMU_OscillatorEnable(cmuOsc_LFXO, true, true);      //enable LFXO oscillator
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);   //select LFXO oscillator for low frequency peripheral branch
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_2);    //set prescaler for LETIMER0 peripheral
      CMU_ClockEnable(cmuClock_LETIMER0, true);           //enable clock for LETIMER0 peripheral
      //CMU_ClockFreqGet(cmuClock_LFA);                     //get clock frequency for low frequency branch
      //CMU_ClockFreqGet(cmuClock_LETIMER0);                //get clock frequency for LETIMER
  }

}


=======
 * This file contains function to initialize the oscillators and set appropriate values in CMU unit for respective energy modes
 *  Created on: Sep 9, 2021
 *      Author: mich1576
 */

/*********************************************************INCLUDES****************************************************************/
#include "oscillators.h"

/****************************************************FUNCTION DEFINITION**********************************************************/
//initializes the clock base on the energy mode setting and initializes the clock for use of LETIMER0
void init_oscillators(void){
  //Select the appropriate Low Frequency clock for the LFA clock tree depending on oscillator

#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);                                   //select LFXO
    CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);                                 //selecting the LFXO as the LFA clock
    CMU_ClockEnable(cmuClock_LFA, true);                                             //enabling the LFA clock
    CMU_ClockDivSet(cmuClock_LETIMER0, PRESCALAR_VALUE_LFXO);                        //providing the prescalar value for LETIMER0 for LFXO as clock input
    CMU_ClockEnable(cmuClock_LETIMER0, true);                                        //enabling the LETIMER0 clock

#elif (LOWEST_ENERGY_MODE == 3)

  CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);                                   //select ULFRCO
  CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_ULFRCO);                                 //selecting the ULFRCO as the LFA clock
  CMU_ClockEnable(cmuClock_LFA, true);                                               //enabling the LFA clock
  CMU_ClockDivSet(cmuClock_LETIMER0, PRESCALAR_VALUE_ULFRCO);                        //providing the prescalar value for LETIMER0 for ULFRCO as clock input
  CMU_ClockEnable(cmuClock_LETIMER0, true);                                          //enabling the LETIMER0 clock
#endif
}
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
