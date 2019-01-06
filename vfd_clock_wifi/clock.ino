
/*
  Clock
*/


#include <Timezone.h>   // https://github.com/JChristensen/Timezone

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);


TimeElements          clock_tm;
boolean               isDst;
boolean               clock_cycling;
TIMER                 clock_cycle_timer;
byte                  clock_screen_mode;
byte                  clock_prev_sec;
CLOCK_REPORT_SEC_FN  *clock_report_sec_fn;
time_t                clock_adjust_secs;
boolean               clock_adjust_sub;

// Screen mode VERSION
TIMER                 clock_version_timer;

// Screen mode TIME
byte                  clock_time_prev_sec;
unsigned long         clock_time_sec_start_ms;
boolean               clock_time_sep_on;
byte                  clock_time_sep_mode;

// Screen mode DATE
unsigned long         clock_date_start_ms;

// Screen mode TEMP (temperature)
TIMER                 clock_temp_update_timer;

// Screen mode ADJUST
boolean               clock_adjusting;

// Keep track of uptime (millis() will overflow every 50 days)
unsigned long         clock_uptime_last_ms;
int                   clock_uptime_overflows;


#define CLOCK_DATE_UPDATE_MS      400
#define CLOCK_VERSION_MS         5000
#define CLOCK_CYCLE_TIME_MS      2500
#define CLOCK_CYCLE_INHIBIT_MS  30000


// Temperature is displayed when:
// (a) There's no time available.
// (b) From 20 to 24 seconds each minute if time is available.
//
// We choose an update period that is convenient for (a) and doesn't occur during
// (b). That's because 1-Wire temperature sampling generates a flickering effect
// now and then, and we don't want that during or just after (b).

#define CLOCK_TEMP_UPDATE_MS   5200


// Helper function

time_t  Clock_HMS_To_Secs (byte hour, byte min, byte sec)
{
  time_t secs = (hour * 60 + min) * 60 + sec;
  return secs;
}


void  Clock_Set_Report_Sec (CLOCK_REPORT_SEC_FN *report_sec_fn)
{
  clock_report_sec_fn = report_sec_fn;
}


boolean  Clock_Is_Time_Set ()
{
  return (timeStatus() != timeNotSet);
}


void  Clock_Read (CMD_PROC *p)
{
  char   line[80];
  int    cnt;

  if (timeStatus() != timeNotSet)
  {
    cnt = sprintf(line,
                  "%04u-%02u-%02u %02u:%02u:%02u",
                  clock_tm.Year + 1970,
                  clock_tm.Month,
                  clock_tm.Day,
                  clock_tm.Hour,
                  clock_tm.Minute,
                  clock_tm.Second);
  
    p->emit_str_fn(p,line,cnt);
  }
  else
  {
    p->emit_str_fn(p,"Time not set",0);
  }
}


void  Clock_Suspend ()
{
  Display_Suspend();
  RGB_Suspend();
}


void  Clock_Resume ()
{
  RGB_Resume();
  Display_Resume();
}


void  Clock_Write (time_t t)
{
  // Set the date and time in the time library
  setTime(t);

  clock_prev_sec = 255;
}


void  Clock_NTP_Report_Time (time_t t)
{
#ifdef  CLOCK_DEBUG
  Serial.printf("Clock_NTP_Report_Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                year(t),month(t),day(t),
                hour(t),minute(t),second(t));
#endif

  // Set the date and time with currently selected adjustment
  Clock_Set_Cur_Adjust(settings.clock_cur_adjust);
  Clock_Write(t);
}


void  Clock_Screen_Mode_Nothing_Enter ()
{
#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Nothing_Enter");
#endif

  Display_Write_Tube_Symbol(0,DISPLAY_SYMBOL_HYPHEN);
  Display_Write_Tube_Symbol(1,DISPLAY_SYMBOL_HYPHEN);
  Display_Write_Tube_Symbol(2,DISPLAY_SYMBOL_HYPHEN);
  Display_Write_Tube_Symbol(3,DISPLAY_SYMBOL_HYPHEN);
  Display_Write_Tube_Symbol(4,DISPLAY_SYMBOL_HYPHEN);
  Display_Write_Tube_Symbol(5,DISPLAY_SYMBOL_HYPHEN);

  Display_Clear_All_Dots();
}


void  Clock_Screen_Mode_Version_Enter ()
{
  const char *p = fw_version;
  int   digit;
  
#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Version_Enter");
#endif

  Display_Clear_All_Dots();

  Display_Write_Tube_Symbol(0,DISPLAY_SYMBOL_V);

  for( digit=0; (digit<6) && (*p); p++ ) {
    if( *p=='.' ) {
      Display_Write_Dot(digit,true);
      continue;
    }
    
    digit++;    
    if( isDigit(*p) )
      Display_Write_Tube_Digit(digit,*p-'0');
    else if( *p=='-' )
      Display_Write_Tube_Symbol(digit,DISPLAY_SYMBOL_HYPHEN);
    else 
      Display_Write_Tube_Symbol(digit,DISPLAY_SYMBOL_SPACE);
  }

  while( ++digit<6 )
    Display_Write_Tube_Symbol( digit, DISPLAY_SYMBOL_SPACE );

}


void  Clock_Screen_Mode_Time_Enter ()
{
#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Time_Enter");
#endif

  // Force recognition of new second
  clock_time_prev_sec = 255;

  Display_Clear_All_Dots();
}


void  Clock_Screen_Mode_Time_Exec ()
{
  if (clock_time_prev_sec != clock_tm.Second)
  {
    byte    u;
    byte    digit;

    clock_time_prev_sec = clock_tm.Second;
    clock_time_sec_start_ms = millis();

    // Hours digits:
    // * 12-hour or 24-hour represenation.
    // * Show or hide leading zero i.e. when the tens digit is zero.

    u = clock_tm.Hour;
    if (settings.clock_hour_format == CLOCK_HOUR_FORMAT_12)
    {
      if (u == 0) u = 12; else
      if (u > 12) u-= 12;
    }

    digit = u / 10;
    if ((digit == 0) && (!settings.clock_hour_lz))
      Display_Write_Tube_Symbol(0,DISPLAY_SYMBOL_SPACE);
    else
      Display_Write_Tube_Digit(0,digit);

    digit = u % 10;
    Display_Write_Tube_Digit(1,digit);

    // Minutes digits
    u = clock_tm.Minute;
    digit = u / 10;
    Display_Write_Tube_Digit(2,digit);
    digit = u % 10;
    Display_Write_Tube_Digit(3,digit);

    // Seconds digits
    u = clock_tm.Second;
    digit = u / 10;
    Display_Write_Tube_Digit(4,digit);
    digit = u % 10;
    Display_Write_Tube_Digit(5,digit);

    // Indicate DST
    Display_Write_Dot(5, isDst);
      
    // Work with a local copy of the mode value in case the setting is changed during the
    // remainder of the second
    clock_time_sep_mode = settings.clock_time_sep_mode;

    if (clock_time_sep_mode == CLOCK_TIME_SEP_MODE_BLINK)
    {
      Display_Write_Sep_Left(true);
      Display_Write_Sep_Right(true);
      clock_time_sep_on = true;
    }
    else
    if (clock_time_sep_mode == CLOCK_TIME_SEP_MODE_AMPM)
    {
      if (clock_tm.Hour < 12)
      {
        // Ante meridiem
        Display_Write_Sep_Left(true);
        Display_Write_Sep_Right(false);
      }
      else
      {
        // Post meridiem
        Display_Write_Sep_Left(false);
        Display_Write_Sep_Right(true);
      }
    }
    else
    if (clock_time_sep_mode == CLOCK_TIME_SEP_MODE_ON)
    {
      Display_Write_Sep_Left(true);
      Display_Write_Sep_Right(true);
    }
    else
    if (clock_time_sep_mode == CLOCK_TIME_SEP_MODE_OFF)
    {
      Display_Write_Sep_Left(false);
      Display_Write_Sep_Right(false);
    }
  }
  else
  {
    if (clock_time_sep_mode == CLOCK_TIME_SEP_MODE_BLINK)
    {
      if (clock_time_sep_on)
      {
        if ((millis() - clock_time_sec_start_ms) > 500)
        {
          Display_Write_Sep_Left(false);
          Display_Write_Sep_Right(false);
          clock_time_sep_on = false;
        }
      }
    }
  }
}


void  Clock_Screen_Mode_Date_Update ()
{
  static  byte  tube_index[CLOCK_DATE_FORMAT_CNT][3] =
  {
    0,2,4,  // D-M-Y
    2,0,4,  // M-D-Y
    4,2,0   // Y-M-D
  };

  byte      u;
  byte      digit;
  uint16_t  yr;

  u = clock_tm.Day;
  digit = u / 10;
  Display_Write_Tube_Digit(tube_index[settings.clock_date_format][0],digit);
  digit = u % 10;
  Display_Write_Tube_Digit(tube_index[settings.clock_date_format][0]+1,digit);

  u = clock_tm.Month;
  digit = u / 10;
  Display_Write_Tube_Digit(tube_index[settings.clock_date_format][1],digit);
  digit = u % 10;
  Display_Write_Tube_Digit(tube_index[settings.clock_date_format][1]+1,digit);

  yr = (clock_tm.Year + 1970) % 100;
  u = (byte)yr;
  digit = u / 10;
  Display_Write_Tube_Digit(tube_index[settings.clock_date_format][2],digit);
  digit = u % 10;
  Display_Write_Tube_Digit(tube_index[settings.clock_date_format][2]+1,digit);

  if (settings.clock_date_sep_mode == CLOCK_DATE_SEP_MODE_ON)
  {
    Display_Write_Sep_Left(true);
    Display_Write_Sep_Right(true);
  }
  else
  if (settings.clock_date_sep_mode == CLOCK_DATE_SEP_MODE_OFF)
  {
    Display_Write_Sep_Left(false);
    Display_Write_Sep_Right(false);
  }
}


void  Clock_Screen_Mode_Date_Exec ()
{
  if ((millis() - clock_date_start_ms) > CLOCK_DATE_UPDATE_MS)
  {
    // Restart update timer
    clock_date_start_ms += CLOCK_DATE_UPDATE_MS;

    Clock_Screen_Mode_Date_Update();
  }
}


void  Clock_Screen_Mode_Date_Enter ()
{
#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Date_Enter");
#endif

  Display_Write_Dot(0,false);
  Display_Write_Dot(1,true);
  Display_Write_Dot(2,false);
  Display_Write_Dot(3,true);
  Display_Write_Dot(4,false);
  Display_Write_Dot(5,false);

  Clock_Screen_Mode_Date_Update();

  // Start update timer
  clock_date_start_ms = millis();
}


void  Clock_Screen_Mode_Temp_Update ()
{
  TS_VALUES *ts_values;
  byte       symbol;
  float      temp;
  int32_t    raw_temp;
  int32_t    abs_temp;
  boolean    sign;
  byte       digit;
  int        tube_index;

#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Temp_Update");
#endif

  ts_values = TS_Get_Values();

  if (settings.clock_temp_format == CLOCK_TEMP_FORMAT_CELSIUS)
  {
    temp   = ts_values->temp;
    symbol = DISPLAY_SYMBOL_C;
  }
  else
  {
    temp = (ts_values->temp * 9 + 160) / 5;
    symbol = DISPLAY_SYMBOL_F;
  }

  // Shift one digit to left to get first decimal and round
  raw_temp = (int32_t)(temp * 10 + 0.5);

  // Tests
  //raw_temp = -1000;  // -1000.0 C -> displays "000.0"
  //raw_temp = -100;   //  -100.0 C -> displays "100.0"
  //raw_temp = -10;    //   -10.0 C -> displays "-10.0"
  //raw_temp = -1;     //    -1.0 C -> displays " -1.0"
  //raw_temp = -0.1;   //    -0.1 C -> displays " -0.1"
  //raw_temp = 0;      //     0.0 C -> displays "  0.0"
  //raw_temp = 0.1;    //     0.1 C -> displays "  0.1"
  //raw_temp = 1;      //     1.0 C -> displays "  1.0"
  //raw_temp = 10;     //    10.0 C -> displays " 10.0"
  //raw_temp = 100;    //   100.0 C -> displays "100.0"
  //raw_temp = 1000;   //  1000.0 C -> displays "000.0"

  if (raw_temp >= 0)
  {
    abs_temp = raw_temp;
    sign     = false;
  }
  else
  {
    abs_temp = -raw_temp;
    sign     = true;
  }

#ifdef  CLOCK_DEBUG
  //Serial.printf("temp=%04Xh sign=%d abs_temp=%d\n",raw_temp,sign,abs_temp);
#endif

  // Symbol of temprature scale (C for Celsius, F for Fahrenheit)
  Display_Write_Tube_Symbol(4, DISPLAY_SYMBOL_DEGREES);
  Display_Write_Tube_Symbol(5, symbol);

  // Tenths digit
  digit = abs_temp % 10;
  abs_temp /= 10;
  Display_Write_Tube_Digit(3, digit);

  Display_Write_Dot(2, true);

  // Ones digit
  digit = abs_temp % 10;
  abs_temp /= 10;
  Display_Write_Tube_Digit(2, digit);

  // Tens digit, hundreds digit
  tube_index = 1;
  for (;;)
  {
    if (abs_temp == 0) break;

    digit = abs_temp % 10;
    abs_temp /= 10;
    Display_Write_Tube_Digit(tube_index, digit);

    if (tube_index == 0) return;
    tube_index--;
  }

  // Tube index is 1 or 0: write sign indication (hyphen for minus, space for plus)
  symbol = (sign == true) ? DISPLAY_SYMBOL_HYPHEN : DISPLAY_SYMBOL_SPACE;
  Display_Write_Tube_Symbol(tube_index, symbol);

  // Write a space symbol if tube 0 wasn't written yet
  if (tube_index > 0) Display_Write_Tube_Symbol(0, DISPLAY_SYMBOL_SPACE);
}


void  Clock_Screen_Mode_Pres_Update ()
{
  TS_VALUES *ts_values;
  byte       symbol;
  int32_t    value;
  int        tube_index;

#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Pres_Update");
#endif

  ts_values = TS_Get_Values();

  // Value is in Pa, so devide by 10 and round to get hPa with one decimal
  value = (int32_t)(ts_values->pressure/10 + 0.5);

  Display_Write_Dot( 3, true );


  // Write digits right to left
  for ( tube_index=4; value && tube_index>=0; tube_index-- ) {
    Display_Write_Tube_Digit( tube_index, value%10 );
    value /= 10;
  }

  // Fill leading spaces
  while ( tube_index >= 0 )
    Display_Write_Tube_Symbol( tube_index--, DISPLAY_SYMBOL_SPACE );

  // Show trend
  switch( TS_Get_Trend() ) {
    case TS_TREND_STABLE:
      Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_HYPHEN );
      break;

    case TS_TREND_UP:
      Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_UP );
      break;

    case TS_TREND_DOWN:
      Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_DOWN );
      break;
  
    case TS_TREND_FASTUP:
      Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_FASTUP );
      break;

    case TS_TREND_FASTDOWN:
      Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_FASTDOWN );
      break;
      
    case TS_TREND_NA:
    default:
      Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_SPACE );
      break;
  }
    
}


void  Clock_Screen_Mode_Hygr_Update ()
{
  TS_VALUES *ts_values;
  int32_t    value;
  int        tube_index;

#ifdef  CLOCK_DEBUG
  //Serial.println("Clock_Screen_Mode_Hygr_Update");
#endif

  ts_values = TS_Get_Values();

  // Multiply value by 10 and round to get % with one decimal
  value = (int32_t)(ts_values->humidity*10 + 0.5);

  // Simulate %
  Display_Write_Tube_Symbol( 4, DISPLAY_SYMBOL_DEGREES );
  Display_Write_Tube_Symbol( 5, DISPLAY_SYMBOL_O );
  
  Display_Write_Dot( 2, true );

  // Write digits right to left
  for ( tube_index=3; value && tube_index>=0; tube_index-- ) {
    Display_Write_Tube_Digit( tube_index, value%10 );
    value /= 10;
  }

  // Fill leading spaces
  while ( tube_index >= 0 )
    Display_Write_Tube_Symbol( tube_index--, DISPLAY_SYMBOL_SPACE );
}


void  Clock_Screen_Mode_Temp_Exec ()
{
  if( !Timer_IsRunning(clock_temp_update_timer) )
  {
    // Restart update timer
    Timer_Start( clock_temp_update_timer, CLOCK_TEMP_UPDATE_MS );

    switch ( clock_screen_mode ) {

      case CLOCK_SCREEN_MODE_TEMP:
        Clock_Screen_Mode_Temp_Update();
        break;

      case CLOCK_SCREEN_MODE_PRES:
        Clock_Screen_Mode_Pres_Update();
        break;

      case CLOCK_SCREEN_MODE_HYGR:
        Clock_Screen_Mode_Hygr_Update();
        break;
    }

    TS_Request_Readout();
  }
}


void  Clock_Screen_Mode_Temp_Enter ()
{
  Display_Write_Tube_Symbol(4,DISPLAY_SYMBOL_DEGREES);

  Display_Clear_All_Dots();

  Display_Write_Sep_Left(false);
  Display_Write_Sep_Right(false);

  // Force update
  Timer_Clear( clock_temp_update_timer );
  Clock_Screen_Mode_Temp_Exec();
}


void  Clock_Screen_Mode_Adjusting_Exec ()
{
  Display_Write_Tube_Symbol(5,DISPLAY_SYMBOL_1 + settings.clock_cur_adjust);
}


void  Clock_Screen_Mode_Adjusting_Enter ()
{
  Display_Write_Tube_Symbol(0,DISPLAY_SYMBOL_SPACE);
  Display_Write_Tube_Symbol(1,DISPLAY_SYMBOL_SPACE);
  Display_Write_Tube_Symbol(2,DISPLAY_SYMBOL_SPACE);
  Display_Write_Tube_Symbol(3,DISPLAY_SYMBOL_SPACE);
  Display_Write_Tube_Symbol(4,DISPLAY_SYMBOL_SPACE);

  Display_Clear_All_Dots();
}


void  Clock_Screen_Mode_Enter (byte mode)
{
  clock_screen_mode = mode;

  switch (clock_screen_mode)
  {
    case CLOCK_SCREEN_MODE_NOTHING:
    {
      Clock_Screen_Mode_Nothing_Enter();
      break;
    }

    case CLOCK_SCREEN_MODE_VERSION:
    {
      Clock_Screen_Mode_Version_Enter();
      break;
    }

    case CLOCK_SCREEN_MODE_TIME:
    {
      Clock_Screen_Mode_Time_Enter();
      break;
    }

    case CLOCK_SCREEN_MODE_DATE:
    {
      Clock_Screen_Mode_Date_Enter();
      break;
    }

    case CLOCK_SCREEN_MODE_TEMP:
    case CLOCK_SCREEN_MODE_PRES:
    case CLOCK_SCREEN_MODE_HYGR:
    {
      Clock_Screen_Mode_Temp_Enter();
      break;
    }

    case CLOCK_SCREEN_MODE_ADJUSTING:
    {
      Clock_Screen_Mode_Adjusting_Enter();
      break;
    }
  }
}


void  Clock_Screen_Mode_Exec ()
{
  switch (clock_screen_mode)
  {
    case CLOCK_SCREEN_MODE_TIME:
    {
      Clock_Screen_Mode_Time_Exec();
      break;
    }

    case CLOCK_SCREEN_MODE_DATE:
    {
      Clock_Screen_Mode_Date_Exec();
      break;
    }

    case CLOCK_SCREEN_MODE_TEMP:
    case CLOCK_SCREEN_MODE_PRES:
    case CLOCK_SCREEN_MODE_HYGR:
    {
      Clock_Screen_Mode_Temp_Exec();
      break;
    }

    case CLOCK_SCREEN_MODE_ADJUSTING:
    {
      Clock_Screen_Mode_Adjusting_Exec();
      break;
    }
  }
}


void  Clock_Screen_Mode_Exit ()
{
}


void  Clock_Set_Adjust (byte adjust_hour, byte adjust_min, byte adjust_sec, boolean adjust_sub)
{
#ifdef  CLOCK_DEBUG
  Serial.printf("Clock_Set_Adjust: %c%02d:%02d:%02d\n",
                adjust_sub ? '-' : '+',
                adjust_hour,
                adjust_min,
                adjust_sec);
#endif

  clock_adjust_secs = Clock_HMS_To_Secs(adjust_hour,adjust_min,adjust_sec);
  clock_adjust_sub  = adjust_sub;
}


void  Clock_Set_Cur_Adjust (byte cur_adjust)
{
  byte      adjust_hour;
  byte      adjust_min;
  byte      adjust_sec;
  boolean   adjust_sub;

  // Always set adjustment, even if the currently selected adjustment doesn't change. We
  // want to update any changed adjustment values in the settings here.

  settings.clock_cur_adjust = cur_adjust;

  switch (cur_adjust)
  {
    case 0:
    {
      adjust_hour = settings.clock_adjust_hour;
      adjust_min  = settings.clock_adjust_min;
      adjust_sec  = settings.clock_adjust_sec;
      adjust_sub  = settings.clock_adjust_sub;
      break;
    }

    case 1:
    {
      adjust_hour = settings.clock_adjust_hour_2;
      adjust_min  = settings.clock_adjust_min_2;
      adjust_sec  = settings.clock_adjust_sec_2;
      adjust_sub  = settings.clock_adjust_sub_2;
      break;
    }

    case 2:
    {
      adjust_hour = settings.clock_adjust_hour_3;
      adjust_min  = settings.clock_adjust_min_3;
      adjust_sec  = settings.clock_adjust_sec_3;
      adjust_sub  = settings.clock_adjust_sub_3;
      break;
    }

    case 3:
    {
      adjust_hour = settings.clock_adjust_hour_4;
      adjust_min  = settings.clock_adjust_min_4;
      adjust_sec  = settings.clock_adjust_sec_4;
      adjust_sub  = settings.clock_adjust_sub_4;
      break;
    }

    default:  return; // Shouldn't end up here
  }

#ifdef  CLOCK_DEBUG
  Serial.printf("Clock_Set_Cur_Adjust: #%u: %c%02d:%02d:%02d\n",
                cur_adjust + 1,
                adjust_sub ? '-' : '+',
                adjust_hour,
                adjust_min,
                adjust_sec);
#endif

  Clock_Set_Adjust(adjust_hour,adjust_min,adjust_sec,adjust_sub);
}


void  Clock_Apply_Cur_Adjust ()
{
  Clock_Set_Cur_Adjust(settings.clock_cur_adjust);
}

void  Clock_Apply_Next_Adjust ()
{
  byte  cur_adjust = (settings.clock_cur_adjust + 1) % SETTINGS_CLOCK_ADJUST_CNT;

  Clock_Set_Cur_Adjust(cur_adjust);
}


void  Clock_Set_Adjusting (boolean adjusting)
{
  clock_adjusting = adjusting;
}


void Clock_Start_Cycling()
{
  byte screen_mode = CLOCK_SCREEN_MODE_NOTHING;

  // content of first cycle
  if( timeStatus()!=timeNotSet )
    screen_mode = CLOCK_SCREEN_MODE_DATE;
  else if( TS_Is_Present() )
    screen_mode = CLOCK_SCREEN_MODE_TEMP;

#ifdef  CLOCK_DEBUG
  Serial.printf( "Clock_Start_Cycling: cycl(old)=%c screen=%d\n", clock_cycling?'t':'f', screen_mode );
#endif

  // Already cycling or no data ?
  if( clock_adjusting || clock_cycling || screen_mode==CLOCK_SCREEN_MODE_NOTHING )
    return;

  clock_cycling = true;
  Timer_Start( clock_cycle_timer, CLOCK_CYCLE_TIME_MS );

  // Handle clock screen mode change and execution

  if (clock_screen_mode != screen_mode)
  {
    Clock_Screen_Mode_Exit();
    Clock_Screen_Mode_Enter(screen_mode);
  }

  Clock_Screen_Mode_Exec();
}


void  Clock_Exec ()
{
  unsigned long ms;
  time_t        t;
  byte          cur_sec;
  byte          cur_min;
  boolean       time_set;
  boolean       ts_present;
  byte          screen_mode;


  // Track uptime
  ms = millis();
  if( ms<clock_uptime_last_ms ) {
    clock_uptime_overflows++;
  }
  clock_uptime_last_ms = ms;


  // Call now() to keep the time library's timekeeping up-to-date.
  //
  // Note: the time library's date and time are always valid and being incremented. If date
  // and time aren't set, the library simply starts counting from 1st of January, 1970.
  t = now();
  
  if (clock_adjust_sub) t -= clock_adjust_secs; else t += clock_adjust_secs;

  // Convert to local time using timezone info, check for DST.
  isDst = false;
  if ( settings.clock_use_dst ) {
    isDst = CE.utcIsDST( t );
    t     = CE.toLocal( t );
  }

  breakTime(t,clock_tm);
  cur_sec = clock_tm.Second;
  cur_min = clock_tm.Minute;


  // Determine the screen mode.
  //
  // A change may be triggered by several events, like a specific second, or a request from
  // another part of the program.
  
  screen_mode = CLOCK_SCREEN_MODE_NOTHING;

  time_set = (timeStatus() != timeNotSet);
  
  if (time_set)
  {
    if ((cur_sec >= 50) && (cur_sec <= 54) && (settings.clock_show_date) && (!Timer_IsRunning(clock_cycle_timer)) && (!clock_cycling) )
    {
      screen_mode = CLOCK_SCREEN_MODE_DATE;
    }
    else
    {
      screen_mode = CLOCK_SCREEN_MODE_TIME;
    }
  }

  if( Timer_IsRunning(clock_version_timer) )
    screen_mode=CLOCK_SCREEN_MODE_VERSION;
  
  // Cycling auxiliary values?
  if( clock_cycling ) {
    if( !Timer_IsRunning(clock_cycle_timer) ) {
      TS_VALUES *ts_values = TS_Get_Values();
      Timer_Restart( clock_cycle_timer, CLOCK_CYCLE_TIME_MS );

#ifdef  CLOCK_DEBUG
      Serial.printf( "Clock_Exec: cycling: cycl=%c screen(old)=%d\n", clock_cycling?'t':'f', clock_screen_mode );
#endif

      // Cycle through all available data in this sequence: Date->Temperture->Humidity->Pressure
      // Ignore settings.clock_show_temp and settings.clock_show_date to use this to show data anloy on request
      
      switch( clock_screen_mode ) {
        case CLOCK_SCREEN_MODE_DATE:
          if( TS_Is_Present() && !isnan(ts_values->temp) )
            screen_mode = CLOCK_SCREEN_MODE_TEMP;
          else if( TS_Is_Present() && !isnan(ts_values->humidity) )
            screen_mode = CLOCK_SCREEN_MODE_HYGR;
          else if( TS_Is_Present() && !isnan(ts_values->pressure) )
            screen_mode = CLOCK_SCREEN_MODE_PRES;
          break;

        case CLOCK_SCREEN_MODE_TEMP:
          if( TS_Is_Present() && !isnan(ts_values->humidity) )
            screen_mode = CLOCK_SCREEN_MODE_HYGR;
          else if( TS_Is_Present() && !isnan(ts_values->pressure) )
            screen_mode = CLOCK_SCREEN_MODE_PRES;
          break;
        
        case CLOCK_SCREEN_MODE_HYGR:
          if( TS_Is_Present() && !isnan(ts_values->pressure) )
            screen_mode = CLOCK_SCREEN_MODE_PRES;
          break;
      }

      // End of cycling, inhibit automatic data display for some time
      if( screen_mode==CLOCK_SCREEN_MODE_NOTHING || screen_mode==CLOCK_SCREEN_MODE_VERSION || screen_mode==CLOCK_SCREEN_MODE_TIME ) {
        clock_cycling = false;
        Timer_Restart( clock_cycle_timer, CLOCK_CYCLE_INHIBIT_MS );
      }
    }

    // Wait till end of cycle
    else
      screen_mode = clock_screen_mode;

#ifdef  CLOCK_DEBUG
      Serial.printf( "Clock_Exec: next cycle: cycl=%c screen(new)=%d\n", clock_cycling?'t':'f', screen_mode );
#endif
  }

  else {

    // Every second...
    // * Invoke report callback function.
    // * Request reading of temperature once a minute, seconds before the temperature is to
    //   be displayed.
  
    if (clock_prev_sec != cur_sec)
    {
      clock_prev_sec = cur_sec;
  
      if (clock_report_sec_fn) clock_report_sec_fn(&clock_tm,Clock_Is_Time_Set());
  
      if (cur_sec == 15) TS_Request_Readout();
    }
  
    if ( TS_Is_Present() && settings.clock_show_temp && (!Timer_IsRunning(clock_version_timer) && ((!time_set) || ((cur_sec>=20) && (cur_sec<=24) && (!Timer_IsRunning(clock_cycle_timer))))) )
    {
      TS_VALUES *ts_values = TS_Get_Values();
      
      screen_mode = CLOCK_SCREEN_MODE_TEMP;
      if(  (cur_min%3==1)&&(!isnan(ts_values->pressure)) )
        screen_mode = CLOCK_SCREEN_MODE_PRES;
      if( (cur_min%3==2)&&(!isnan(ts_values->humidity)) )
        screen_mode = CLOCK_SCREEN_MODE_HYGR;
    }
    
    if (clock_adjusting)
    {
      screen_mode = CLOCK_SCREEN_MODE_ADJUSTING;
    }
  }

  // Handle clock screen mode change and execution

  if (clock_screen_mode != screen_mode)
  {
    Clock_Screen_Mode_Exit();
    Clock_Screen_Mode_Enter(screen_mode);
  }

  Clock_Screen_Mode_Exec();
}


void  Clock_Init ()
{
  Clock_Screen_Mode_Enter(CLOCK_SCREEN_MODE_VERSION);
  Timer_Start(clock_version_timer,CLOCK_VERSION_MS);
}


double Clock_Get_Uptime()
{
  unsigned long ms = millis();

  if( ms<clock_uptime_last_ms )
    clock_uptime_overflows++;
  
  clock_uptime_last_ms = ms;

  return ms/1000.0 + (0xFFFFFFFF/1000.0)*clock_uptime_overflows; 
}
