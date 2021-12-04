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

#define SI7021_DEVICE_ADDR 0x40
#define APDS9960_DEVICE_ADDR 0x39

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
  uint8_t cmd_data = 0xF3;
  transfer_seq.addr = SI7021_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_WRITE;
  transfer_seq.buf[0].data = &cmd_data;
  transfer_seq.buf[0].len = sizeof(cmd_data);

  //enable I2C interrupt
  NVIC_EnableIRQ(I2C0_IRQn);

  //initialize I2C transfer
  transferStatus = I2C_TransferInit(I2C0, &transfer_seq);

  //check transfer function return status
  if(transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
  }
}

uint32_t write_read(uint8_t reg, uint8_t *data) {
  uint8_t cmd_data[1];
  I2C_TransferReturn_TypeDef transferStatus;

  cmd_data[0] = reg;

  //structure to write command from master to slave
    transfer_seq.addr = APDS9960_DEVICE_ADDR<<1;
    transfer_seq.flags = I2C_FLAG_WRITE_READ;
    transfer_seq.buf[0].data = cmd_data;
    transfer_seq.buf[0].len = 1;
    transfer_seq.buf[1].data = data;
    transfer_seq.buf[1].len = 1;

    //enable I2C interrupt
    //  NVIC_EnableIRQ(I2C0_IRQn);

      //initialize I2C transfer
      transferStatus = I2CSPM_Transfer(I2C0, &transfer_seq);

      //check transfer function return status
      if(transferStatus != i2cTransferDone) {
          LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
          *data = 0xff;
          return (uint32_t)transferStatus;
      }

      return (uint32_t)1;
}

uint32_t write_write(uint8_t reg, uint8_t data) {
  uint8_t cmd_data[2];
  uint8_t no_data[1];
  I2C_TransferReturn_TypeDef transferStatus;

  cmd_data[0] = reg;
  cmd_data[1] = data;

  //structure to write command from master to slave
    transfer_seq.addr = APDS9960_DEVICE_ADDR << 1;
    transfer_seq.flags = I2C_FLAG_WRITE;
    transfer_seq.buf[0].data = cmd_data;
    transfer_seq.buf[0].len = 2;
    transfer_seq.buf[1].data = no_data;
    transfer_seq.buf[1].len  = 0;

    //enable I2C interrupt
    //  NVIC_EnableIRQ(I2C0_IRQn);

      //initialize I2C transfer
      transferStatus = I2CSPM_Transfer(I2C0, &transfer_seq);

      //check transfer function return status
      if(transferStatus != i2cTransferDone) {
          LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
          return (uint32_t)transferStatus;
      }

      return (uint32_t)1;
}

int ReadDataBlock(uint8_t reg, uint8_t *data, uint8_t length)
{
  I2C_TransferReturn_TypeDef transferStatus;
  uint8_t cmd_data[1];

  transfer_seq.addr = APDS9960_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_WRITE_READ;
  /* Select register to start reading from */
  cmd_data[0] = reg;
  transfer_seq.buf[0].data = cmd_data;
  transfer_seq.buf[0].len  = 1;
  /* Select length of data to be read */
  transfer_seq.buf[1].data = data;
  transfer_seq.buf[1].len  = length;

  transferStatus = I2CSPM_Transfer(I2C0, &transfer_seq);
  if (transferStatus != i2cTransferDone)
  {
    return (int)transferStatus;
  }
  return (int)length;
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

  //enable I2C interrupt
  NVIC_EnableIRQ(I2C0_IRQn);

  //initialize I2C transfer
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
