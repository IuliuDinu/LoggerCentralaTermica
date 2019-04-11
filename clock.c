/******************************************************************************

                          eXtreme Electronics xAPI(TM)
						  ----------------------------
xAPI is a Powerful but easy to use C library to program the xBoard(TM)
series of AVR development board. The library has support for commonly use tasks
like:-

*LCD interfacing
*MUXED 7 segment displays.
*Remote Control
*Serial Communication
*DC Motor Controls with Speed Control
*Analog Measurement for Sensor Interface.
*Temperature Measurement.
*I2C Communication.
*EEPROM Interface
*Real Time Clock (RTC Interface)

The APIs are highly documented and easy to use even by a beginner.

For More Info Log On to 
www.eXtremeElectronics.co.in

Copyright 2008-2014 eXtreme Electronics India

                                    Clock Core
						           ----------
This module is used for interfacing with DS1307 based clock module.
The library provide easy to use to use functions to get and set time.

For More information please see supplied tutorials and videos.

                                     NOTICE
									--------
NO PART OF THIS WORK CAN BE COPIED, DISTRIBUTED OR PUBLISHED WITHOUT A
WRITTEN PERMISSION FROM EXTREME ELECTRONICS INDIA. THE LIBRARY, NOR ANY PART
OF IT CAN BE USED IN COMMERCIAL APPLICATIONS. IT IS INTENDED TO BE USED FOR
HOBBY, LEARNING AND EDUCATIONAL PURPOSE ONLY. IF YOU WANT TO USE THEM IN 
COMMERCIAL APPLICATION PLEASE WRITE TO THE AUTHOR.


WRITTEN BY:
AVINASH GUPTA
me@avinashgupta.com

*******************************************************************************/

#include <io.h>
#include <delay.h>
#include <alemele.h>
#include <iuliu_lcd.h>

#include "i2c2.h"
#include "ds1307_2.h"
#include "clock.h"


/***************************************

Setup the DS1307 Chip. Start it and set
12 Hr Mode.
Must be called before any other clock
related functions.

Parameters
----------
NONE

Return Value
------------
0=Failed
1=Success

****************************************/
bool ClockInit()
{
	uint8_t temp;
    
    //Initialize I2C Bus
	I2CInit();

	//Clear CH bit of RTC
	#define CH 7

	
	if(!DS1307Read(0x00,&temp)) return 0;

	//Clear CH Bit
	temp&=(~(1<<CH));

	if(!DS1307Write(0x00,temp)) return 0;

	//Set 12 Hour Mode
	if(!DS1307Read(0x02,&temp)) return 0;

	//Set 12Hour BIT
	temp|=(0b01000000);

	//Write Back to DS1307
	if(!DS1307Write(0x02,temp)) return 0;

	return 1;

}
/***************************************

Get second

Parameters
----------
NONE

Return Value
------------
the "second" part of time.

****************************************/
uint8_t GetSecond()
{
	uint8_t sec,temp;

	//Read the Second Register
	DS1307Read(0x00,&temp);
	sec=(((temp & 0b01110000)>>4)*10)+(temp & 0b00001111);

	return sec;

	
}
/***************************************

Get minute

Parameters
----------
NONE

Return Value
------------
the "minute" part of time.

****************************************/

uint8_t GetMinute()
{
	uint8_t min,temp;

	//Read the Minute Register
	DS1307Read(0x01,&temp);
	min=(((temp & 0b01110000)>>4)*10)+(temp & 0b00001111);

	return min;
	
}

/***************************************

Get hour

Parameters
----------
NONE

Return Value
------------
the "hour" part of time.

****************************************/

uint8_t GetHour()
{
	uint8_t hr,temp;

	//Read the Hour Register
	DS1307Read(0x02,&temp);
	hr=(((temp & 0b00010000)>>4)*10)+(temp & 0b00001111);

	return hr;

}

/***************************************

Get am/pm

Parameters
----------
NONE

Return Value
------------
0=am
1=pm

****************************************/

uint8_t GetAmPm()
{
	uint8_t am_pm,temp;

	//Read the Hour Register
	DS1307Read(0x02,&temp);

	am_pm=(temp & 0b00100000)>>4;

	return am_pm;

}
/***************************************

Set the second

Parameters
----------
seconds

Return Value
------------
0=failure
1=success

****************************************/

bool SetSecond(uint8_t sec)
{
	uint8_t temp,result;

	temp=((sec/10)<<4)|(sec%10);
	result=DS1307Write(0x00,temp);

	return result;
}

/***************************************

Set the minutes

Parameters
----------
minutes

Return Value
------------
0=failure
1=success

****************************************/

bool SetMinute(uint8_t min)
{
	uint8_t temp,result;

	temp=((min/10)<<4)|(min%10);
	result=DS1307Write(0x01,temp);

	return result;
	
}

/***************************************

Set the hour

Parameters
----------
hour

Return Value
------------
0=failure
1=success

****************************************/

bool SetHour(uint8_t hr)
{
	uint8_t temp,result,am_pm;

	am_pm=GetAmPm();

	temp=((hr/10)<<4)|(hr%10);
	temp|=0b01000000; //12 Hr Mode
	
	if(am_pm)
	{
		temp|=0b00100000;
	}
	result=DS1307Write(0x02,temp);

	return result;
	
}

/***************************************

Set the second

Parameters
----------
0=am
1=pm

Return Value
------------
0=failure
1=success

****************************************/

bool SetAmPm(bool pm)
{
	uint8_t temp,result;

	DS1307Read(0x02,&temp);

	if(pm)
		temp|=0b00100000;//SET
	else
		temp&=0b11011111;//CLEAR

	result=DS1307Write(0x02,temp);

	return result;
			
}

/***************************************

Gets the current time as a ascii/text string - 24 hours format.

Example 00:49:22 (HH:MM:SS)


Parameters
----------
Pointer to a string.

Return Value
------------
0=failure
1=success

Example Usage
-------------

char time[9];			//The string must be at least 9bytes long
GetTimeString(time);	//Now the string time contains the current time


****************************************/

bool GetTimeString24H(char *Time)
{
	uint8_t data;
    char hrs[3];

	// read seconds
    if(!DS1307Read(0x00,&data))
	{
		//I/O Error
		return 0;
	}

	Time[8]='\0';

	Time[7]=48+(data & 0b00001111);
	Time[6]=48+((data & 0b01110000)>>4);
	Time[5]=':';

	// read minutes
    if(!DS1307Read(0x01,&data))
	{
		//I/O Error
		return 0;
	}
	
	Time[4]=48+(data & 0b00001111);
	Time[3]=48+((data & 0b01110000)>>4);
	Time[2]=':';
	
	// read hours
    if(!DS1307Read(0x02,&data))
	{
		//I/O Error
		return 0;
	}
    
    //Time[1]=48+(data & 0b00001111);
	//Time[0]=48+((data & 0b00010000)>>4);
    hrs[1] = 48+(data & 0b00001111);
    hrs[0] = 48+((data & 0b00010000)>>4);   
    
    //AM/PM
	if(data & 0b00100000) // PM
	{
		//Time[9]='P';
        uint8_t hr_u8=atoi(hrs);
        if (hr_u8 != 12)
        {    hr_u8 += 12; }
        itoa(hr_u8,hrs);
        Time[1] = hrs[1];
        Time[0] = hrs[0];
	}
	else // AM
	{
		//Time[9]='A';
        uint8_t hr_u8=atoi(hrs);
        if(hr_u8 == 12)
        {
            Time[1] = '0';
            Time[0] = '0';
        }
        else
        {
            Time[1] = hrs[1];
            Time[0] = hrs[0];    
        }
	}

	return 1;//Success

}

/***************************************

Gets the current time as a ascii/text string - 12 hours format.

Example 12:49:22 AM (HH:MM:SS AM)


Parameters
----------
Pointer to a string.

Return Value
------------
0=failure
1=success

Example Usage
-------------

char time[12];			//The string must be at least 12bytes long
GetTimeString(time);	//Now the string time contains the current time


****************************************/

bool GetTimeString12H(char *Time)
{
	uint8_t data;
    
	// read seconds
    if(!DS1307Read(0x00,&data))
	{
		//I/O Error
		return 0;
	}

	Time[11]='\0';
	Time[10]='M';
	Time[8]=' ';

	Time[7]=48+(data & 0b00001111); // mask units of seconds
	Time[6]=48+((data & 0b01110000)>>4); // mask tens of seconds
	Time[5]=':';

	// read minutes
    if(!DS1307Read(0x01,&data))
	{
		//I/O Error
		return 0;
	}
	
	Time[4]=48+(data & 0b00001111);
	Time[3]=48+((data & 0b01110000)>>4);
	Time[2]=':';
	
	// read hours
    if(!DS1307Read(0x02,&data))
	{
		//I/O Error
		return 0;
	}
    
    Time[1]=48+(data & 0b00001111);
	Time[0]=48+((data & 0b00010000)>>4);   
    
	//AM/PM
	if(data & 0b00100000)
	{
		Time[9]='P';
	}
	else
	{
		Time[9]='A';
	}

	return 1;//Success

}

/***************************************

Gets the current time as a ascii/text string - 24 hours format,
by altering some global parameters (char) hrs,mins,secs, not a "Time" string

Example 00:49:22 (HH:MM:SS)


Parameters
----------
Pointer to a string.

Return Value
------------
0=failure
1=success

Example Usage
-------------

char hrs,mins,secs;                     //Separate numeric parameters for hours, minutes, seconds 
GetTimeString24H_2(&hrs, &mins, &secs);	//Now the three parameters contain the current time


****************************************/

bool GetTimeString24H_2(char *hrs, char *mins, char *secs)
{
	uint8_t data;

	// read seconds
    if(!DS1307Read(0x00,&data))
	{
		//I/O Error
		return 0;
	}

    *secs = (10 * ((data & 0b01110000)>>4) ) + (data & 0b00001111);
    //secs = data; 

	// read minutes
    if(!DS1307Read(0x01,&data))
	{
		//I/O Error
		return 0;
	}
	
    *mins = 10 * ((data & 0b01110000)>>4) + (data & 0b00001111);
 	
	// read hours
    if(!DS1307Read(0x02,&data))
	{
		//I/O Error
		return 0;
	}
    
    *hrs = 10 * ((data & 0b00010000)>>4) + (data & 0b00001111);   
    
    //AM/PM
	if(data & 0b00100000) // PM
	{
        if (*hrs != 12)
        {    *hrs += 12; }
	}
	else // AM
	{
        if(*hrs == 12)
        {   *hrs = 0;    }
	}

	return 1;//Success
}

bool SetYear(uint8_t yr)
{
	uint8_t temp,result;

	temp=((yr/10)<<4)|(yr%10);

	result=DS1307Write(0x06,temp);

	return result;
	
}

bool SetMonth(uint8_t mo)
{
	uint8_t temp,result;

	temp=((mo/10)<<4)|(mo%10);

	result=DS1307Write(0x05,temp);

	return result;
	
}

bool SetDay(uint8_t day)
{
	uint8_t temp,result;

	temp=((day/10)<<4)|(day%10);

	result=DS1307Write(0x04,temp);

	return result;
	
}

bool SetDOW(uint8_t dow)
{
	uint8_t temp,result;

	temp=dow;

	result=DS1307Write(0x03,temp);

	return result;    
}

uint8_t GetDay()
{
	uint8_t day,temp;

	//Read the Date Register
	DS1307Read(0x04,&temp);
	day=(((temp & 0b00110000)>>4)*10)+(temp & 0b00001111);

	return day;
}

uint8_t GetMonth()
{
	uint8_t month,temp;

	//Read the Hour Register
	DS1307Read(0x05,&temp);
	month=(((temp & 0b00010000)>>4)*10)+(temp & 0b00001111);

	return month;

}

uint8_t GetYear()
{
	uint8_t year,temp;

	//Read the Hour Register
	DS1307Read(0x06,&temp);
	year=(((temp & 0b11110000)>>4)*10)+(temp & 0b00001111);

	return year;

}

uint8_t GetDOW()
{
	uint8_t dow,temp;

	//Read the Hour Register
	DS1307Read(0x03,&temp);
	dow = temp & 0b00000111;

	return dow;

}

bool GetDateString(char *Date)
{
	uint8_t data;
    
    Date[8]='\0';
    
	// read year
    if(!DS1307Read(0x06,&data))
	{
		//I/O Error
		return 0;
	}
        
	Date[7]=48+(data & 0b00001111);
	Date[6]=48+((data & 0b11110000)>>4);
	Date[5]='.';

	// read months
    if(!DS1307Read(0x05,&data))
	{
		//I/O Error
		return 0;
	}
	
	Date[4]=48+(data & 0b00001111);
	Date[3]=48+((data & 0b00010000)>>4);
	Date[2]='.';
	
	// read days
    if(!DS1307Read(0x04,&data))
	{
		//I/O Error
		return 0;
	}
    
    Date[1]=48+(data & 0b00001111);
	Date[0]=48+((data & 0b00110000)>>4);   
    

	return 1;//Success

}

uint8_t PREV_PINC=0xFF;