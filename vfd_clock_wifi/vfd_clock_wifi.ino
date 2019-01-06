
/*
  VFD Clock WiFi

  This is the head .ino file. When you compile the source, the Arduino IDE combines this
  source file and all other .ino files into one big .cpp file in which this .ino file
  comes first.

  The sketch retrieves the current date and time using NTP. The program uses port 5011 for
  receiving the NTP server's response.

  The sketch uses the cluster of time servers "pool.ntp.org" by default.

  The sketch can read temperature from a 1-Wire sensor. Supported 1-Wire slaves are
  DS18B20, DS18S20, DS1822. Pin 11 is used as 1-Wire bus. Don't forget the pull-up
  resistor between DQ and VCC.

    Wiring:

      1-Wire slave    ESP32-DevKitC

          GND  <------->  GND

          VCC  <---+--->  3.3V
                  4k7
          DQ  <----+--->  GPIO18

  The sketch can read temperature from a I2C LM75 or SE95D sensor. Don't forget to provide
  pull-up resistors for SDA (GPIO21) and SCL (GPIO22). 3k3 or 4k7 resistors work well. The
  I2C bus operates at 3.3V.

  This sketch listens to incoming commands on the serial port (115200 baud). A command
  is processed when a CR (13) or LF (10) character arrives. The commands are processed in
  the cmd_proc module. See the module's source for an overview of the available commands.

  The sketch includes a command server that listens to incoming connections on port 5010.
  Once connected, the client can send commands and receive responses identical to the
  serial port.

  Required libraries: (see menu: Sketch -> Include Library -> Manage Libraries...)

  - Time by Michael Margolis
    Version 1.5.0
    http://www.pjrc.com/teensy/td_libs_Time.html

 - Timezone by Jack Christensen
    Version 1.2.2  (Warning about architecture mismatch can be ignored)
    https://github.com/JChristensen/Timezone


  - BME280 by Tyler Glenn
    Version 2.3.0
    https://github.com/finitespace/BME280

  Required packages:

  - Arduino core for ESP32 WiFi chip
    Downloaded on 2017-10-06 from https://github.com/espressif/arduino-esp32
    Installed on 2017-10-07 for Windows 7 per instructions on this website

  Tested with:
    Arduino 1.8.7

  Author:  Peter S'heeren, Axiris
  Date:    2018-10-05
  License: public domain

  Revision history:

    2018-02-08  Peter S'heeren, Axiris

      * Initial release.

    2018-05-27  Peter S'heeren, Axiris

      * Added three additional time adjustments to the settings.
      * Hold button pressed to select next time adjustment.
      * Replaced macro DNS_DEBUG with DNS_LOOKUP_DEBUG.

    2018-10-05  Peter S'heeren, Axiris

      * Rearrange the source to make Arduino IDE 1.8.7 happy.
      * Added WIFI hostname to the settings. The hostname is usually retrievable when
        DHCP is used.
      * Fixed displaying of separator tubes when showing date.

    2018-12-30  //MAF
    
      Some experimental extensions:
      * Support for DST (EU time only, but easy to modify/extend, see top of clock.ino)
      * Support for BME280, BMP280 sensors (new file bme280.ino)
      * Display humidity and pressure (incl. 3h trend inidcator) if available
      * Cycle through auxiliary data (date, temperature, humidity, pressure) when button is pushed and display is on
      * Debouncing of button
      * New commands to control above functions and read data (see comments in cmd_proc.ino)
      * timer.ino to avoid complications of 32bit overflow of millis() (only used in new functions)
      * New commands "help" and "system status"
      * Show firmware revision at boot time
*/


#include "WiFi.h"
#include "EEPROM.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <TimeLib.h>
#include <Wire.h>


// Firmware version (allowed characters:m digits, hyphen and ., max. 5 chars excl. dots)
const char fw_version[]   = "1.03.12";
const char compile_date[] = __DATE__ " " __TIME__;


// Uncomment a line if you want the code to print debug-information to the serial interface

//#define APP_DEBUG
//#define CLOCK_DEBUG
//#define CMD_PROC_DEBUG
//#define CMD_SERVER_DEBUG
//#define DISPLAY_DEBUG
//#define DNS_LOOKUP_DEBUG
//#define LM75_DEBUG
//#define MAIN_DEBUG
//#define NTP_DEBUG
//#define OW_DEBUG
//#define OWTEMP_DEBUG
//#define RGB_DEBUG
//#define SERIAL_DEBUG
//#define SETTINGS_DEBUG
//#define TOKENIZER_DEBUG
//#define TS_DEBUG
//#define WIFI_DEBUG


#define GetContAd(var_ad,cont_type_name,var_name)                 \
  (cont_type_name*)                                               \
  ((uint8_t*)(var_ad) - (uint8_t*)&((cont_type_name*)0)->var_name)


// Assignment of I/O pins

// Display
#define PIN_DATA      17    // DATA
#define PIN_CLK       16    // CLOCK
#define PIN_LE        32    // LATCH ENABLE
#define PIN_STANDBY   33    // STANDBY

// RGB
#define PIN_RED       25    // Red channel
#define PIN_GREEN     26    // Green channel
#define PIN_BLUE      27    // Blue channel

// I2C bus
#define PIN_SDA       21    // Serial Data
#define PIN_SDL       22    // Serial Clock

// 1-Wire bus
#define PIN_DQ        18    // Data line

// Main task
#define PIN_RAISE     35    // Button


// Maximum length of a WIFI hostname (number of characters excluding terminating zero)
#define WIFI_HOSTNAME_MAX_LEN      63

// Maximum length of an NTP hostname (number of characters excluding terminating zero)
#define NTP_HOSTNAME_MAX_LEN      253


/*
  Tokenizer
*/


// Token identifiers

typedef enum  _TOK_ID
{
  TOK_ID_EOL,
  TOK_ID_NUMBER,
  TOK_ID_STRING,
  TOK_ID_LABEL,
  TOK_ID_CHAR
}
  TOK_ID;


// Tokenizer context

typedef struct  _TOK
{
  // Input string
  char       *s;              // ASCIIZ string
  uint32_t    fi;             // Fetch index in s

  // Output token
  TOK_ID      id;             // Token identifier (TOK_ID_Xxx)
  char        buf[256];       // Tokenized element
  uint32_t    buf_si;         // Store index in buf
  uint32_t    val;            // Value; valid if id is TOK_ID_NUMBER
  uint32_t    start_index;    // Start index of token
}
  TOK;


void  Tok_Init (TOK *t, char *s);

boolean  Tok_Fetch (TOK *t);

boolean  Tok_Fetch_EOL (TOK *t);


/*
  Command processor
*/


typedef struct  _CMD_PROC   CMD_PROC;


// Emit zero-terminated string callback function.
//
// If cnt is zero, the string length is unknown meaning the callee should use strlen() if
// needed.

typedef void  CMD_PROC_EMIT_STR_FN (CMD_PROC *p, char *s, uint32_t cnt);


// Close the interface, if possible.

typedef void  CMD_PROC_CLOSE_FN (CMD_PROC *p);


#define CMD_PROC_RCV_BUF_LEN   128

struct  _CMD_PROC
{
  char      rcv_buf[CMD_PROC_RCV_BUF_LEN + 1];  // Receive buffer
  byte      rcv_si;                             // Store index
  boolean   rcv_error;                          // Receive error condition
  byte      rcv_fi;                             // Fetch index
  char      rcv_fetch;                          // Last fetched character

  boolean   echo_commands;

  CMD_PROC_EMIT_STR_FN   *emit_str_fn;
  CMD_PROC_CLOSE_FN      *close_fn;
};


void  Cmd_Proc_Feed (CMD_PROC *p, char *s, uint32_t cnt);

void  Cmd_Proc_Reset (CMD_PROC *p);


/*
  WIFI
*/


// Maximum length of a WIFI SSID (number of characters excluding terminating zero)
#define WIFI_SSID_MAX_LEN       32

// Maximum length of a WIFI password (number of characters excluding terminating zero)
#define WIFI_PASSWORD_MAX_LEN   63


void  WIFI_Init ();

void  WIFI_Exec ();

void  WIFI_Connect ();

void  WIFI_Disconnect ();

boolean  WIFI_Is_Disconnected ();

boolean  WIFI_Is_Connecting ();

boolean  WIFI_Is_Connected ();

boolean  WIFI_Is_Disconnecting ();

void  WIFI_Dump (CMD_PROC *p);


/*
  Serial interface (USB-based)
*/


void  Serial_Init ();

void  Serial_Exec ();

CMD_PROC  *Serial_Get_Cmd_Proc ();


/*
  Command server
*/


typedef struct  _CMD_SERVER_CLIENT
{
  WiFiClient    client;
  CMD_PROC      cmd_proc;
}
  CMD_SERVER_CLIENT;


void  Cmd_Server_Init ();

void  Cmd_Server_Exec ();

void  Cmd_Server_Start ();

void  Cmd_Server_Stop ();

boolean  Cmd_Server_Has_Started ();

void  Cmd_Server_Client_Close (uint32_t client_index);

void  Cmd_Server_Client_Close_All ();

void  Cmd_Server_Dump (CMD_PROC *p);


/*
  DNS lookup host by name
*/


typedef struct  _DNS_HOST_BY_NAME_IO
{
  char       *hostname;
  ip_addr_t   addr;
  IPAddress   res_ad;
  boolean     completed;
}
  DNS_HOST_BY_NAME_IO;


void  DNS_Host_By_Name_Init (DNS_HOST_BY_NAME_IO *io);

boolean  DNS_Host_By_Name_Has_Completed (DNS_HOST_BY_NAME_IO *io);


/*
  NTP query
*/


typedef void  NTP_REPORT_TIME_FN (time_t t);


void  NTP_Init ();

void  NTP_Exec ();

void  NTP_Start (NTP_REPORT_TIME_FN *report_time_fn);

boolean  NTP_Is_Busy ();


/*
  1-Wire master
*/


typedef void  OW_LOCK_FN ();

typedef void  OW_UNLOCK_FN ();


typedef struct  _OW_ENUM
{
  byte      rom_code[8];
  byte      prev_rom_code[8];
  boolean   rom_code_found;
  boolean   crc_valid;
  byte      exec_state;
  byte      last_diff_nbr;
  byte      xfr_byte_index;
  byte      xfr_bit_index;
  byte      xfr_byte;
  byte      prev_diff_nbr;
  byte      bit_nbr;
  byte      res_bit_1;
  byte      res_bit_2;
}
  OW_ENUM;


typedef struct  _OW_TOUCH_BITS
{
  byte       *buf;
  uint16_t    bits_left;
  byte        byte_index;
  byte        bit_index;
  byte        exec_state;
  boolean     buf_ro;       // Buffer is read-only y/n
}
  OW_TOUCH_BITS;


void  OW_Init (byte pin_dq, OW_LOCK_FN *lock_fn, OW_UNLOCK_FN *unlock_fn);

void  OW_Exec ();

boolean  OW_Bus_Reset ();

void  OW_Enum_First_Start ();

void  OW_Enum_Next_Start ();

boolean  OW_Enum_Is_Busy ();

boolean  OW_Enum_Is_CRC_Valid ();

byte  *OW_Enum_Get_ROM_Code ();

void  OW_Touch_Bits_Start (byte *buf, uint16_t bits, boolean buf_ro);

boolean  OW_Touch_Bits_Is_Busy ();


/*
  1-Wire temperature sensor
*/


void  OWTemp_Init ();

void  OWTemp_Exec ();

void  OWTemp_Request_Read_Temp ();

boolean  OWTemp_Is_Present ();

int16_t  OWTemp_Get_Temp ();

byte  OWTemp_Get_Frac_Cnt ();


/*
  LM75/SE95D I2C temperature sensor
*/


void  LM75_Init ();

void  LM75_Exec ();

void  LM75_Request_Read_Temp ();

boolean  LM75_Is_Present ();

int16_t  LM75_Get_Temp_12_4 ();


/*
  BME/BMP280 I2C temperature, pressure (,humidity) sensor
*/


void  BME_Init ();

void  BME_Exec ();

void  BME_Request_Read_Temp ();

boolean  BME_Is_Present ();

float  BME_Get_Temp ();

float  BME_Get_Pressure ();

boolean BME_Has_Humidity ();

float  BME_Get_Humidity ();


/*
  Temperature sensor
*/


typedef struct
{
  float   temp;
  float   humidity;
  float   pressure;
  boolean tempIsValid;
  boolean humidityIsValid;
  boolean pressureIsValid;
} TS_VALUES;



typedef enum
{
  TS_TREND_NA,
  TS_TREND_STABLE,
  TS_TREND_UP,
  TS_TREND_DOWN,
  TS_TREND_FASTUP,
  TS_TREND_FASTDOWN
}
  TS_TREND;


void  TS_Init ();

void  TS_Exec ();

void  TS_Request_Readout ();

boolean  TS_Is_Present ();

TS_VALUES  *TS_Get_Values ();

void  TS_Read (CMD_PROC *p);

void  TS_Store_History ();

void  TS_Clear_History();

void  TS_Read_History (CMD_PROC *p);

TS_TREND  TS_Get_Trend ();


/*
  Display driver
*/


typedef enum  _DISPLAY_SYMBOL
{
  DISPLAY_SYMBOL_0,
  DISPLAY_SYMBOL_1,
  DISPLAY_SYMBOL_2,
  DISPLAY_SYMBOL_3,
  DISPLAY_SYMBOL_4,
  DISPLAY_SYMBOL_5,
  DISPLAY_SYMBOL_6,
  DISPLAY_SYMBOL_7,
  DISPLAY_SYMBOL_8,
  DISPLAY_SYMBOL_9,
  DISPLAY_SYMBOL_HYPHEN,
  DISPLAY_SYMBOL_DEGREES,
  DISPLAY_SYMBOL_C,
  DISPLAY_SYMBOL_F,
  DISPLAY_SYMBOL_SPACE,
  DISPLAY_SYMBOL_O,
  DISPLAY_SYMBOL_V,
  DISPLAY_SYMBOL_UP,
  DISPLAY_SYMBOL_DOWN,
  DISPLAY_SYMBOL_FASTUP,
  DISPLAY_SYMBOL_FASTDOWN,
  DISPLAY_SYMBOL_CNT
}
  DISPLAY_SYMBOL;


void  Display_Init ();

void  Display_Exec ();

void  Display_Enter_Timer_Mux ();

void  Display_Exit_Timer_Mux ();

boolean  Display_Is_Suspended ();

void  Display_Suspend ();

void  Display_Resume ();

void  Display_Write_Tube_Digit (byte tube_index, byte digit);

void  Display_Write_Tube_Symbol (byte tube_index, byte symbol);

void  Display_Write_Dot (byte dot_index, boolean state);

void  Display_Clear_All_Dots ();

void  Display_Write_Sep_Left (boolean state);

void  Display_Write_Sep_Right (boolean state);


/*
  RGB driver
*/


#define RGB_SOLID_ID_CNT         18
#define RGB_GRAD_SET_ID_CNT       6


typedef enum  _RGB_METHOD
{
  RGB_METHOD_SOLID,
  RGB_METHOD_GRAD,
  RGB_METHOD_CNT
}
  RGB_METHOD;


typedef struct  _RGB_SOLID
{
  uint16_t    red;
  uint16_t    green;
  uint16_t    blue;
}
  RGB_SOLID;


typedef struct  _RGB_GRAD
{
  int16_t     val;
  int16_t     add;
  uint16_t    left;
  uint16_t    cnt;
}
  RGB_GRAD;


void  RGB_Init ();

void  RGB_Exec ();

boolean  RGB_Is_Suspended ();

void  RGB_Suspend ();

void  RGB_Resume ();

void  RGB_Select_Solid (RGB_SOLID *solid);

void  RGB_Select_Solid_Id (byte id);

void  RGB_Solid_Id_To_Settings (byte id);

void  RGB_Solid_To_Settings ();

void  RGB_Solid_Id_To_Settings_Sleep (byte id);

void  RGB_Solid_To_Settings_Sleep ();

void  RGB_Select_Grad_Set (RGB_GRAD *grad_set);

void  RGB_Select_Grad_Set_Id (byte id);

void  RGB_Grad_Set_Id_To_Settings (byte id);

void  RGB_Grad_Set_To_Settings ();


/*
  Clock
*/


// Formatting of date

typedef enum  _CLOCK_DATE_FORMAT
{
  CLOCK_DATE_FORMAT_DMY,
  CLOCK_DATE_FORMAT_MDY,
  CLOCK_DATE_FORMAT_YMD,
  CLOCK_DATE_FORMAT_CNT
}
  CLOCK_DATE_FORMAT;


// Mode of separators during displaying of date

typedef enum  _CLOCK_DATE_SEP_MODE
{
  CLOCK_DATE_SEP_MODE_ON,
  CLOCK_DATE_SEP_MODE_OFF,
  CLOCK_DATE_SEP_MODE_CNT
}
  CLOCK_DATE_SEP_MODE;


// Formatting of hour

typedef enum  _CLOCK_HOUR_FORMAT
{
  CLOCK_HOUR_FORMAT_24,
  CLOCK_HOUR_FORMAT_12,
  CLOCK_HOUR_FORMAT_CNT
}
  CLOCK_HOUR_FORMAT;


// Mode of separators during displaying of time (that is, hours-minutes-seconds display)

typedef enum  _CLOCK_TIME_SEP_MODE
{
  CLOCK_TIME_SEP_MODE_BLINK,
  CLOCK_TIME_SEP_MODE_AMPM,
  CLOCK_TIME_SEP_MODE_ON,
  CLOCK_TIME_SEP_MODE_OFF,
  CLOCK_TIME_SEP_MODE_CNT
}
  CLOCK_TIME_SEP_MODE;


// Formatting of temperature

typedef enum  _CLOCK_TEMP_FORMAT
{
  CLOCK_TEMP_FORMAT_CELSIUS,
  CLOCK_TEMP_FORMAT_FAHRENHEIT,
  CLOCK_TEMP_FORMAT_CNT
}
  CLOCK_TEMP_FORMAT;


// A screen displays specific information on the tubes and the separators.

typedef enum  _CLOCK_SCREEN_MODE
{
  CLOCK_SCREEN_MODE_NOTHING,
  CLOCK_SCREEN_MODE_VERSION,
  CLOCK_SCREEN_MODE_TIME,
  CLOCK_SCREEN_MODE_DATE,
  CLOCK_SCREEN_MODE_TEMP,
  CLOCK_SCREEN_MODE_PRES,
  CLOCK_SCREEN_MODE_HYGR,
  CLOCK_SCREEN_MODE_ADJUSTING
}
  CLOCK_SCREEN_MODE;


typedef  void  CLOCK_REPORT_SEC_FN (TimeElements *tm, boolean time_set);


void  Clock_Init ();

void  Clock_Exec ();

void  Clock_Suspend ();

void  Clock_Resume ();

void  Clock_Read (CMD_PROC *p);

void  Clock_Write (time_t t);

void  Clock_Set_Adjust (byte adjust_hour, byte adjust_min, byte adjust_sec, boolean adjust_sub);

void  Clock_Set_Cur_Adjust (byte cur_adjust);

void  Clock_Apply_Cur_Adjust ();

void  Clock_Apply_Next_Adjust ();

void  Clock_Set_Adjusting (boolean adjusting);

void  Clock_Start_Cycling ();

void  Clock_Set_Report_Sec (CLOCK_REPORT_SEC_FN *report_sec_fn);

boolean  Clock_Is_Time_Set ();

time_t  Clock_HMS_To_Secs (byte hour, byte min, byte sec);

double Clock_Get_Uptime ();


/*
  Main
*/


void  Main_Init ();

void  Main_Exec ();

void  Main_NTP_Refresh ();


/*
  Settings
*/


#define SETTINGS_WIFI_DNS_CNT       2

#define SETTINGS_CLOCK_ADJUST_CNT   4


// Settings structure
//
// [1] To specify an IP address instead of a hostname, pass the IP address as the
//     hostname, e.g. "192.168.1.1".

typedef struct  _SETTINGS
{
  uint8_t     id_product[30];                           // Product identification
  uint16_t    id_rev;                                   // Revision of this structure

  // Revision 0000h

  char        wifi_ssid[WIFI_SSID_MAX_LEN + 1];         // Network name, zero-terminated string
  char        wifi_password[WIFI_PASSWORD_MAX_LEN + 1]; // Password for authentication, zero-terminated string
  uint32_t    wifi_local_ip;                            // Local IP address
  uint32_t    wifi_subnet;                              // Subnet mask
  uint32_t    wifi_gateway;                             // Gateway IP address
  uint32_t    wifi_dns_list[SETTINGS_WIFI_DNS_CNT];     // List of DNS server IP addresses
  boolean     wifi_use_static;                          // Use static IP address instead of DHCP y/n

  boolean     serial_echo_commands;                     // Echo received commands over serial interface y/n

  boolean     cs_echo_commands;                         // Echo received commands back to network client y/n

  char        ntp_hostname[NTP_HOSTNAME_MAX_LEN + 1];   // NTP server hostname, zero-terminated string [1]
  uint32_t    ntp_addr;                                 // NTP server IP address
  boolean     ntp_use_addr;

  boolean     owtemp_enabled;

  boolean     lm75_enabled;
  byte        lm75_adsel;                               // Address selection (0..7)

  byte        clock_adjust_hour;                        // Hours to adjust (0..23)
  byte        clock_adjust_min;                         // Minutes to adjust (0..59)
  byte        clock_adjust_sec;                         // Seconds to adjust (0..59)
  boolean     clock_adjust_sub;                         // Subtract (true) or add (false) adjustment time
  boolean     clock_show_date;                          // Show date (true) or not (false)
  byte        clock_date_format;                        // Assign value from CLOCK_DATE_FORMAT enumeration
  byte        clock_date_sep_mode;                      // Assign value from CLOCK_DATE_SEP_MODE enumeration
  byte        clock_hour_format;                        // Assign value from CLOCK_HOUR_FORMAT enumeration
  boolean     clock_hour_lz;                            // Show leading zero in hour (true) or not (false)
  byte        clock_time_sep_mode;                      // Assign value from CLOCK_TIME_SEP_MODE enumeration
  boolean     clock_show_temp;                          // Show temperature (true) or not (false)
  byte        clock_temp_format;                        // Assign value from CLOCK_TEMP_FORMAT enumeration

  byte        rgb_method;
  RGB_SOLID   rgb_solid;
  RGB_GRAD    rgb_grad[3];
  RGB_SOLID   rgb_sleep_solid;

  boolean     main_wifi_connect;
  boolean     main_ntp_request;
  byte        main_ntp_refresh_hour;
  byte        main_ntp_refresh_min;
  byte        main_ntp_refresh_sec;
  boolean     main_cmd_server_start;
  byte        main_sleep_hour;
  byte        main_sleep_min;
  byte        main_sleep_sec;
  byte        main_wakeup_hour;
  byte        main_wakeup_min;
  byte        main_wakeup_sec;
  boolean     main_sleep_enabled;

  // Revision 0001h

  byte        clock_cur_adjust;                         // Active adjustment settings (0..3)
  byte        clock_adjust_hour_2;                      // Hours to adjust (0..23)
  byte        clock_adjust_min_2;                       // Minutes to adjust (0..59)
  byte        clock_adjust_sec_2;                       // Seconds to adjust (0..59)
  boolean     clock_adjust_sub_2;                       // Subtract (true) or add (false) adjustment time
  byte        clock_adjust_hour_3;                      // Hours to adjust (0..23)
  byte        clock_adjust_min_3;                       // Minutes to adjust (0..59)
  byte        clock_adjust_sec_3;                       // Seconds to adjust (0..59)
  boolean     clock_adjust_sub_3;                       // Subtract (true) or add (false) adjustment time
  byte        clock_adjust_hour_4;                      // Hours to adjust (0..23)
  byte        clock_adjust_min_4;                       // Minutes to adjust (0..59)
  byte        clock_adjust_sec_4;                       // Seconds to adjust (0..59)
  boolean     clock_adjust_sub_4;                       // Subtract (true) or add (false) adjustment time

  // Revision 0002h

  char        wifi_hostname[WIFI_HOSTNAME_MAX_LEN + 1]; // Hostname, zero-terminated string


  // Revision 0003h

  boolean     clock_use_dst;                            // Use timezone info for adjustment 
  boolean     bme_enabled;


  // To extend the structure: add new settings here and increment the revision.
}
  SETTINGS;


// All settings as written to and read from EEPROM.

SETTINGS    settings;


uint8_t     settings_id_product[30] =
{
  'V', 'F', 'D', ' ', 'C', 'L', 'O', 'C',
  'K', ' ', 'W', 'I', 'F', 'I', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void  Settings_Init ();

void  Settings_Reset ();

boolean  Settings_Clear ();

boolean  Settings_Write ();

void  Settings_Read ();

void  Settings_Dump (CMD_PROC *p);


/*
  Timer funktions
*/

typedef struct {
  boolean       isRunning;
  unsigned long start;
  unsigned long end;
} TIMER;

void          Timer_Start( TIMER &t, unsigned long duration, unsigned long *now=NULL );
void          Timer_Restart( TIMER &t, unsigned long increment );
void          Timer_Clear( TIMER &t, unsigned long *now=NULL );
boolean       Timer_IsRunning( TIMER &t, unsigned long *now=NULL );
unsigned long Timer_GetResidual( TIMER &t, unsigned long *now=NULL );


/*
  Arduino loop function
*/


void  loop ()
{
  Serial_Exec();
  WIFI_Exec();
  Cmd_Server_Exec();
  NTP_Exec();
  OW_Exec();
  TS_Exec();
  Display_Exec();
  RGB_Exec();
  Clock_Exec();
  Main_Exec();
}


/*
  Arduino setup function
*/


void  setup ()
{
  // Serial must be initialized first
  Serial_Init();

  // Join the I2C bus as master
  Wire.begin(PIN_SDA,PIN_SDL,100000);

  // We need settings early on
  Settings_Init();

  WIFI_Init();
  Cmd_Server_Init();
  NTP_Init();
  OW_Init(PIN_DQ,Display_Enter_Timer_Mux,Display_Exit_Timer_Mux);
  TS_Init();
  Display_Init();
  RGB_Init();
  Clock_Init();
  Main_Init();

  // Delay to allow the serial interface come up so the serial monitor doesn't
  // receive rubbish or loose incoming text.
  delay(100);
}
