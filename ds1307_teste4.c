#include <io.h>
#include <iuliu_lcd.h>
#include <mega8.h>
#include <inttypes.h>
#include <delay.h>
#include <stdlib.h>
#include <I2C2.h>
#include <ds1307_2.h>
#include <clock.h>
#include <alemele.h>

#define BTN_RIGHT 1-PINC.0
#define BTN_OK 1-PINC.2
#define BTN_LEFT 1-PINC.3

#define MENU_0_IDLE 0
#define MENU_1_SET_HOUR 1
#define MENU_2_SET_DATE 2
#define MENU_3_READ_TODAY 3
#define MENU_4_READ_DAILY 4
#define MENU_5_READ_MONTHLY 5
#define MENU_6_RESET_EEPROM 6

#define MAX_NB_OF_ENTRIES_PER_DAY 85
#define MAX_NB_OF_DAYS_IN_A_MONTH 31

#define ONE_SECOND 16

//char Time[12]; // for 12H display
char Time[9];
char Time1[9];
char Time2[9];
char Date[9];
char buffer[4];
//int i=0;
char displayRefresh=0;
char menuIcons[5] = {127,32,219,32,126};
//char menuIconsEnd[5] = {127,32,219,32,32};
char menuNumber = 1;
char menuTimer = 0;
char semisec = 0;
//char actualSetting = 0;
//char firstDisplay = 1;
char inputRelayStartStop = 0;
char inputRelayStartStop_Old = 0;
char to_be_saved_due_to_date_set;
int for_i;


enum State 
{STATE_IDLE=0,
STATE_MENU, 
STATE_SET_HOUR, 
STATE_SET_DATE, 
STATE_READ_ONE_DAY,
STATE_READ_STATISTICS_1_MONTH,
STATE_READ_STATISTICS_MONTHLY,
STATE_RESET_EEPROM} actualState;

struct clock_type
       {
            int hour;
            int minute;
            int second;
            int ampm;
            int year;
            int month;
            int day;
            int dow;
       } clock;
       
eeprom char lista_durate_1zi[86];
eeprom char lista_porniri_1zi[256];
eeprom int total_durate_zi[31];
eeprom char total_porniri_zi[32];
eeprom int total_durate_luna[6];
eeprom int total_porniri_luna[6];
eeprom char numar_luna[7];
eeprom char indexes[3];
//eeprom char indexes[7] = {0, 0, 0, 0, 0, 0, '\0'};
eeprom char flag_indexes0_ovf;
eeprom char start_day;
eeprom char stop_day;
eeprom char today;
eeprom char this_month;
eeprom char this_month_old;
eeprom char day_saved;
eeprom char month_saved;
// rest 34 char

// External Interrupt 1 service routine
interrupt [EXT_INT1] void ext_int1_isr(void)
{
    inputRelayStartStop++;

}

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    //i++;
    displayRefresh++;
    menuTimer++;
}

void RESET_EEPROM_char_array(eeprom char *array, char size)
{
    int i;
    for (i=0; i < size-1; i++)
        array[i] = 0;        
}

void RESET_EEPROM_int_array(eeprom int *array, char size)
{
    int i;
    for (i=0; i < size; i++)
        array[i] = 0;    
}

void Save1DayData(char decrement)
{
    //unsigned char current_day;
    int i, temp = 0;
    //current_day = GetDay();
    if (flag_indexes0_ovf == 0) // no overflow occured
    {
        for (i=0; i < indexes[0]; i++)
            temp += lista_durate_1zi[i];
        total_durate_zi[start_day-1-decrement] = temp;
        total_porniri_zi[start_day-1-decrement] = indexes[0];        
    }
    else // overflow occured   
    {
        for (i=0; i < indexes[0]; i++)
            temp += lista_durate_1zi[i];
        total_durate_zi[start_day-1-decrement] += temp;
        total_porniri_zi[start_day-1-decrement] += indexes[0];
        flag_indexes0_ovf = 0; // reset overflow flag   
    }   
  
}

void Sum1DayData()
{
    int i, temp = 0;
    for (i=0; i < MAX_NB_OF_ENTRIES_PER_DAY; i++)
        temp += lista_durate_1zi[i];
    total_durate_zi[start_day-1] = temp;
    total_porniri_zi[start_day-1] = indexes[0];        
}

void Save1MonthData(int month)
{
    int i, temp = 0;
    
    for (i=0; i < MAX_NB_OF_DAYS_IN_A_MONTH; i++)
        temp += total_durate_zi[i];
    total_durate_luna[indexes[1]] = temp;
    
    temp = 0;
    
    for (i=0; i < MAX_NB_OF_DAYS_IN_A_MONTH  ; i++)
        temp += total_porniri_zi[i];
    total_porniri_luna[indexes[1]] = temp;
    
    numar_luna[indexes[1]] = month;

    if (numar_luna[indexes[1]] == 0)    
    {
        numar_luna[indexes[1]] = 12;
    }

    //indexes[1]++; // To be done where this function is called
}



char TimeDiffInMinutes(char *Time1, char *Time2)
{
    char hours[3],minutes[3],seconds[3];
    char hours1,hours2,minutes1,minutes2,seconds1,seconds2;
    hours[0]=Time2[0]; hours[1]=Time2[1];
    hours2 = atoi(hours);
    hours[0]=Time1[0]; hours[1]=Time1[1];
    hours1 = atoi(hours);
    if (hours2 < hours1)    
    {    hours2 += 24; }
    hours2 -= hours1;   // hours2 keeps the number of hours
    minutes[0]=Time2[3]; minutes[1]=Time2[4];
    minutes2 = atoi(minutes);
    minutes[0]=Time1[3]; minutes[1]=Time1[4];
    minutes1 = atoi(minutes);
    if (minutes2 < minutes1)
    {
        minutes2 += 60;
        hours2 -= 1;
    }
    minutes2 -= minutes1;   // minutes2 keeps the number of minutes
    seconds[0]=Time2[6]; seconds[1]=Time2[7];
    seconds2 = atoi(seconds);
    seconds[0]=Time1[6]; seconds[1]=Time1[7];
    seconds1 = atoi(seconds);
    if (seconds2 < seconds1)
    {
        seconds2 += 60;
        minutes2 -= 1;
    }
    seconds2 -= seconds1;
    if (seconds2 > 30)
        minutes2 += 1;
    minutes2 += hours2*60;
    
    return minutes2;   
    
}

char TimeDiffInMinutes2(char hrs1, char mins1, char secs1, char *hrs2, char *mins2, char *secs2)
{
    char hours[3],minutes[3],seconds[3];
    char hours1,hours2,minutes1,minutes2,seconds1,seconds2;
    
    if (*hrs2 < hrs1)    
    {    *hrs2 += 24; }
    *hrs2 -= hrs1;   // hrs2 keeps the number of hours
    
    if (*mins2 < mins1)
    {
        *mins2 += 60;
        *hrs2 -= 1;
    }
    *mins2 -= mins1;   // mins2 keeps the number of minutes
    
    if (*secs2 < secs1)
    {
        *secs2 += 60;
        *mins2 -= 1;
    }
    *secs2 -= secs1;
    
    if (*secs2 > 30)
        *mins2 += 1;
    *mins2 += *hrs2*60;
    
    return *mins2;   
    
}

void ProcessInputRelay_ON()
{
    char hour,minute,second,ampm;
    
    start_day = GetDay(); // incepe mereu "azi" sau "maine"
    
    /*if (stop_day != start_day ) // last record
    {
        Save1DayData(1);
        indexes[0] = 0;
    }*/
    
    GetTimeString24H(Time1); // for calculating the difference when relay stops
    lcd_clear();
    LCD_GoToLineColumn(0,0);
    lcd_puts("O pornit!");
    delay_ms(300);
    inputRelayStartStop_Old = 1;
    
    hour = GetHour();
    minute = GetMinute();
    second = GetSecond();
    ampm = GetAmPm();
    
    //if ((ampm != 0) && (hour == 11) && (minute > 50)) - aparent nu mai e nevoie
    
    if ((ampm != 0) && (hour != 12))
        lista_porniri_1zi[3*indexes[0]] = hour+12;
    else 
    {
        if ((ampm == 0) && (hour == 12))
            lista_porniri_1zi[3*indexes[0]] = 0;
        else
            lista_porniri_1zi[3*indexes[0]] = hour;     // 3k - the positions in lista_porniri_1zi[] for hour values
    }
    lista_porniri_1zi[(3*indexes[0])+1] = minute;       // 3k+1 - the positions in lista_porniri_1zi[] for minute values
    lista_porniri_1zi[(3*indexes[0])+2] = second;       // 3k+2 - the positions in lista_porniri_1zi[] for second values
    
    indexes[0]++; //TBD in ProcessInputRelay_OFF? What to do if MCU resets (ie power loss) during Relay=ON ?   
}

void ProcessInputRelay_OFF()
{   // se termina fie "azi", fie "maine"; se retine valoarea zilei de "azi" pana la finalul functiei
    char minutes_of_work;
    char buffer[4];
    
    GetTimeString24H(Time2); // for calculating the difference when relay stops
    lcd_clear();
    LCD_GoToLineColumn(0,0);
    lcd_puts("S-o oprit!");
    minutes_of_work = TimeDiffInMinutes(Time1,Time2);
    LCD_GoToLineColumn(1,0);
    itoa(minutes_of_work,buffer);
    lcd_puts(buffer);
    delay_ms(100);
    inputRelayStartStop = 0;
    inputRelayStartStop_Old = 0;
    
    lista_durate_1zi[indexes[0]-1] = minutes_of_work;
    //indexes[0]++;
    
    stop_day = GetDay();
    
    if (stop_day != start_day) // stops after midnight --> last record
    {
        Save1DayData(0);
        indexes[0] = 0;
    }
    
    if (indexes[0] > (MAX_NB_OF_ENTRIES_PER_DAY - 1)) // more than 85 records
    {
        Sum1DayData(); // fa backup la suma partiala - insumarea duratelor si numarului de porniri continua, dar lista de porniri si durate se suprascrie
        indexes[0] = 0;        
        flag_indexes0_ovf = 1;
    }
    
    if (day_saved == 1)
    day_saved = 0;        
    
}

void f_SetHour(void)
{
    static char hrs[3],mins[3],secs[3];
    static char actualSetting = 0;
    static char actualSettingNext = 0;
    static char firstDisplay = 1;
    if (firstDisplay == 1)
    {
        GetTimeString24H(Time);
        hrs[0]=Time[0]; hrs[1]=Time[1];
        mins[0]=Time[3]; mins[1]=Time[4];
        secs[0]=Time[6]; secs[1]=Time[7];
        clock.hour = atoi(hrs);
        clock.minute = atoi(mins);
        clock.second = atoi(secs);
        itoa(clock.hour,hrs);
        itoa(clock.minute,mins);
        itoa(clock.second,secs);
      
        lcd_clear();
        LCD_GoToLineColumn(0,0);
        if (clock.hour <10)
        {
            lcd_putchar('0');
            LCD_GoToLineColumn(0,1);
            lcd_putchar(hrs[0]);
        }
        else
            lcd_puts(hrs);
        LCD_GoToLineColumn(0,2);
        lcd_putchar(':');
        LCD_GoToLineColumn(0,3);
        if (clock.minute < 10)
        {
            lcd_putchar('0');
            LCD_GoToLineColumn(0,4);
            lcd_putchar(mins[0]);
        }
        else
            lcd_puts(mins);
        LCD_GoToLineColumn(0,5);
        lcd_putchar(':');
        LCD_GoToLineColumn(0,6);
        if (clock.second < 10)
        {
            lcd_putchar('0');
            LCD_GoToLineColumn(0,7);
            lcd_putchar(secs[0]);        
        }
        else
            lcd_puts(secs);
        
        PORTD.0 = 1;
        delay_ms(2000);
        PORTD.0 = 0;
        firstDisplay = 0;
    }
    
    if ((actualSetting == 0) && (displayRefresh>4))
    {
        if (semisec%2 == 0)
        {
           LCD_GoToLineColumn(0,0);
           lcd_puts("  "); 
        }
        else
        {
            LCD_GoToLineColumn(0,0);
            if (clock.hour <10)
            {
                lcd_putchar('0');
                LCD_GoToLineColumn(0,1);
                lcd_putchar(hrs[0]);
            }
            else
                lcd_puts(hrs);
            if (actualSettingNext > actualSetting)
                actualSetting=actualSettingNext;
        }
        semisec++;
        displayRefresh=0;
    }
    
    if ((actualSetting == 1) && (displayRefresh>4))
    {
        if (semisec%2 == 0)
        {
           LCD_GoToLineColumn(0,3);
           lcd_puts("  "); 
        }
        else
        {
            LCD_GoToLineColumn(0,3);
            if (clock.minute < 10)
            {
                lcd_putchar('0');
                LCD_GoToLineColumn(0,4);
                lcd_putchar(mins[0]);
            }
            else
                lcd_puts(mins);
            if (actualSettingNext > actualSetting)
                actualSetting=actualSettingNext;
        }
        semisec++;
        displayRefresh=0;
    }
    
        if ((actualSetting == 2) && (displayRefresh>4))
    {
        if (semisec%2 == 0)
        {
           LCD_GoToLineColumn(0,6);
           lcd_puts("  "); 
        }
        else
        {
            LCD_GoToLineColumn(0,6);
            if (clock.second < 10)
            {
                lcd_putchar('0');
                LCD_GoToLineColumn(0,7);
                lcd_putchar(secs[0]);        
            }
            else
                lcd_puts(secs);
            if (actualSettingNext > actualSetting)
                actualSetting=actualSettingNext;
        }
        semisec++;
        displayRefresh=0;
    }
    
    if (BTN_LEFT == 1)
    {
        while (BTN_LEFT == 1) {}
        switch (actualSetting)
       {
       case 0:
            clock.hour--;
            if (clock.hour < 0)
                clock.hour = 23;
            itoa(clock.hour,hrs);
            break;
       case 1:
            clock.minute--;
            if (clock.minute < 0)
                clock.minute = 59;
            itoa(clock.minute,mins);
            break;
       case 2:
            clock.second--;
            if (clock.second < 0)
                clock.second = 59;
            itoa(clock.second,secs);
            break;
       default:
            actualState = STATE_MENU;
            return;
       }
       
       menuTimer = 0;
    }
    
    if (BTN_RIGHT == 1)
    {
        while (BTN_RIGHT == 1) {}
        switch (actualSetting)
       {
       case 0:
            clock.hour++;
            if (clock.hour > 23)
                clock.hour = 0;
            itoa(clock.hour,hrs);
            break;
       case 1:
            clock.minute++;
            if (clock.minute > 59)
                clock.minute = 0;
            itoa(clock.minute,mins);
            break;
       case 2:
            clock.second++;
            if (clock.second > 59)
                clock.second = 0;
            itoa(clock.second,secs);
            break;
       default:
            actualState = STATE_MENU;
            return;
       }
       
       menuTimer = 0;
    }
    
    if (BTN_OK == 1)
    {
        while (BTN_OK == 1) {}
        if (actualSetting == 2)
        {                
            if (clock.hour >= 12)
            {
                if (clock.hour == 12)
                {
                    SetHour(12);
                    SetAmPm(1); //PM
                }
                else
                {
                    SetHour(clock.hour-12);
                    SetAmPm(1); //PM
                }
            }
            else
            {
                SetHour(clock.hour);
                SetAmPm(0); //AM
            }
            SetMinute(clock.minute);
            SetSecond(clock.second);
            actualState = STATE_IDLE;
            firstDisplay = 1;
            actualSettingNext = 0;
            actualSetting = 0;
            delay_ms(2000);
            return;    
        }
        else
            actualSettingNext++;
    }
    
    if (menuTimer > (15*ONE_SECOND))
    {
        actualState = STATE_IDLE;
        firstDisplay = 1;
        return;
    }
    
    return;
      
}

void f_SetDate(void)
{
    static char yrs[3],mons[3],days[3];
    static char actualSetting = 0;
    static char actualSettingNext = 0;
    static char firstDisplay = 1;
    if (firstDisplay == 1)
    {
        GetDateString(Date);
        days[0]=Date[0]; days[1]=Date[1];
        mons[0]=Date[3]; mons[1]=Date[4];
        yrs[0]=Date[6]; yrs[1]=Date[7];
        clock.day = atoi(days);
        clock.month = atoi(mons);
        clock.year = atoi(yrs);
        itoa(clock.day,days);
        itoa(clock.month,mons);
        itoa(clock.year,yrs);
      
        lcd_clear();
        LCD_GoToLineColumn(1,0);
        if (clock.day <10)
        {
            lcd_putchar('0');
            LCD_GoToLineColumn(1,1);
            lcd_putchar(days[0]);
        }
        else
            lcd_puts(days);
        LCD_GoToLineColumn(1,2);
        lcd_putchar('.');
        LCD_GoToLineColumn(1,3);
        if (clock.month < 10)
        {
            lcd_putchar('0');
            LCD_GoToLineColumn(1,4);
            lcd_putchar(mons[0]);
        }
        else
            lcd_puts(mons);
        LCD_GoToLineColumn(1,5);
        lcd_putchar('.');
        LCD_GoToLineColumn(1,6);
        lcd_puts("20");
        LCD_GoToLineColumn(1,8);
        if (clock.year < 10)
        {
            lcd_putchar('0');
            LCD_GoToLineColumn(1,9);
            lcd_putchar(yrs[0]);        
        }
        else
            lcd_puts(yrs);
        
        BLINK_LED0(2000,0);
        firstDisplay = 0;
    }
    
    if ((actualSetting == 0) && (displayRefresh>4)) // day
    {
        if (semisec%2 == 0)
        {
           LCD_GoToLineColumn(1,0);
           lcd_puts("  "); 
        }
        else
        {
            LCD_GoToLineColumn(1,0);
            if (clock.day <10)
            {
                lcd_putchar('0');
                LCD_GoToLineColumn(1,1);
                lcd_putchar(days[0]);
            }
            else
                lcd_puts(days);
            if (actualSettingNext > actualSetting)
                actualSetting=actualSettingNext;
        }
        semisec++;
        displayRefresh=0;
    }
    
    if ((actualSetting == 1) && (displayRefresh>4))  // month
    {
        if (semisec%2 == 0)
        {
           LCD_GoToLineColumn(1,3);
           lcd_puts("  "); 
        }
        else
        {
            LCD_GoToLineColumn(1,3);
            if (clock.month < 10)
            {
                lcd_putchar('0');
                LCD_GoToLineColumn(1,4);
                lcd_putchar(mons[0]);
            }
            else
                lcd_puts(mons);
            if (actualSettingNext > actualSetting)
                actualSetting=actualSettingNext;
        }
        semisec++;
        displayRefresh=0;
    }
    
        if ((actualSetting == 2) && (displayRefresh>4)) // year
    {
        if (semisec%2 == 0)
        {
           LCD_GoToLineColumn(1,6);
           lcd_puts("    "); 
        }
        else
        {
            LCD_GoToLineColumn(1,6);
            lcd_puts("20");
            LCD_GoToLineColumn(1,8);
            if (clock.year < 10)
            {
                lcd_putchar('0');
                LCD_GoToLineColumn(1,9);
                lcd_putchar(yrs[0]);        
            }
            else
                lcd_puts(yrs);
            if (actualSettingNext > actualSetting)
                actualSetting=actualSettingNext;
        }
        semisec++;
        displayRefresh=0;
    }
    
    if (BTN_LEFT == 1)
    {
        while (BTN_LEFT == 1) {}
        switch (actualSetting)
       {
       case 0:
            clock.day--;
            if (clock.day < 1)
                    clock.day = 31;
            itoa(clock.day,days);
            break;
       case 1:
            clock.month--;
            if (clock.month < 1)
                clock.month = 12;
            itoa(clock.month,mons);
            break;
       case 2:
            clock.year--;
            if (clock.year < 0)
                clock.year = 99;
            itoa(clock.year,yrs);
            break;
       default:
            actualState = STATE_MENU;
            return;
       }
       
       menuTimer = 0;
    }
    
    if (BTN_RIGHT == 1)
    {
        while (BTN_RIGHT == 1) {}
        switch (actualSetting)
       {
       case 0:
            clock.day++;
            if (clock.day > 31)
                clock.day = 1;
            itoa(clock.day,days);
            break;
       case 1:
            clock.month++;
            if (clock.month > 12)
                clock.month = 1;
            itoa(clock.month,mons);
            break;
       case 2:
            clock.year++;
            if (clock.year > 99)
                clock.year = 0;
            itoa(clock.year,yrs);
            break;
       default:
            actualState = STATE_MENU;
            return;
       }
       
       menuTimer = 0;
    }
    
    if (BTN_OK == 1)
    {
        while (BTN_OK == 1) {}
        if (actualSetting == 2)
        {
            if ((clock.month == 2) && (clock.day > 29))
            {
                LCD_GoToLineColumn(0,0);
                lcd_puts("FEBRUARY > 29 ?!");
                delay_ms(2000);
            }
            else
            { 
                if (clock.day == 1)
                {
                    to_be_saved_due_to_date_set = 1;
                    this_month_old = GetMonth();
                }
                SetYear(clock.year);
                SetMonth(clock.month);
                SetDay(clock.day);
            }
            actualState = STATE_IDLE;
            firstDisplay = 1;
            actualSettingNext = 0;
            actualSetting = 0;
            delay_ms(2000);
            return;    
        }
        else
            actualSettingNext++;
    }
    
    if (menuTimer > (15*ONE_SECOND))
    {
        actualState = STATE_IDLE;
        firstDisplay = 1;
        return;
    }
    
    return;   
}

void f_Idle(void)
{
    
    /*lcd_clear();
    LCD_GoToLineColumn(0,0);
    itoa(total_durate_zi[3],buffer);
    lcd_puts(buffer);
    LCD_GoToLineColumn(1,0);
    itoa(total_porniri_zi[3],buffer);
    lcd_puts(buffer);
    delay_ms(2000);*/
    
    today = GetDay();
    
    if ((inputRelayStartStop == 0) && (day_saved == 0))
    {
        //today = GetDay();
        if (today != stop_day) // new day
        {
            PORTD.0 = 1;
            delay_ms(4000);
            PORTD.0 = 0;
            Save1DayData(0);
            indexes[0] = 0;
            day_saved = 1;
        }
    }
    
    if ((today == 1) && (month_saved == 0))
    {
        PORTD.0 = 1;
        delay_ms(4000);
        PORTD.0 = 0;
        if (to_be_saved_due_to_date_set == 1)
        {
            Save1MonthData(this_month_old);
            to_be_saved_due_to_date_set = 0;
        }
        else
        {
            Save1MonthData(GetMonth() - 1);
        }
        indexes[1]++;
        if (indexes[1] > 5)
            indexes[1] = 0;
        month_saved = 1;
        RESET_EEPROM_char_array(total_porniri_zi,32);
        RESET_EEPROM_int_array(total_durate_zi,31);        
    }
    
    if ((today != 1) && (month_saved != 0))
    {
        month_saved = 0;
    }
    
    
    /*if ((inputRelayStartStop == 0) && (month_saved == 0))
    {
        this_month = GetMonth();
        if (this_month != this_month_old) // new month
        {
            Save1MonthData();
            indexes[1]++;
            month_saved = 1;
        }
    }*/
    
    if (inputRelayStartStop > inputRelayStartStop_Old)
    {
        if (inputRelayStartStop_Old == 0) // it's a start
        {
            ProcessInputRelay_ON();
        }
        else    // it's a stop
        {
            ProcessInputRelay_OFF();
        }
    }
        
    if (displayRefresh>16)
    {
        lcd_clear();
        LCD_GoToLineColumn(0,0);
        GetTimeString24H(Time);
        lcd_puts(Time);
        LCD_GoToLineColumn(0,11);
        itoa(indexes[0],buffer);
        lcd_puts(buffer);
        LCD_GoToLineColumn(0,14);
        itoa(indexes[1],buffer);
        lcd_puts(buffer);
        LCD_GoToLineColumn(0,17);
        itoa(month_saved,buffer);
        lcd_puts(buffer);
        LCD_GoToLineColumn(1,0);
        GetDateString(Date);
        lcd_puts(Date);
        LCD_GoToLineColumn(1,9);
        lcd_puts(menuIcons);
        //LCD_GoToLineColumn(1,5);
        //lcd_puts("              ");
        displayRefresh = 0;
    }
    
    //delay_ms(500);
    if (BTN_OK == 1)
    {
        while (BTN_OK == 1) { PORTD.0 = 1;}
        PORTD.0 = 0;
        actualState = STATE_MENU;
        menuTimer = 0;
        
    }
    
    return;    
}

void f_Menu()
{
    if (displayRefresh>16)
    {
        lcd_clear();
        LCD_GoToLineColumn(0,0);
        switch(menuNumber)
        {
            case MENU_0_IDLE:
                actualState = STATE_IDLE;
                menuNumber = 1;
                return;
                break;
            case MENU_1_SET_HOUR:
                lcd_puts("1. SET HOUR");
                break;
            case MENU_2_SET_DATE:
                lcd_puts("2. SET DATE");
                break;
            case MENU_3_READ_TODAY:
                lcd_puts("3. READ LAST DAY");
                break;
            case MENU_4_READ_DAILY:
                lcd_puts("4. READ DAILY");
                break;
            case MENU_5_READ_MONTHLY:
                lcd_puts("5. READ MONTHLY");
                break;
            case MENU_6_RESET_EEPROM:
                lcd_puts("6. RESET EEPROM");
                break;
            default:
                break;            
        }
        //lcd_puts("1. SET HOUR");
        LCD_GoToLineColumn(1,0);
        lcd_puts(menuIcons);
        LCD_GoToLineColumn(1,5);
        lcd_puts("              ");
        displayRefresh = 0;
    }
    if (BTN_RIGHT == 1)
    {
        while (BTN_RIGHT == 1) {}
        menuNumber++;
        menuTimer = 0;
        if(menuNumber == 7)
        {
            menuNumber = 1;
            actualState = STATE_IDLE;
            return;
        }
    }
    
    if (BTN_LEFT == 1)
    {
        while (BTN_LEFT == 1) {}
        menuNumber--;
        menuTimer = 0;
        if(menuNumber == 0)
        {
            menuNumber = 1;
            actualState = STATE_IDLE;
            return;
        } 
    }
    
    if (BTN_OK == 1)
    {
        while (BTN_OK == 1) {}
        switch(menuNumber)
        {
            case MENU_1_SET_HOUR:
                actualState = STATE_SET_HOUR;
                break;
            case MENU_2_SET_DATE:
                actualState = STATE_SET_DATE;
                break;
            case MENU_3_READ_TODAY:
                actualState = STATE_READ_ONE_DAY;                
                break;
            case MENU_4_READ_DAILY:
                actualState = STATE_READ_STATISTICS_1_MONTH;
                break;
            case MENU_5_READ_MONTHLY:
                actualState = STATE_READ_STATISTICS_MONTHLY;
                break;
            case MENU_6_RESET_EEPROM:
                actualState = STATE_RESET_EEPROM;
                break;
            default:
                break;
        }
        for_i = 0;
        menuTimer = 0;
        return;
    }
    
    if (menuTimer > (5*ONE_SECOND))
    {
        actualState = STATE_IDLE;
        menuNumber = 1;
        return;
    }
    
    return;
}

//void f_SetHour(void)
//{}

//void f_SetDate()
//{}

void f_ReadOneDay()
{
    //char i = 0;
    char buffer[4];
    
    lcd_clear();
    LCD_GoToLineColumn(0,1);
    itoa(for_i+1,buffer);
    lcd_puts(buffer);
            
    // Afiseaza ora pornirii
    ShowTheTime(0,8, lista_porniri_1zi[3*for_i], lista_porniri_1zi[(3*for_i)+1], lista_porniri_1zi[(3*for_i)+2]);
            
    // Afiseaza durata
    LCD_GoToLineColumn(1,8);
    itoa(lista_durate_1zi[for_i],buffer);
    lcd_puts(buffer);
    lcd_puts(" min");
       
    while ((BTN_LEFT == 0) && (BTN_RIGHT == 0) && (BTN_OK == 0))
    {
        if (menuTimer > (11*ONE_SECOND))
        {
            actualState = STATE_IDLE;
            return;
        }
    }
       
    if (BTN_OK == 1)
    {
    while (BTN_OK == 1) {}
    actualState = STATE_IDLE;    
    }
       
    if (BTN_RIGHT == 1)
    {
    while (BTN_RIGHT == 1) {}
    menuTimer = 0;
    for_i++;    
    }
       
    if (BTN_LEFT == 1)
    {
    while (BTN_LEFT == 1) {}
    menuTimer = 0;
    for_i--;    
    }
    
    if (for_i > indexes[0]-1)
        for_i = 0;
    if (for_i < 0)
        for_i = indexes[0]-1;

   return;


}

void f_Read_Statistics_Daily()
{
    //char i = 0;
    char buffer[4];
    
    lcd_clear();
    LCD_GoToLineColumn(0,1);
    itoa(for_i+1,buffer);
    lcd_puts(buffer);
            
    // Afiseaza numar porniri
    LCD_GoToLineColumn(0,7);
    itoa(total_porniri_zi[for_i],buffer);
    lcd_puts(buffer);
            
    // Afiseaza durate
    LCD_GoToLineColumn(1,7);
    itoa(total_durate_zi[for_i],buffer);
    lcd_puts(buffer);
    lcd_puts(" min");
       
    while ((BTN_LEFT == 0) && (BTN_RIGHT == 0) && (BTN_OK == 0))
    {
        if (menuTimer > (11*ONE_SECOND))
        {
            actualState = STATE_IDLE;
            return;
        }
    }
       
    if (BTN_OK == 1)
    {
    while (BTN_OK == 1) {}
    actualState = STATE_IDLE;    
    }
       
    if (BTN_RIGHT == 1)
    {
    while (BTN_RIGHT == 1) {}
    menuTimer = 0;
    for_i++;    
    }
       
    if (BTN_LEFT == 1)
    {
    while (BTN_LEFT == 1) {}
    menuTimer = 0;
    for_i--;    
    }
    
    if (for_i > 30)
        for_i = 0;
    if (for_i < 0)
        for_i = 30;

   return;


}

void f_Read_Statistics_Monthly()
{
    //char i = 0;
    char buffer[4];
    
    lcd_clear();
    LCD_GoToLineColumn(0,1);
    itoa(for_i+1,buffer);
    lcd_puts(buffer);
    LCD_GoToLineColumn(0,3);
    itoa(numar_luna[for_i],buffer);
    lcd_puts(buffer);
            
    // Afiseaza numar porniri
    LCD_GoToLineColumn(0,7);
    itoa(total_porniri_luna[for_i],buffer);
    lcd_puts(buffer);
            
    // Afiseaza durate
    LCD_GoToLineColumn(1,7);
    itoa(total_durate_luna[for_i],buffer);
    lcd_puts(buffer);
    lcd_puts(" min");
       
    while ((BTN_LEFT == 0) && (BTN_RIGHT == 0) && (BTN_OK == 0))
    {
        if (menuTimer > (11*ONE_SECOND))
        {
            actualState = STATE_IDLE;
            return;
        }
    }
       
    if (BTN_OK == 1)
    {
    while (BTN_OK == 1) {}
    actualState = STATE_IDLE;    
    }
       
    if (BTN_RIGHT == 1)
    {
    while (BTN_RIGHT == 1) {}
    menuTimer = 0;
    for_i++;    
    }
       
    if (BTN_LEFT == 1)
    {
    while (BTN_LEFT == 1) {}
    menuTimer = 0;
    for_i--;    
    }
    
    if (for_i > 5)
        for_i = 0;
    if (for_i < 0)
        for_i = 5;

    return;
}

void f_Reset_All_Eeprom()
{
    RESET_EEPROM_char_array(lista_durate_1zi,86);
    RESET_EEPROM_char_array(lista_porniri_1zi,256);
    RESET_EEPROM_char_array(total_porniri_zi,32);
    RESET_EEPROM_char_array(numar_luna,7);
    RESET_EEPROM_char_array(indexes,3);
    RESET_EEPROM_int_array(total_durate_zi,31);
    RESET_EEPROM_int_array(total_durate_luna,6);
    RESET_EEPROM_int_array(total_porniri_luna,6);
    flag_indexes0_ovf = 0;
    start_day = today;
    stop_day = today;
    day_saved = 1;
    month_saved = 1;
    
    BLINK_LED1(2000,1000);
    actualState = STATE_IDLE;
    return;
}

void main(void)
{
    CPU_Init_1M_LCD_RTC_TIM0INT_EXT1_ENABLED();
    
    DDRD=0x00;
    DDRC=0x00;
    PORTD.2 = 1;   

    lcd_init(20);
    lcd_clear();
    BLINK_LED1(1000,0);
    
    //today = GetDay(); //month_saved = 1;
    //indexes[1] = 0;     month_saved = 0;
    
    //ClockInit(); 
    
    //SetHour(0);
    //SetMinute(33);
    //SetSecond(00);
    //SetAmPm(0);
    
    actualState = STATE_IDLE; 
    to_be_saved_due_to_date_set = 0;
    
    while (1)
    {   
        switch(actualState)
        {
            case STATE_IDLE:
                f_Idle();
                break;
            case STATE_MENU:
                f_Menu();
                break;
            case STATE_SET_HOUR:
                f_SetHour();
                break;
            case STATE_SET_DATE:
                f_SetDate();
                break;
            case STATE_READ_ONE_DAY:
                f_ReadOneDay();
                break;
            case STATE_READ_STATISTICS_1_MONTH:
                f_Read_Statistics_Daily();
                break;
            case STATE_READ_STATISTICS_MONTHLY:
                f_Read_Statistics_Monthly();
                break;
            case STATE_RESET_EEPROM:
                f_Reset_All_Eeprom();
                break;
            default:
                break;
        }
        
      
    }
}