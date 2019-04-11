#ifndef __ALEMELE_INCLUDED__
#define __ALEMELE_INCLUDED__

#include <alemele.c>

void CPU_Init();
void CPU_Init_1M_LCD_RTC_TIM_0_INTERNAL();
void CPU_Init_1M_LCD_TIM0INT_EXT_INT0();
void CPU_Init_1M_LCD_RTC_TIM0INT_EXT1_ENABLED();

void BLINK_LED0(int time_on, int time_off);
void BLINK_LED1(int time_on, int time_off);
void BLINK_LED2(int time_on, int time_off);
void BLINK_LED2(int time_on, int time_off);

void ShowTheTime(char coord_x, char coord_y, char h, char m, char s);

#endif