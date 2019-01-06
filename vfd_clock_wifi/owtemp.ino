
/*
  1-Wire temperature sensor
*/


typedef enum  _OWTEMP_SENSOR_TYPE
{
  OWTEMP_SENSOR_TYPE_DS18S20,
  OWTEMP_SENSOR_TYPE_DS1822,
  OWTEMP_SENSOR_TYPE_DS18B20
}
  OWTEMP_SENSOR_TYPE;


byte            owtemp_exec_state   = 0;
boolean         owtemp_enabled      = false;
boolean         owtemp_present      = false;
boolean         owtemp_read_request = false;
unsigned long   owtemp_timer_ms;
byte            owtemp_sensor_type;           // Sensor type (assign value from OWTEMP_SENSOR_TYPE enumeration)
byte            owtemp_xfr_buf[1+8+1+9];
int16_t         owtemp_reg;                   // Temperature register, 16-bit
byte            owtemp_frac_cnt;              // Number of fractional digits in temperature register


#define OWTEMP_RETRY_MS   5000


void  OWTemp_Request_Read_Temp ()
{
  owtemp_read_request = true;
}


boolean  OWTemp_Is_Present ()
{
  return owtemp_present;
}


int16_t  OWTemp_Get_Temp ()
{
  return owtemp_reg;
}


byte  OWTemp_Get_Frac_Cnt ()
{
  return owtemp_frac_cnt;
}


void  OWTemp_Exec_Set_State (byte s)
{
  owtemp_exec_state = s;
}


void  OWTemp_Exec_Enum_First ()
{
  OW_Enum_First_Start();

  OWTemp_Exec_Set_State(1);
}


void  OWTemp_Exec_Enum_Next ()
{
  OW_Enum_Next_Start();

  OWTemp_Exec_Set_State(1);
}


void  OWTemp_Exec ()
{
  switch (owtemp_exec_state)
  {
    case 0:
    {
      // Handle changes in setting
      if (owtemp_enabled != settings.owtemp_enabled)
      {
        owtemp_enabled = settings.owtemp_enabled;

        if (owtemp_enabled)
        {
          // Transition of disabled->enabled

          // Request immediate reading of temperature
          owtemp_read_request = true;
        }
        else
        {
          // Transition of enabled->disabled

          owtemp_present = false;
        }
      }

      if (owtemp_enabled)
      {
        if (owtemp_read_request)
        {
          owtemp_read_request = false;

          OWTemp_Exec_Enum_First();
          return;
        }
        else
        if (!owtemp_present)
        {
          if ((millis() - owtemp_timer_ms) > OWTEMP_RETRY_MS)
          {
            OWTemp_Exec_Enum_First();
            return;
          }
        }
      }

      return;
    }

    case 1:
    {
      if (OW_Enum_Is_Busy()) return;

      byte *rom_code = OW_Enum_Get_ROM_Code();
      if (!rom_code)
      {
#ifdef  OWTEMP_DEBUG
        Serial.println("OW ENUM: not found");
#endif

        // Restart the retry timer
        owtemp_timer_ms = millis();
        owtemp_present = false;

        OWTemp_Exec_Set_State(0);
        return;
      }

#ifdef  OWTEMP_DEBUG
      Serial.printf("OW ENUM: %02X-%02X%02X%02X%02X%02X%02X-%02X - crc %s\n",
                    rom_code[0],
                    rom_code[6],
                    rom_code[5],
                    rom_code[4],
                    rom_code[3],
                    rom_code[2],
                    rom_code[1],
                    rom_code[7],
                    OW_Enum_Is_CRC_Valid() ? "ok" : "wrong");
#endif

      if (!OW_Enum_Is_CRC_Valid())
      {
        // Enumerate next slave
        OWTemp_Exec_Enum_Next();
        return;
      }

      // Check family code
      if (rom_code[0] == 0x10) owtemp_sensor_type = OWTEMP_SENSOR_TYPE_DS18S20; else
      if (rom_code[0] == 0x22) owtemp_sensor_type = OWTEMP_SENSOR_TYPE_DS1822; else
      if (rom_code[0] == 0x28) owtemp_sensor_type = OWTEMP_SENSOR_TYPE_DS18B20; else
      {
        // Enumerate next slave
        OWTemp_Exec_Enum_Next();
        return;
      }

      // MATCH ROM command
      owtemp_xfr_buf[0] = 0x55;
      memcpy(&owtemp_xfr_buf[1],rom_code,8);

      // CONVERT TEMPERATURE command
      owtemp_xfr_buf[9] = 0x44;

      // Reset signal
      OW_Bus_Reset();

      // Write-only transaction
      OW_Touch_Bits_Start(owtemp_xfr_buf,(1+8+1)*8,true);

#ifdef  OWTEMP_DEBUG
      Serial.println("Converting temperature");
#endif

      OWTemp_Exec_Set_State(2);
      return;
    }

    case 2:
    {
      if (OW_Touch_Bits_Is_Busy()) return;

      // Start wait timer
      owtemp_timer_ms = millis();

#ifdef  OWTEMP_DEBUG
      Serial.println("Waiting 800 ms");
#endif

      OWTemp_Exec_Set_State(3);
      return;
    }

    case 3:
    {
      // Wait timer
      if ((millis() - owtemp_timer_ms) <= 800) return;

      // READ SCRATCHPAD command
      owtemp_xfr_buf[9] = 0xBE;
      memset(&owtemp_xfr_buf[10],0xFF,9);

      // Reset signal
      OW_Bus_Reset();

      // Read-write transaction
      OW_Touch_Bits_Start(owtemp_xfr_buf,(1+8+1+9)*8,false);

#ifdef  OWTEMP_DEBUG
      Serial.println("Reading scratchpad");
#endif

      OWTemp_Exec_Set_State(4);
      return;
    }

    case 4:
    {
      if (OW_Touch_Bits_Is_Busy()) return;

      byte crc = OW_CRC8_Buf(0,&owtemp_xfr_buf[10],9);
      if (crc != 0x00)
      {
#ifdef  OWTEMP_DEBUG
        Serial.println("Invalid CRC");
#endif

        // Enumerate next slave
        OWTemp_Exec_Enum_Next();
        return;
      }

      owtemp_reg = (owtemp_xfr_buf[10+1] << 8) | owtemp_xfr_buf[10+0];

      if ((owtemp_sensor_type == OWTEMP_SENSOR_TYPE_DS18B20) || (owtemp_sensor_type == OWTEMP_SENSOR_TYPE_DS1822))
      {
        // Temperature register: sssssxxxxxxx.xxxx b
        //                       <------><------->
        //                          MSB     LSB
        owtemp_frac_cnt = 4;
      }
      else
      if (owtemp_sensor_type == OWTEMP_SENSOR_TYPE_DS18S20)
      {
        // Temperature register: ssssssssxxxxxxx.x b
        //                       <------><------->
        //                          MSB     LSB
        owtemp_frac_cnt = 1;
      }

#ifdef  OWTEMP_DEBUG
      Serial.printf("Temperature measured: reg=%04Xh, frac_cnt=%u\n",owtemp_reg,owtemp_frac_cnt);
#endif

      owtemp_present = true;
      OWTemp_Exec_Set_State(0);
      return;
    }
  }
}


void  OWTemp_Init ()
{
}

