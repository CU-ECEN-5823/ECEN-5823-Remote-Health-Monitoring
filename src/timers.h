/*
 * timers.h
<<<<<<< HEAD
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
=======
 * This file contains function prototype for timer initialization and interrupt settings. Check .c file for function definition.
 *  Created on: Sep 9, 2021
 *      Author: mich1576
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

<<<<<<< HEAD
#include "src/oscillators.h"
#include "app.h"

#include "em_letimer.h"

#define ACTUAL_CLK_FREQ select_oscillator()     //get actual clock frequency
#define VALUE_TO_LOAD_COMP0 (LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000     //calculate value to load in COMP0

//Function to initialize LETIMER
void mytimer_init();

//function for interrupt based delay
void timerWaitUs_interrupt(uint32_t us_wait);

//function for polling delay
void timerWaitUs_polled(uint32_t us_wait);

=======
#include "em_letimer.h"
#include "main.h"
#include "app.h"
#include "oscillators.h"
#include <stdint.h>

//function prototypes
/* function       : initLETIMER0
 * params         : void
 * brief          : initialize LETIMER0, set the compare register values based on the energy mode settings and set the correct settings for interrupt
 * return type    : void
 */
void initLETIMER0 ();

/* function       : timerWaitUs_polled
 * params         : uint32_t us_wait
 * brief          : Takes input in microseconds and provides required amount of delay using polling method
 * return type    : void
 */
void timerWaitUs_polled(uint32_t us_wait);

/* function       : timerWaitUs_interrupt
 * params         : uint32_t us_wait
 * brief          : Takes input in microseconds and provides required amount of delay using interrupt method
 * return type    : void
 */
void timerWaitUs_irq(uint32_t us_wait);
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
#endif /* SRC_TIMERS_H_ */
