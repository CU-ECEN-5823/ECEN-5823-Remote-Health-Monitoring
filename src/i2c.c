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

I2C_TransferReturn_TypeDef transferStatus;
uint8_t cmd_data;
uint8_t read_data[2];
I2C_TransferSeq_TypeDef write_seq;
I2C_TransferSeq_TypeDef read_seq;

void i2c_init() {

  cmd_data = 0xF3;
  write_seq.addr = SI7021_DEVICE_ADDR << 1;
  write_seq.flags = I2C_FLAG_WRITE;
  write_seq.buf[0].data = &cmd_data;
  write_seq.buf[0].len = sizeof(cmd_data);

  read_seq.addr = SI7021_DEVICE_ADDR << 1;
  read_seq.flags = I2C_FLAG_READ;
  read_seq.buf[0].data = &read_data[0];
  read_seq.buf[0].len = sizeof(read_data);

  I2CSPM_Init(&I2C_Config);
  //uint32_t i2c_bus_frequency = I2C_BusFreqGet(I2C0);

}

void write_cmd() {
  transferStatus = I2CSPM_Transfer(I2C0, &write_seq);
  if(transferStatus != i2cTransferDone) {
      LOG_ERROR("I2CSPM_Transfer status %d write: failed\n\r", (uint32_t)transferStatus);
  }
}

void read_cmd() {
  transferStatus = I2CSPM_Transfer(I2C0, &read_seq);
  if(transferStatus != i2cTransferDone) {
      LOG_ERROR("I2CSPM_Transfer %d read: failed\n\r", (uint32_t)transferStatus);
  }
}

float convertTemp() {

  float tempCelcius;
  uint16_t read_temp;
  read_temp = (read_data[0] <<8 );
  read_temp += read_data[1];
  tempCelcius = (175.72*(read_temp));
  tempCelcius /= 65536;
  tempCelcius -= 46.85;
  return tempCelcius;
}

void read_temp_from_si7021() {

  float sensor_temp;
  enable_sensor();
  I2CSPM_Init(&I2C_Config);

  timerWaitUs(100000);

  write_cmd();

  timerWaitUs(20000);

  read_cmd();

  disable_sensor();
  sensor_temp = convertTemp();
  LOG_INFO("Temp = %f C\n\r", sensor_temp);
}


