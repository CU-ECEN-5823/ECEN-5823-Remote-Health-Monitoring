/*
 * scheduler.h
 *
 *  Created on: Sep 15, 2021
 *      Author: salon
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "app.h"

#include "em_letimer.h"
#include "main.h"

void schedulerSetEventCOMP1();

void schedulerSetEventTransferDone();

//function to set a scheduler event
void schedulerSetEventUF();

//function to get scheduler event
uint32_t getNextEvent();

void temperature_state_machine(uint32_t event);


#endif /* SRC_SCHEDULER_H_ */
