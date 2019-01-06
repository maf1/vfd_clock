
/*
  Temperature sensor
*/


#define TS_SAMPLE_TIME_MS   (15*60*1000)  // sample every 15min for 3h
#define TS_HISTORY_SAMPLES  12
#define TS_TREND_THRESHOLD1 (1.0*100)     // in Pa   
#define TS_TREND_THRESHOLD2 (3.0*100)     // http://www.wetterstationen.info/forum/wetter-statistiken/funktion-der-offiziellen-luftdruck-tendenz-mit-arduino/


typedef enum  _TS_SRC
{
  TS_SRC_NONE,
  TS_SRC_OWTEMP,
  TS_SRC_LM75,
  TS_SRC_BME

}
  TS_SRC;


TS_SRC    ts_src;

TIMER     ts_sample_timer;
float     ts_history[TS_HISTORY_SAMPLES];


boolean  TS_Is_Present ()
{
  if (BME_Is_Present())
  {
    ts_src = TS_SRC_BME;
    return true;
  }
  else
  if (OWTemp_Is_Present())
  {
    ts_src = TS_SRC_OWTEMP;
    return true;
  }
  else
  if (LM75_Is_Present())
  {
    ts_src = TS_SRC_LM75;
    return true;
  }
  else
  {
    ts_src = TS_SRC_NONE;
    return false;
  }
}


TS_VALUES *TS_Get_Values ()
{
  static TS_VALUES  ts_values; 
  ts_values.temp     = NAN;
  ts_values.humidity = NAN;
  ts_values.pressure = NAN;

  switch( ts_src ) {
  
    case TS_SRC_OWTEMP:
      ts_values.temp = OWTemp_Get_Temp()+0.5;
      for( int i=OWTemp_Get_Frac_Cnt(); i; i-- )
        ts_values.temp /= 2.0;
      break;
      
    case TS_SRC_LM75:
      ts_values.temp = (LM75_Get_Temp_12_4()+0.5)/16;
      break;

    case TS_SRC_BME:
      ts_values.temp     = BME_Get_Temp();
      ts_values.pressure = BME_Get_Pressure();
      ts_values.humidity = BME_Get_Humidity();
      break;
      
  }

  return &ts_values;
}


void  TS_Store_History()
{
  int i;
  
  // Time to sample?
  if( Timer_IsRunning(ts_sample_timer) )
    return;
  Timer_Restart( ts_sample_timer, TS_SAMPLE_TIME_MS );

  // Shift values in history and store newest one
  for( i=0; i<TS_HISTORY_SAMPLES-1; i++ )
    ts_history[i] = ts_history[i+1];
  ts_history[TS_HISTORY_SAMPLES-1] = TS_Get_Values()->pressure;
}


void  TS_Clear_History()
{
  int i;
  
  for( i=0; i<TS_HISTORY_SAMPLES; i++ )
    ts_history[i] = 0;

  Timer_Clear( ts_sample_timer );
}


TS_TREND TS_Get_Trend()
{
  float pressure = TS_Get_Values()->pressure;
  float delta;
  
  if( ts_history[0]==0.0 || isnan(ts_history[0]) || isnan(pressure) )
    return TS_TREND_NA;

  delta = pressure - ts_history[0];
  
  if( fabs(delta)<TS_TREND_THRESHOLD1 )
    return TS_TREND_STABLE;

  if( fabs(delta)<TS_TREND_THRESHOLD2 )
    return delta>0 ? TS_TREND_UP : TS_TREND_DOWN;

  return delta>0 ? TS_TREND_FASTUP : TS_TREND_FASTDOWN;
}


void TS_Read (CMD_PROC *p)
{

  if( TS_Is_Present() ) {
    char       buffer[40];
    int        len;
    TS_VALUES *values = TS_Get_Values();
    
    len = sprintf( buffer, "%.1f °%c", (settings.clock_temp_format==CLOCK_TEMP_FORMAT_CELSIUS)?values->temp:(values->temp*9+160)/5, (settings.clock_temp_format==CLOCK_TEMP_FORMAT_CELSIUS)?'C':'F' );
    if( !isnan(values->pressure) )
      len += sprintf( buffer+len, " %.1f hPa", values->pressure/100 );
    if( !isnan(values->humidity) )
      len += sprintf( buffer+len, " %.1f %%RH", values->humidity );
    p->emit_str_fn( p, buffer, len );
  }
  else
    p->emit_str_fn( p, "No sensor or values available", 0 );
    
}


void TS_Read_History (CMD_PROC *p)
{
  int lines = 0;

  if( ts_src==TS_SRC_BME ) {
    char       buffer[40];
    int        i;

    for( i=0; i<TS_HISTORY_SAMPLES; i++ ) {
      if( !isnan(ts_history[i]) && ts_history[i]>0 ) {
        int len = sprintf( buffer, "%3d min %.1f hPa", ((TS_HISTORY_SAMPLES-i)*TS_SAMPLE_TIME_MS-Timer_GetResidual(ts_sample_timer))/60000, ts_history[i]/100  );
        p->emit_str_fn( p, buffer, len );
        lines++;
      }
    }

    if( !isnan(BME_Get_Pressure()) ) {
      int len = sprintf( buffer, "%3d min %.1f hPa", 0, BME_Get_Pressure()/100  );
      p->emit_str_fn( p, buffer, len );
      lines++;
    }   
  }
  
  if( !lines )
    p->emit_str_fn( p, "No sensor or values available", 0 );
}


void  TS_Request_Readout ()
{
  switch( ts_src ) {
    case TS_SRC_OWTEMP:
      OWTemp_Request_Read_Temp();
      break;
    case TS_SRC_LM75: 
      LM75_Request_Read_Temp(); 
      break;
    case TS_SRC_BME: 
      BME_Request_Readout();
      break;
  }
}


void  TS_Exec ()
{
  OWTemp_Exec();
  LM75_Exec();
  BME_Exec();
}


void  TS_Init ()
{
  OWTemp_Init();
  LM75_Init();
  BME_Init();
}
