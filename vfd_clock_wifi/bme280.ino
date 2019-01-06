/*
 *  BME280 : temperature, pressure, humidity
 *  BMP280: temparture, pressure
 */


#include <BME280I2C.h>   // https://github.com/finitespace/BME280
#include <Wire.h>


#define BME_RETRY_MS     2000

BME280I2C       bme;  
boolean         bme_enabled      = false;
boolean         bme_present      = false;
boolean         bme_read_request = false;
TIMER           bme_retry_timer;
float           bme_temp;                
float           bme_pressure;                
float           bme_humidity;                


void  BME_Request_Readout ()
{
  bme_read_request = true;
}


boolean  BME_Is_Present ()
{
  return bme_present;
}


float  BME_Get_Temp ()
{
  return bme_temp;
}

float  BME_Get_Pressure ()
{
  return bme_pressure;
}

float  BME_Get_Humidity ()
{
  return bme_humidity;
}


static void BME_Readout ()
{
   float temp(NAN), hum(NAN), pres(NAN);
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  // Presence not yet detected?
  if( !bme_present)  {
    if( bme.begin() ) {
      switch( bme.chipModel() ) {
        case BME280::ChipModel_BME280:
          bme_present = true;
#ifdef BME_DEBUG
          Serial.println("Found BME280 sensor! Success.");
#endif
          break;
        case BME280::ChipModel_BMP280:
          bme_present = true;
#ifdef BME_DEBUG
          Serial.println("Found BMP280 sensor! No Humidity available.");
#endif
          break;
        default:
          Serial.println("Found UNKNOWN sensor! Error!");
          bme_present = false;
      }
    }
    if( !bme_present ) {
#ifdef BME_DEBUG
      Serial.println("BME_Read_Out: not present");
#endif
      // Start retry timer
      Timer_Start( bme_retry_timer, BME_RETRY_MS );
      return;
    }
  }

  // Get and store values
  // Todo: handle errors (might reset presence flag)
  bme.read( pres, temp, hum, tempUnit, presUnit );
  bme_temp     = temp;
  bme_pressure = pres;
  bme_humidity = hum;

#ifdef BME_DEBUG
      Serial.printf( "BME_Read_Out: %.1f °C\n", temp );
      Serial.printf( "BME_Read_Out: %.1f Pa\n", pres );
      Serial.printf( "BME_Read_Out: %.1f %\n",  hum );
#endif

}


void  BME_Exec ()
{
  if (bme_enabled != settings.bme_enabled)
  {
    bme_enabled = settings.bme_enabled;

    if (bme_enabled)
    {
      // Transition of disabled->enabled

      // Request immediate reading of temperature
      bme_read_request = true;
      
    }
    else
    {
      // Transition of enabled->disabled

      bme_present = false;
    }
  }

  if (bme_enabled)
  {
    if (bme_read_request)
    {
      bme_read_request = false;
  
      BME_Readout();
    }
    else
    if (!bme_present)
    {
      if ( !Timer_IsRunning(bme_retry_timer) )
      {
        BME_Readout();
      }
    }
  }
}


void  BME_Init ()
{
}
