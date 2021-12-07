/*
 * i2c.h
<<<<<<< HEAD
 *
 *  Created on: Sep 16, 2021
 *      Author: salon
=======
 * This file contains function prototypes used for i2c operations.Check .c file for function definition.
 *  Created on: Sep 15, 2021
 *      Author: mich1576
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

<<<<<<< HEAD
#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "app.h"

#include "em_letimer.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "main.h"

//function to initialize I2C peripheral
void i2c_init();

//function to convert data in temperature value
float convertTemp();

//function to write a command to slave
void write_cmd();

uint32_t write_read(uint8_t reg, uint8_t *data);

uint32_t write_write(uint8_t reg, uint8_t data);

int ReadDataBlock(uint8_t reg, uint8_t *data, uint8_t length);

void I2C_pulse_write_polled(uint8_t* command, int array_len);

void I2C_pulse_read_polled();

void read_return_check();

int extract_data();

//function to read temperature from sensor
void read_cmd();
=======
//includes
#include "sl_i2cspm.h"
#include "timers.h"

//Defining the device address and the port for enabling the sensor
#define SI7021_DEVICE_ADDR 0x40

//Defining the port used for I2C0 and the pins used for SCL and SDA
#define I2C_port            gpioPortC
#define SCL_pin             10
#define SDA_pin             11
#define SCL_PORT_LOCATION   14
#define SDA_PORT_LOCATION   16

//function prototypes

/* function     : I2C_init
 * params       : none
 * brief        : Initializes the I2CSPM_Init_TypeDef structure
 * return_type  : void
 * */
void I2C_init();

/* function     : I2C_write
 * params       : none
 * brief        : Used to perform I2C write.
 *                Sends start bit, slave address, send write command
 *                wait for acknowledgment, send measure command,
 *                wait for acknowledgment
 * return_type  : void
 * */
void I2C_write();

/* function     : I2C_read
 * params       : none
 * brief        : Used to perform I2C read.
 *                Sends repeated start bit, slave address, send read command
 *                read MS and LS byte
 * return_type  : void
 * */
void I2C_read();

/* function     : warmup()
 * params       : none
 * brief        : Enable the sensor
 * return_type  : void
 * */
void warmup();

/* function     : turnoff
 * params       : none
 * brief        : Disable the sensor
 * return_type  : void
 * */
void turnoff();

/* function     : store
 * params       : none
 * brief        : Convert and log the temperature
 * return_type  : int
 * */
int store();
>>>>>>> 88f2a69d6c34b4934217767d18295fca71d5373e

#endif /* SRC_I2C_H_ */
