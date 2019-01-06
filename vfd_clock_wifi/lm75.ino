
/*
  LM75/SE95D I2C temperature sensor
*/


// Address of LM75/SE95D with A[2..0]=000b

#define I2C_AD_LM75   0x48


boolean         lm75_enabled      = false;
boolean         lm75_present      = false;
boolean         lm75_read_request = false;
unsigned long   lm75_retry_ms;
int16_t         lm75_temp;                  // Temperature, signed 8.4, bits 15-12 are sign-extended


#define LM75_RETRY_MS     2000


void  LM75_Request_Read_Temp ()
{
  lm75_read_request = true;
}


boolean  LM75_Is_Present ()
{
  return lm75_present;
}


int16_t  LM75_Get_Temp_12_4 ()
{
  return lm75_temp;
}


void  LM75_Read_Temp ()
{
  byte  buf[2];

  Wire.requestFrom(I2C_AD_LM75 + settings.lm75_adsel,sizeof(buf));
  if (Wire.available() < sizeof(buf))
  {
#ifdef  LM75_DEBUG
    Serial.println("LM75_Read_Temp: not present");
#endif

    // Start retry timer
    lm75_retry_ms = millis();

    // Report temperature sensor not present
    lm75_present = false;
    return;
  }

  for (byte u = 0; u  < sizeof(buf); u++) buf[u] = Wire.read();

  lm75_temp = (int16_t)((buf[0] << 8) | buf[1]);
  lm75_temp /= 16;

#ifdef  LM75_DEBUG
  Serial.printf("LM75_Read_Temp: [%02Xh,%02Xh]\n",buf[0],buf[1]);
#endif

  // Report temperature sensor present
  lm75_present = true;
}


void  LM75_Exec ()
{
  if (lm75_enabled != settings.lm75_enabled)
  {
    lm75_enabled = settings.lm75_enabled;

    if (lm75_enabled)
    {
      // Transition of disabled->enabled

      // Request immediate reading of temperature
      lm75_read_request = true;
    }
    else
    {
      // Transition of enabled->disabled

      lm75_present = false;
    }
  }

  if (lm75_enabled)
  {
    if (lm75_read_request)
    {
      lm75_read_request = false;
  
      LM75_Read_Temp();
    }
    else
    if (!lm75_present)
    {
      if ((millis() - lm75_retry_ms) > LM75_RETRY_MS)
      {
        LM75_Read_Temp();
      }
    }
  }
}


void  LM75_Init ()
{
}

