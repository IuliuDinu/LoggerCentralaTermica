#include <iuliu_lcd.h>

void LCD_GoToLineColumn(int x, int y)
{
	lcd_gotoxy(y,x);
}