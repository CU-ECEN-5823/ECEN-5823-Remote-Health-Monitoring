/*
 * irq.h
<<<<<<< HEAD
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
=======
 * This file contains function prototype defining the functionality in interrupt handler.Check .c file for function definition.
 *  Created on: Sep 9, 2021
 *      Author: mich1576
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
 */

#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_

<<<<<<< HEAD
#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "src/scheduler.h"
#include "app.h"

#include "em_letimer.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "main.h"

uint32_t letimerMilliseconds();


=======
#include "timers.h"
#include "oscillators.h"
#include "scheduler.h"

/* function     : letimerMilliseconds
 * params       : void
 * brief        : Calculates milliseconds since boot
 * return_type  : uint32_t
 * */
uint32_t letimerMilliseconds();

>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
#endif /* SRC_IRQ_H_ */
