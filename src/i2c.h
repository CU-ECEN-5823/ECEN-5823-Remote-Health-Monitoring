/*
 * i2c.h
 *
 *  Created on: Sep 16, 2021
 *      Author: salon
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

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

#endif /* SRC_I2C_H_ */
