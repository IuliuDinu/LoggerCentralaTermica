#ifndef __I2C2_H_INCLUDED__
#define __I2C2_H_INCLUDED__

#define TRUE 1
#define FALSE 0

#include <I2C2.c>

void I2CInit();
void I2CClose();

void I2CStart();
void I2CStop();

uint8_t I2CWriteByte(uint8_t data);
uint8_t I2CReadByte(uint8_t *data,uint8_t ack);	

#endif



