/*
 * scheduler.h
 * Contains function prototypes for scheduling temperature measurement event. Check .c file for function definition.
 *  Created on: Sep 15, 2021
 *      Author: mich1576
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include <stdint.h>
#include "em_core.h"
#include "i2c.h"

//enum to determine which event to be in for server
typedef enum {
evtNoEvent = 0,
evtUF = 1,
evtCOMP1 = 2,
evtI2C = 4,
}event_t;

//enum to determine which state to be in for server
typedef enum{
Idle = 0,
Warmup = 1,
write = 2,
read = 3,
timerwait = 4,
}state_t;

//enum to determine which state to be in for client
typedef enum {
 opening = 0,
 discover_services = 1,
 discover_characteristics = 2,
 enable_indication = 3,
 scanning = 4,
} conn_state_t;



//***************************************************SERVER SCHEDULER EVENTS********************************************//
//function prototypes
/*  function          : scheduler_evtUF
 *  params            : none
 *  brief             : set the external signal of ble to underflow event
 *                      Makes sure that the variable is assigned value in critical section
 *                      Based on the previous state of the state machine, which would be idle
 *
 *  return-type       : void
 * */
void scheduler_evtUF ();

/*  function          : scheduler_evtCOMP1
 *  params            : none
 *  brief             : set the external signal of ble global variable to COMP1 event
 *                      Makes sure that the variable is assigned value in critical section
 *  return-type       : void
 * */
void scheduler_evtCOMP1 ();

/*  function          : scheduler_evtI2C
 *  params            : none
 *  brief             : set the external signal of ble global variable to I2C event
 *                      Makes sure that the variable is assigned value in critical section
 *  return-type       : void
 * */
void scheduler_evtI2C ();

//*****************************************************SERVER STATE MACHINE****************************************************//
/*  function          : temperature_state_machine
 *  params            : sl_bt_msg_t *evt
 *  brief             : Takes event sent by bluetooth struct as input and based on the previous state updates the next state
 *  return-type       : void
 * */
void temperature_state_machine (sl_bt_msg_t *evt);


//*****************************************************CLIENT STATE MACHINE****************************************************//
/*  function          : discovery_state_machine
 *  params            : none
 *  brief             : sequences through service and characteristics.
 *  return-type       : void
 * */
void discovery_state_machine ();
#endif /* SRC_SCHEDULER_H_ */
