#ifndef __DS1307_2_H_INCLUDED__
#define __DS1307_2_H_INCLUDED__

#include <io.h>
#include <ds1307_2.c>

uint8_t DS1307Read(uint8_t address,uint8_t *data);
uint8_t DS1307Write(uint8_t address,uint8_t data);

#endif
