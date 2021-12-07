/*
  gpio.c

   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */




#include "src/gpio.h"
#include "app.h"
#include "main.h"

// Set GPIO drive strengths and modes of operation
//NOTE: gpioInit has been removed from the app_init function anyways.
void gpioInit()
{

  // Student Edit:

<<<<<<< HEAD
  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  //GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
=======
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
  GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(sensor_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(sensor_port, sensor_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(lcd_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(lcd_port, lcd_pin, gpioModePushPull, false);

  GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_pin, true, true, true);

  GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig(PB1_port, PB1_pin, PB1_pin, true, true, true);

  GPIO_PinModeSet(gesture_port, gesture_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig(gesture_port, gesture_pin, gesture_pin, false, true, true);

} // gpioInit()


void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED0_port,LED0_pin);
}


void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED0_port,LED0_pin);
}


void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED1_port,LED1_pin);
}


void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED1_port,LED1_pin);
}

<<<<<<< HEAD
void enable_sensor() {
  GPIO_PinOutSet(sensor_port, sensor_pin);
}

void disable_sensor() {
  GPIO_PinOutClear(sensor_port, sensor_pin);
}

void gpioSetDisplayExtcomin(bool value) {

  if(value == true) {
      GPIO_PinOutSet(lcd_port, lcd_pin);
  }
  else {
      GPIO_PinOutClear(lcd_port, lcd_pin);
  }
=======
void gpioToggleLED0()
{
  static  bool on=false;
  if (on == false) {
    on = true;
    GPIO_PinOutSet(LED0_port,LED0_pin);
  } else {
    on = false;
    GPIO_PinOutClear(LED0_port,LED0_pin);
  }
} // gpioToggleLED0(0


//void gpioSensorEnSetOn()
//{
//  GPIO_PinModeSet(SENSOR_PORT, SENSOR_ENABLE, gpioModePushPull, on_off);
//}

//Enables/Disables I2C sensor
void gpio_I2C(int on_off){
  GPIO_PinModeSet(SENSOR_PORT, SENSOR_ENABLE, gpioModePushPull, on_off);

}

//toggle the EXTCOMIN pin for operation of LCD
void gpioSetDisplayExtcomin(bool value)
{
  GPIO_PinModeSet(DISP_PORT, DISP_EXTCOMIN, gpioModePushPull, value);
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
}




<<<<<<< HEAD
=======

>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
