/*
 * irq.c
<<<<<<< HEAD
 *
 * Function to
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "src/irq.h"

uint32_t letimerMilliseconds() {
  uint32_t time_ms;
  ble_data_struct_t *bleData = getBleDataPtr();
  time_ms = ((bleData->rollover_cnt)*3000);
  return time_ms;
}

void LETIMER0_IRQHandler(void) {

  ble_data_struct_t *bleData = getBleDataPtr();

  // determine pending interrupts in peripheral
  uint32_t reason = LETIMER_IntGetEnabled(LETIMER0);
  // clear pending interrupts in peripheral
  LETIMER_IntClear(LETIMER0, reason);

  //check for COMP1 interrupt
  if(reason & LETIMER_IF_COMP1) {

      //disable COMP1 interrupt of timer peripheral
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);

      //set scheduler event
      schedulerSetEventCOMP1();
  }

  //check for UF interrupt
  if(reason & LETIMER_IF_UF) {

      //set scheduler event
      schedulerSetEventUF();

      bleData->rollover_cnt+=1;

  }

}

void I2C0_IRQHandler(void) {

  I2C_TransferReturn_TypeDef transferStatus;

  //get I2C transfer status
  transferStatus = I2C_Transfer(I2C0);

  //check if I2C transfer is done
  if(transferStatus == i2cTransferDone) {

      //disable I2C transfer
      NVIC_DisableIRQ(I2C0_IRQn);

      //set scheduler event
      schedulerSetEventTransferDone();
  }

  if(transferStatus < 0) {

      LOG_ERROR("I2C_TStatus %d : failed\n\r", (uint32_t)transferStatus);

  }
}

void GPIO_EVEN_IRQHandler(void) {

  ble_data_struct_t *bleData = getBleDataPtr();

  // determine pending interrupts in peripheral
  uint32_t reason = GPIO_IntGet();

  GPIO_IntClear(reason);

  //get the push button status
  uint8_t button_status = GPIO_PinInGet(PB0_port, PB0_pin);

  //check if the interrupt triggered was from PB0
  if(reason == 64) {

      if(!button_status) {
          bleData->button_pressed = true;
          schedulerSetEventButtonPressed();
      }

      else {
          bleData->button_pressed = false;
          schedulerSetEventButtonReleased();
      }
  }

}


void GPIO_ODD_IRQHandler(void) {

  ble_data_struct_t *bleData = getBleDataPtr();

  // determine pending interrupts in peripheral
  uint32_t reason = GPIO_IntGet();

  GPIO_IntClear(reason);

#if DEVICE_IS_BLE_SERVER

  if(reason == 2048)
    schedulerSetGestureEvent();

#endif

  //get the push button status
  uint8_t button_status = GPIO_PinInGet(PB1_port, PB1_pin);

  //check if the interrupt triggered was from PB1
  if(reason == 128) {

      if(!button_status) {
          bleData->pb1_button_pressed = true;
          schedulerSetEventButtonPressed();
      }

      else {
          bleData->pb1_button_pressed = false;
          schedulerSetEventButtonReleased();
      }
  }
=======
 * This file contains function prototype defining the functionality in interrupt handler
 *  Created on: Sep 9, 2021
 *      Author: mich1576
 */

/*********************************************************INCLUDES****************************************************************/
#include "irq.h"
#include "ble.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

I2C_TransferReturn_TypeDef transferStatus;

/****************************************************FUNCTION DEFINITION*********************************************************/
//Check for any interrupts, turn the LED on and off accordingly and reset the interrupt flags.
void LETIMER0_IRQHandler (void) {

    uint32_t flags;
    ble_data_struct_t *bleDataPtr = getBleDataPtr();
        // determine source of IRQ
        flags = LETIMER_IntGetEnabled(LETIMER0);
        // clear source of IRQ set in step 3
        LETIMER_IntClear(LETIMER0, flags);

        if(flags & LETIMER_IF_UF){
            //call scheduler to read temperature data from SI7021
            scheduler_evtUF ();
            bleDataPtr->rollover_count++;
        }

        if(flags & LETIMER_IF_COMP1){
            //call scheduler to read temperature data from SI7021
            scheduler_evtCOMP1 ();
        }

} // LETIMER0_IRQHandler()

void I2C0_IRQHandler(void) {

        // This shepherds the IC2 transfer along,
        // it’s a state machine! see em_i2c.c
        // It accesses global variables :
        // transferSequence
        // cmd_data
        // read_data
        transferStatus = I2C_Transfer(I2C0);
        if (transferStatus == i2cTransferDone) {
            scheduler_evtI2C ();
        }
        if(transferStatus < 0) {
            LOG_ERROR("%d", transferStatus);
        }
} // I2C0_IRQHandler()

uint32_t letimerMilliseconds(){
  ble_data_struct_t *bleDataPtr = getBleDataPtr();
#if ((LOWEST_ENERGY_MODE == 0) || (LOWEST_ENERGY_MODE == 1) || (LOWEST_ENERGY_MODE == 2))
  bleDataPtr->milliseconds = bleDataPtr->rollover_count*LETIMER_PERIOD_MS + MILLIECONDS_PER_TICK_LFXO;
#elif (LOWEST_ENERGY_MODE == 3)
  bleDataPtr->milliseconds = bleDataPtr->rollover_count*LETIMER_PERIOD_MS + MILLIECONDS_PER_TICK_ULFRCO;
#endif

    return bleDataPtr->milliseconds;
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
}
