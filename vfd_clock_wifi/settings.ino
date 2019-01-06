
/*
  Program settings
*/


#define SETTINGS_REVISION 0x0003


char  *Settings_IP_Addr (uint32_t addr)
{
  static  char  addr_str[3+1+3+1+3+1+3+1];

  sprintf
  (
    addr_str,
    "%u.%u.%u.%u",
    addr & 255,
    (addr >> 8) & 255,
    (addr >> 16) & 255,
    (addr >> 24) & 255
  );

  return addr_str;
}


void  Settings_Id_Dump (CMD_PROC *p, char *line)
{
}


void  Settings_WIFI_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"WIFI:",0);

  cnt = sprintf(line,"* SSID:     %s",settings.wifi_ssid);
  p->emit_str_fn(p,line,cnt);
/*
  cnt = sprintf(line,"* Password: %s",settings.wifi_password);
  p->emit_str_fn(p,line,cnt);
*/
  cnt = sprintf(line,"* Local:    %s",Settings_IP_Addr(settings.wifi_local_ip));
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Subnet:   %s",Settings_IP_Addr(settings.wifi_subnet));
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Gateway:  %s",Settings_IP_Addr(settings.wifi_gateway));
  p->emit_str_fn(p,line,cnt);

  for (uint32_t u = 0; u < SETTINGS_WIFI_DNS_CNT; u++)
  {
    cnt = sprintf(line,"* DNS %u:    %s",u+1,Settings_IP_Addr(settings.wifi_dns_list[u]));
    p->emit_str_fn(p,line,cnt);
  }

  cnt = sprintf(line,"* Method:   %s",settings.wifi_use_static ? "static" : "dynamic");
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Hostname: %s",settings.wifi_hostname);
  p->emit_str_fn(p,line,cnt);
}


void  Settings_Serial_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"Serial:",0);

  cnt = sprintf(line,"* Echo commands: %s",settings.serial_echo_commands ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);
}


void  Settings_CS_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"Command server:",0);

  cnt = sprintf(line,"* Echo commands: %s",settings.cs_echo_commands ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);
}


void  Settings_NTP_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"NTP:",0);

  cnt = sprintf(line,"* Hostname: %s",settings.ntp_hostname);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Address:  %s",Settings_IP_Addr(settings.ntp_addr));
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Method:   %s",settings.ntp_use_addr ? "use address" : "look up hostname");
  p->emit_str_fn(p,line,cnt);
}


void  Settings_OWTemp_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"OWTemp:",0);

  cnt = sprintf(line,"* Enabled: %s",settings.owtemp_enabled ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);
}


void  Settings_LM75_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"LM75/SE95D:",0);

  cnt = sprintf(line,"* Enabled: %s",settings.lm75_enabled ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* ADSEL:   %u",settings.lm75_adsel);
  p->emit_str_fn(p,line,cnt);
}


void  Settings_BME_Dump (CMD_PROC *p, char *line)
{
  int    cnt;

  p->emit_str_fn(p,"BME/BMP280:",0);

  cnt = sprintf(line,"* Enabled: %s",settings.bme_enabled ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);
}


void  Settings_Clock_Dump (CMD_PROC *p, char *line)
{
  int    cnt;
  char  *s;

  p->emit_str_fn(p,"Clock:",0);

  cnt = sprintf(line,"* Active adjustment:  %u",settings.clock_cur_adjust + 1);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Adjust 1:           %c%02u:%02u:%02u",
                settings.clock_adjust_sub ? '-' : '+',
                settings.clock_adjust_hour,
                settings.clock_adjust_min,
                settings.clock_adjust_sec);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Adjust 2:           %c%02u:%02u:%02u",
                settings.clock_adjust_sub_2 ? '-' : '+',
                settings.clock_adjust_hour_2,
                settings.clock_adjust_min_2,
                settings.clock_adjust_sec_2);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Adjust 3:           %c%02u:%02u:%02u",
                settings.clock_adjust_sub_3 ? '-' : '+',
                settings.clock_adjust_hour_3,
                settings.clock_adjust_min_3,
                settings.clock_adjust_sec_3);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Adjust 4:           %c%02u:%02u:%02u",
                settings.clock_adjust_sub_4 ? '-' : '+',
                settings.clock_adjust_hour_4,
                settings.clock_adjust_min_4,
                settings.clock_adjust_sec_4);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Use DST:            %s", settings.clock_use_dst ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);
  
  cnt = sprintf(line,"* Show date:          %s",settings.clock_show_date ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  if (settings.clock_date_format == CLOCK_DATE_FORMAT_DMY) s = "DD-MM-YY"; else
  if (settings.clock_date_format == CLOCK_DATE_FORMAT_MDY) s = "MM-DD-YY"; else
  if (settings.clock_date_format == CLOCK_DATE_FORMAT_YMD) s = "YY-MM-DD"; else
                                                           s = "???";
  cnt = sprintf(line,"* Date format:        %s",s);
  p->emit_str_fn(p,line,cnt);

  if (settings.clock_date_sep_mode == CLOCK_DATE_SEP_MODE_ON)    s = "always on"; else
  if (settings.clock_date_sep_mode == CLOCK_DATE_SEP_MODE_OFF)   s = "always off"; else
                                                                 s = "???";
  cnt = sprintf(line,"* Date separators:    %s",s);
  p->emit_str_fn(p,line,cnt);

  if (settings.clock_hour_format == CLOCK_HOUR_FORMAT_24) s = "24"; else
  if (settings.clock_hour_format == CLOCK_HOUR_FORMAT_12) s = "12"; else
                                                          s = "???";
  cnt = sprintf(line,"* Hour format:        %s",s);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Hour leading zero:  %s",settings.clock_hour_lz ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  if (settings.clock_time_sep_mode == CLOCK_TIME_SEP_MODE_BLINK) s = "blinking"; else
  if (settings.clock_time_sep_mode == CLOCK_TIME_SEP_MODE_AMPM)  s = "AM/PM indication"; else
  if (settings.clock_time_sep_mode == CLOCK_TIME_SEP_MODE_ON)    s = "always on"; else
  if (settings.clock_time_sep_mode == CLOCK_TIME_SEP_MODE_OFF)   s = "always off"; else
                                                                 s = "???";
  cnt = sprintf(line,"* Time separators:    %s",s);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Show temperature:   %s",settings.clock_show_temp ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  if (settings.clock_temp_format == CLOCK_TEMP_FORMAT_CELSIUS)    s = "Celsius"; else
  if (settings.clock_temp_format == CLOCK_TEMP_FORMAT_FAHRENHEIT) s = "Fahrenheit"; else
                                                                  s = "???";
  cnt = sprintf(line,"* Temperature format: %s",s);
  p->emit_str_fn(p,line,cnt);
}


int  Settings_Sprintf_Grad_S16 (char *s, int16_t i)
{
  if (i >= 0)
  {
    return sprintf(s," +%04u",i);
  }
  else
  {
    i = -i;
    return sprintf(s," -%04u",i);
  }
}


void  Settings_RGB_Dump (CMD_PROC *p, char *line)
{
  int    cnt;
  char  *s;

  p->emit_str_fn(p,"RGB:",0);

  if (settings.rgb_method == RGB_METHOD_SOLID) s = "solid"; else
  if (settings.rgb_method == RGB_METHOD_GRAD)  s = "gradient"; else
                                               s = "???";
  cnt = sprintf(line,"* Method:    %s",s);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Solid RGB: %04u %04u %04u",
                settings.rgb_solid.red,
                settings.rgb_solid.green,
                settings.rgb_solid.blue);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Gradient:  RED  ");
  cnt += Settings_Sprintf_Grad_S16(line+cnt,settings.rgb_grad[0].val);
  cnt += Settings_Sprintf_Grad_S16(line+cnt,settings.rgb_grad[0].add);
  cnt += sprintf(line+cnt," %04u %04u",settings.rgb_grad[0].left,settings.rgb_grad[0].cnt);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"*            GREEN");
  cnt += Settings_Sprintf_Grad_S16(line+cnt,settings.rgb_grad[1].val);
  cnt += Settings_Sprintf_Grad_S16(line+cnt,settings.rgb_grad[1].add);
  cnt += sprintf(line+cnt," %04u %04u",settings.rgb_grad[1].left,settings.rgb_grad[1].cnt);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"*            BLUE ");
  cnt += Settings_Sprintf_Grad_S16(line+cnt,settings.rgb_grad[2].val);
  cnt += Settings_Sprintf_Grad_S16(line+cnt,settings.rgb_grad[2].add);
  cnt += sprintf(line+cnt," %04u %04u",settings.rgb_grad[2].left,settings.rgb_grad[2].cnt);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Sleep RGB: %04u %04u %04u",
                settings.rgb_sleep_solid.red,
                settings.rgb_sleep_solid.green,
                settings.rgb_sleep_solid.blue);

  p->emit_str_fn(p,line,cnt);
}


void  Settings_Main_Dump (CMD_PROC *p, char *line)
{
  int           cnt;
  
  p->emit_str_fn(p,"Main:",0);

  cnt = sprintf(line,"* WIFI connect:     %s",settings.main_wifi_connect ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* NTP request:      %s",settings.main_ntp_request ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* NTP refresh rate: %02u:%02u:%02u",
                settings.main_ntp_refresh_hour,
                settings.main_ntp_refresh_min,
                settings.main_ntp_refresh_sec);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Command server:   %s",settings.main_cmd_server_start ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Sleep time:       %02u:%02u:%02u",
                settings.main_sleep_hour,
                settings.main_sleep_min,
                settings.main_sleep_sec);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,
                "* Wakeup time:      %02u:%02u:%02u",
                settings.main_wakeup_hour,
                settings.main_wakeup_min,
                settings.main_wakeup_sec);

  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Sleep mode:       %s",settings.main_sleep_enabled ? "yes" : "no");
  p->emit_str_fn(p,line,cnt);
}


void  Settings_Dump (CMD_PROC *p)
{
  char   line[80];

  Settings_Id_Dump(p,line);
  Settings_WIFI_Dump(p,line);
  Settings_Serial_Dump(p,line);
  Settings_CS_Dump(p,line);
  Settings_NTP_Dump(p,line);
  Settings_OWTemp_Dump(p,line);
  Settings_LM75_Dump(p,line);
  Settings_BME_Dump(p,line);
  Settings_Clock_Dump(p,line);
  Settings_RGB_Dump(p,line);
  Settings_Main_Dump(p,line);
}


boolean  Is_Valid_ASCII7Z (char *s, uint32_t buf_size)
{
  char  c;

  for (;;)
  {
    buf_size--;
    if (buf_size == 0) return false;

    c = (*s);
    if (c == 0) return true;
    if ((c < 32) || (c > 126)) return false;

    s++;
  }
}


boolean  Is_Valid_Boolean (boolean b)
{
  return ((b == true) || (b == false));
}


boolean  Is_In_Range (uint32_t u, uint32_t min, uint32_t max)
{
  return ((u >= min) && (u <= max));
}


void  Settings_Set_Up_Rev_0001 ()
{
  settings.clock_cur_adjust    = 0;
  settings.clock_adjust_hour_2 = 0;
  settings.clock_adjust_min_2  = 0;
  settings.clock_adjust_sec_2  = 0;
  settings.clock_adjust_sub_2  = 0;
  settings.clock_adjust_hour_3 = 0;
  settings.clock_adjust_min_3  = 0;
  settings.clock_adjust_sec_3  = 0;
  settings.clock_adjust_sub_3  = 0;
  settings.clock_adjust_hour_4 = 0;
  settings.clock_adjust_min_4  = 0;
  settings.clock_adjust_sec_4  = 0;
  settings.clock_adjust_sub_4  = 0;
}


void  Settings_Set_Up_Rev_0002 ()
{
  strcpy(settings.wifi_hostname,"VFD-Clock-ESP32");
}


void  Settings_Set_Up_Rev_0003 ()
{
  settings.clock_use_dst = 0;
  settings.bme_enabled = 0;
}


boolean  Settings_Check_Validity ()
{
#ifdef  SETTINGS_DEBUG
  Serial.printf("Settings_Check_Validity: current revision is %04Xh\n",settings.id_rev);
#endif

  // Check parameters of Revision 0000h

  if (memcmp(&settings.id_product,&settings_id_product,sizeof(settings_id_product)) != 0) return false;
  // settings.id_rev: Any value is accepted.
  if (settings.id_rev!=SETTINGS_REVISION) return false;

  if (!Is_Valid_ASCII7Z(settings.wifi_ssid,sizeof(settings.wifi_ssid))) return false;
  if (!Is_Valid_ASCII7Z(settings.wifi_password,sizeof(settings.wifi_password))) return false;
  if (!Is_Valid_Boolean(settings.wifi_use_static)) return false;

  if (!Is_Valid_Boolean(settings.serial_echo_commands)) return false;

  if (!Is_Valid_Boolean(settings.cs_echo_commands)) return false;

  if (!Is_Valid_ASCII7Z(settings.ntp_hostname,sizeof(settings.ntp_hostname))) return false;
  if (!Is_Valid_Boolean(settings.ntp_use_addr)) return false;

  if (!Is_Valid_Boolean(settings.owtemp_enabled)) return false;

  if (!Is_Valid_Boolean(settings.lm75_enabled)) return false;
  if (!Is_In_Range(settings.lm75_adsel,0,7)) return false;

  if (!Is_In_Range(settings.clock_adjust_hour,0,23)) return false;
  if (!Is_In_Range(settings.clock_adjust_min,0,59)) return false;
  if (!Is_In_Range(settings.clock_adjust_sec,0,59)) return false;
  if (!Is_Valid_Boolean(settings.clock_adjust_sub)) return false;
  if (!Is_Valid_Boolean(settings.clock_show_date)) return false;
  if (!Is_In_Range(settings.clock_date_format,0,CLOCK_DATE_FORMAT_CNT-1)) return false;
  if (!Is_In_Range(settings.clock_date_sep_mode,0,CLOCK_DATE_SEP_MODE_CNT-1)) return false;
  if (!Is_In_Range(settings.clock_hour_format,0,CLOCK_HOUR_FORMAT_CNT-1)) return false;
  if (!Is_In_Range(settings.clock_time_sep_mode,0,CLOCK_TIME_SEP_MODE_CNT-1)) return false;
  if (!Is_Valid_Boolean(settings.clock_hour_lz)) return false;
  if (!Is_Valid_Boolean(settings.clock_show_temp)) return false;
  if (!Is_In_Range(settings.clock_temp_format,0,CLOCK_TEMP_FORMAT_CNT-1)) return false;

  if (!Is_In_Range(settings.rgb_method,0,RGB_METHOD_CNT-1)) return false;
  if (!Is_In_Range(settings.rgb_solid.red,0,4095)) return false;
  if (!Is_In_Range(settings.rgb_solid.green,0,4095)) return false;
  if (!Is_In_Range(settings.rgb_solid.blue,0,4095)) return false;
  // settings.rgb_grad: all possible values are valid.
  if (!Is_In_Range(settings.rgb_sleep_solid.red,0,4095)) return false;
  if (!Is_In_Range(settings.rgb_sleep_solid.green,0,4095)) return false;
  if (!Is_In_Range(settings.rgb_sleep_solid.blue,0,4095)) return false;

  if (!Is_Valid_Boolean(settings.main_wifi_connect)) return false;
  if (!Is_Valid_Boolean(settings.main_ntp_request)) return false;
  // settings.main_ntp_refresh_hour: all possible values are valid.
  if (!Is_In_Range(settings.main_ntp_refresh_min,0,59)) return false;
  if (!Is_In_Range(settings.main_ntp_refresh_sec,0,59)) return false;
  if (!Is_Valid_Boolean(settings.main_cmd_server_start)) return false;
  if (!Is_In_Range(settings.main_sleep_hour,0,23)) return false;
  if (!Is_In_Range(settings.main_sleep_min,0,59)) return false;
  if (!Is_In_Range(settings.main_sleep_sec,0,59)) return false;
  if (!Is_In_Range(settings.main_wakeup_hour,0,23)) return false;
  if (!Is_In_Range(settings.main_wakeup_min,0,59)) return false;
  if (!Is_In_Range(settings.main_wakeup_sec,0,59)) return false;
  if (!Is_Valid_Boolean(settings.main_sleep_enabled)) return false;

  if (settings.id_rev >= 0x0001)
  {
    // Check parameters of Revision 0001h

    if (!Is_In_Range(settings.clock_cur_adjust,0,3)) return false;
    if (!Is_In_Range(settings.clock_adjust_hour_2,0,23)) return false;
    if (!Is_In_Range(settings.clock_adjust_min_2,0,59)) return false;
    if (!Is_In_Range(settings.clock_adjust_sec_2,0,59)) return false;
    if (!Is_Valid_Boolean(settings.clock_adjust_sub_2)) return false;
    if (!Is_In_Range(settings.clock_adjust_hour_3,0,23)) return false;
    if (!Is_In_Range(settings.clock_adjust_min_3,0,59)) return false;
    if (!Is_In_Range(settings.clock_adjust_sec_3,0,59)) return false;
    if (!Is_Valid_Boolean(settings.clock_adjust_sub_3)) return false;
    if (!Is_In_Range(settings.clock_adjust_hour_4,0,23)) return false;
    if (!Is_In_Range(settings.clock_adjust_min_4,0,59)) return false;
    if (!Is_In_Range(settings.clock_adjust_sec_4,0,59)) return false;
    if (!Is_Valid_Boolean(settings.clock_adjust_sub_4)) return false;
  }
  else
  {
    // Set up parameters for Revision 0001h
    Settings_Set_Up_Rev_0001();
  }

  if (settings.id_rev >= 0x0002)
  {
    // Check parameters of Revision 0002h
    if (!Is_Valid_ASCII7Z(settings.wifi_hostname,sizeof(settings.wifi_hostname))) return false;
  }
  else
  {
    // Set up parameters for Revision 0002h
    Settings_Set_Up_Rev_0002();
  }


  if (settings.id_rev >= 0x0003)
  {
    // Check parameters of Revision 0003h
    if (!Is_Valid_Boolean(settings.clock_use_dst)) return false;
    if (!Is_Valid_Boolean(settings.bme_enabled)) return false;

  }
  else
  {
    // Set up parameters for Revision 0003h
    Settings_Set_Up_Rev_0003();
  }

  if (settings.id_rev < SETTINGS_REVISION) settings.id_rev = SETTINGS_REVISION;

  // Some settings come with a function
  Clock_Apply_Cur_Adjust();

  // If settings.rev is higer than 0002h, don't change its value to 0002h since this isn't
  // necessary.

  // The settings are valid
  return true;
}


void  Settings_Reset ()
{
#ifdef  SETTINGS_DEBUG
  Serial.println("Settings_Reset");
#endif

  // Zero all settings
  memset(&settings,0,sizeof(settings));

  // Set up non-zero settings
  memcpy(&settings.id_product,&settings_id_product,sizeof(settings.id_product));
  settings.id_rev = SETTINGS_REVISION;
  strcpy(settings.ntp_hostname,"pool.ntp.org");
  settings.clock_hour_lz = true;
  Settings_Set_Up_Rev_0001();
  Settings_Set_Up_Rev_0002();
  Settings_Set_Up_Rev_0003();
}


boolean  Settings_Clear ()
{
  int       ad;
  boolean   res;

#ifdef  SETTINGS_DEBUG
  Serial.println("Settings_Clear");
#endif

  if (!EEPROM.begin(sizeof(SETTINGS)))
  {
    // Always print this message
    Serial.println("Settings_Clear: can't access EEPROM!");
    return false;
  }

  for (ad = 0; ad < sizeof(SETTINGS); ad++)
  {
    EEPROM.write(ad,0xFF);
  }

  res = EEPROM.commit();
  if (!res)
  {
    // Always print this message
    Serial.println("Settings_Clear: can't commit data to EEPROM!");
  }

  EEPROM.end();

  return res;
}


boolean  Settings_Write ()
{
  int       ad;
  uint8_t  *p;
  boolean   res;

#ifdef  SETTINGS_DEBUG
  Serial.println("Settings_Write");
#endif

  if (!EEPROM.begin(sizeof(SETTINGS)))
  {
    // Always print this message
    Serial.println("Settings_Write: can't access EEPROM!");
    return false;
  }

  for (ad = 0, p = (uint8_t*)&settings; ad < sizeof(SETTINGS); ad++, p++)
  {
    EEPROM.write(ad,*p);
  }

  res = EEPROM.commit();
  if (!res)
  {
    // Always print this message
    Serial.println("Settings_Write: can't commit data to EEPROM!");
  }

  EEPROM.end();

  return res;
}


boolean  Settings_Read_Main ()
{
  int       ad;
  uint8_t  *p;

#ifdef  SETTINGS_DEBUG
  Serial.println("Settings_Read_Main");
#endif

  if (!EEPROM.begin(sizeof(SETTINGS)))
  {
    // Always print this message
    Serial.println("Settings_Read: can't access EEPROM!");
    return false;
  }

  for (ad = 0, p = (uint8_t*)&settings; ad < sizeof(SETTINGS); ad++, p++)
  {
    (*p) = EEPROM.read(ad);
  }

  EEPROM.end();

  return true;
}


void  Settings_Read ()
{
#ifdef  SETTINGS_DEBUG
  Serial.println("Settings_Read");
#endif

  if (Settings_Read_Main())
  {
    if (!Settings_Check_Validity())
    {
      // Always print this message
      Serial.println("Settings_Read: settings are undefined or invalid");

      Settings_Reset();
    }
  }
  else
  {
    Settings_Reset();
  }
}


void  Settings_Init ()
{
#ifdef  SETTINGS_DEBUG
  Serial.println("Settings_Init");
  Serial.printf("SETTINGS: %u B\n",sizeof(SETTINGS));
#endif

  Settings_Read();
}
