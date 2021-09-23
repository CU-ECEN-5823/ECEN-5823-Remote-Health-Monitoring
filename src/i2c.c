/*
 * i2c.c
 *
 *  Created on: Sep 16, 2021
 *      Author: salon
 */
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "src/i2c.h"
#include "app.h"

#include "em_i2c.h"
#include "main.h"

#define SI7021_DEVICE_ADDR 0x40


uint8_t cmd_data;
uint8_t read_data[2];
I2C_TransferSeq_TypeDef transfer_seq;


void i2c_init() {

  //struct for i2c initialization
  I2CSPM_Init_TypeDef I2C_Config = {
      .port = I2C0,
      .sclPort = gpioPortC,
      .sclPin =  10,
      .sdaPort = gpioPortC,
      .sdaPin = 11,
      .portLocationScl = 14,
      .portLocationSda = 16,
      .i2cRefFreq = 0,
      .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
      .i2cClhr = i2cClockHLRStandard
  };

  //initialize i2c peripheral
  I2CSPM_Init(&I2C_Config);

}

//function to perform write command operation on slave
void write_cmd() {
  I2C_TransferReturn_TypeDef transferStatus;

  i2c_init();

  //structure to write command from master to slave
  cmd_data = 0xF3;
  transfer_seq.addr = SI7021_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_WRITE;
  transfer_seq.buf[0].data = &cmd_data;
  transfer_seq.buf[0].len = sizeof(cmd_data);

  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit(I2C0, &transfer_seq);

  //check transfer function return status
  if(transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
  }
}

//function to perform read operation from slave
void read_cmd() {
  I2C_TransferReturn_TypeDef transferStatus;

  i2c_init();

  //structure to read temperature measurement from slave
  transfer_seq.addr = SI7021_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_READ;
  transfer_seq.buf[0].data = &read_data[0];
  transfer_seq.buf[0].len = sizeof(read_data);

  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit(I2C0, &transfer_seq);

  //check transfer function return status
  if(transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit status %d read: failed\n\r", (uint32_t)transferStatus);
  }
}

//function to convert data read from sensor into temperature value in Celcius
float convertTemp() {

  float tempCelcius;
  uint16_t read_temp;
  read_temp = (read_data[0] << 8);
  read_temp += read_data[1];
  tempCelcius = (175.72*(read_temp));
  tempCelcius /= 65536;
  tempCelcius -= 46.85;
  return tempCelcius;
}

/*
//This function will be called from app_process_action everytime LETIMER interrupt is serviced i.e., every 3 seconds
//function to execute sequence for load power management and i2c communication with sensor
void read_temp_from_si7021() {

  float sensor_temp;

  //enable temperature sensor
  enable_sensor();

  //wait for 90ms
  timerWaitUs_interrupt(80000);

  //initialize i2c peripheral
  i2c_init();

  //write command to slave to get temperature measurement
  write_cmd();

  //wait 12ms for measurement
  timerWaitUs_interrupt(11000);

  //initialize i2c peripheral
  i2c_init();

  //read temperature value from slave
  read_cmd();

  //disable sensor
  disable_sensor();

  //convert sensor data into temperature
  sensor_temp = convertTemp();

  //log temperature value
  LOG_INFO("Temp = %f C\n\r", sensor_temp);
} */


