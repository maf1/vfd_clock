/*
  Command processor

  All commands:
*/

const char usage[] =
"  help|usage|?                                  Show this help\n"
"\n"
"  close|quit|q                                  Close the command interface (network connection only)\n"
"\n"
"  echocmd|ec YN                                 Enable or disable echoing of commands on the command interface\n"
"\n"
"  sys                                           System\n"
"    reset                                       System reset\n"
"    status|s                                    Show system status\n"
"\n"
"  wifi|w                                        Wifi\n"
"    start|s                                     Start WiFi\n"
"    stop|p                                      Stop WiFi\n"
"    dump|d                                      Dump information\n"
"    set                                         Settings\n"
"      ssid \"...\"                                Specify the SSID, network name (max. 32 characters)\n"
"      password \"...\"                            Specify the password (max. 63 characters)\n"
"      local IPV4AD                              Static IPv4 local address\n"
"      subnet IPV4AD                             Static IPv4 subnet mask\n"
"      gateway IPV4AD                            Static IPv4 gateway address\n"
"      dns NUMBER IPV4AD                         Static IPv4 DNS address (NUMBER=0..1)\n"
"      method|m                                  Method of assigning IPv4 address\n"
"        dynamic|dyn                             Use DHCP\n"
"        static|st                               Use static address\n"
"      hostname \"...\"                            Specify the hostname (max. 63 characters)\n"
"\n"
"  serial|ser                                    Serial interface\n"
"    set                                         Settings\n"
"      echocmd|ec YN                             Enable or disable echoing of commands on the command interface\n"
"\n"
"  cmdsrv|cs                                     Command server\n"
"    start|s                                     Start the command server\n"
"    stop|p                                      Stop the command server\n"
"    dump|d                                      Dump information\n"
"    close|c\n"
"      all                                       Close all client connections\n"
"      INDEX                                     Close specific client connection (INDEX=0..3)\n"
"    set                                         Settings\n"
"      echocmd|ec YN                             Enable or disable echoing of commands on the command interface\n"
"\n"
"  ntp                                           NTP\n"
"    start|s                                     Start NTP request\n"
"    set                                         Settings\n"
"      method|m                                  Method of determining IPv4 address of NTP server\n"
"        hostname|h                              Use hostname for looking up IPv4 address\n"
"        addr|a                                  Use IPv4 address\n"
"      hostname \"...\"                            Hostname of NTP server\n"
"      addr IPV4AD                               IPv4 address of NTP server\n"
"\n"
"  owtemp                                        1-Wire temperature sensor\n"
"    set                                         Settings\n"
"      enabled|ena YN                            Enable or disable temperature sensor\n"
"    read|r                                      Read current tempertaure\n"
"\n"
"  lm75|se95d                                    I2C temperature sensor\n"
"    set                                         Settings\n"
"      enabled|ena YN                            Enable or disable temperature sensor\n"
"      adsel N                                   Specify A[2..0] pin configuration of the chip (N=0..7)\n"
"    read|r                                      Read current tempertaure\n"
"\n"
"  bme280                                        I2C temperature, pressure (,humidity) sensor\n"
"    set                                         Settings\n"
"      enabled|ena YN                            Enable or disable sensor\n"
"    read|r                                      Read current tempertaure, pressure (and humidity)\n"
"    history|h                                   Read pressure history\n"
"    clear|c                                     Clear pressure history\n"
"\n"
"  clock|c                                       Clock\n"
"    write|w YEAR MONTH DAY HOUR MINUTE SECOND   Write date and time\n"
"    read|r                                      Read current date and time\n"
"    suspend                                     Suspend the clock\n"
"    resume                                      Resume the clock\n"
"    cycle|c                                     Trigger one cycling through all available auxiliary values (date, temperature, humidity, pressure)\n"
"    set                                         Settings\n"
"      adjust [N] +|- HOUR MINUTE SECOND         Adjustment applied on time received from NTP server (multiple adjustments, N=1..4, default is 1)\n"
"      selectadjust|sa N                         Select adjustment (N=1..4)\n"
"      usedst YN                                 Use timezone info for adjsutment (inckl. DST)\n"
"      showdate|sd YN                            Show date y/n\n"
"      dateformat|df dmy|mdy|ymd                 Date format (dmy=day-month-year, mdy=month-day-year, ymd=year-month-day)\n"
"      datesepmode|dsm on|off                    Mode of separator tubes when showing date (on=always on, off=always off)\n"
"      hourformat|hf 12|24                       Hour format (12=12-hour format, 24=24-hour format)\n"
"      hourlz|hlz YN                             Show hour with leading zero y/n\n"
"      timesepmode|tsm blink|ampm|on|off         Mode of separator tubes when showing time (blink=blink ones per second, ampm=indicate AM/PM, on=always on, off=always off)\n"
"      showtemp|st YN                            Show temperature y/n\n"
"      tempformat|tf celsius|c|fahrenheit|f      Temperature format\n"
"\n"
"  display|d                                     Display driver\n"
"    suspend                                     Suspend the display driver\n"
"    resume                                      Resume the display driver\n"
"\n"
"  rgb                                           RGB driver\n"
"    solid|s                                     Select solid RGB colors\n"
"      id N                                      Select scheme from list (N=0..)\n"
"      raw RED GREEN BLUE                        Specify RGB values\n"
"    grad|g                                      Select animated gradient colors\n"
"      id N                                      Select scheme from list (N=0..)\n"
"    set                                         Settings\n"
"      method|m                                  Color method\n"
"        solid|s                                 Use solid RGB colors\n"
"        grad|g                                  Use animated gradient colors\n"
"      solid|s                                   Solid RGB colors\n"
"        id N                                    Select scheme from list (N=0..)\n"
"        raw RED GREEN BLUE                      Specify RGB values\n"
"        use                                     Copy active solid RGB colors to settings\n"
"        apply                                   Apply solid RGB colors in settings as active\n"
"      grad|g                                    Animated gradient colors\n"
"        id N                                    Select scheme from list (N=0..)\n"
"        use                                     Copy active gradient colors to settings\n"
"        apply                                   Apply gradient colors in settings as active\n"
"      sleep                                     Solid RGB colors shown during sleep mode\n"
"        id N                                    Select scheme from list (N=0..)\n"
"        raw RED GREEN BLUE                      Specify RGB values\n"
"        use                                     Copy active solid RGB colors to settings\n"
"        apply                                   Apply solid RGB colors in settings as active during sleep mode\n"
"\n"
"  main|m                                        Main function\n"
"    ntprefresh|nrf                              Perform an NTP request and refresh clock date and time\n"
"    set                                         Settings\n"
"      wificonnect|wc YN                         Automatically connect to wireless network y/n\n"
"      ntprequest|nrq YN                         Periodically perform an NTP request y/n\n"
"      ntprefresh|nrf HOURS MINUTE SECOND        NTP refresh period (HOURS=0..255)\n"
"      cmdsrvstart|css YN                        Automatically start command server y/n\n"
"      sleeptime|st HOUR MINUTE SECOND           Time to go to sleep\n"
"      wakeuptime|wut HOUR MINUTE SECOND         Time to wake up\n"
"      sleepmode|sm YN                           Enable or disable sleep mode\n"
"\n"
"  settings|s                                    Settings\n"
"    write|w                                     Write settings from RAM to EEPROM\n"
"    read|r                                      Read settings from EEPROM to RAM, verify, apply default settings if invalid\n"
"    reset                                       Reset settings in RAM\n"
"    clear                                       Clear settings in EEPROM\n"
"    dump|d                                      Dump information\n"
"\n"
"Recurring parameters:\n"
"  IPV4AD  IPv4 address, for example: 192.168.1.1\n"
"  YEAR    1970..2225\n"
"  MONTH   1..12\n"
"  DAY     1..31\n"
"  HOUR    0..23\n"
"  MINUTE  0..59\n"
"  SECOND  0..59\n"
"  RED     0..4095\n"
"  GREEN   0..4095\n"
"  BLUE    0..4095\n"
"  YN      yes|y,no|n";


void  Sys_Status_Dump (CMD_PROC *p)
{
  char   line[80];
  int           cnt, days, hours, mins;
  double        up;
  
  p->emit_str_fn(p,"System:",0);

  up    = Clock_Get_Uptime();
  days  = up / (24*60*60);
  up   -= days*(24*60*60);
  hours = up / (60*60);
  up   -= hours*(60*60);
  mins  = up / 60;
  up   -= mins*60;

  cnt = sprintf(line,"* Uptime:           %d days %02d:%02d:%04.1f",days,hours,mins,up);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* FW version:       %s",fw_version);
  p->emit_str_fn(p,line,cnt);
  
  cnt = sprintf(line,"* FW compiled:      %s",compile_date);
  p->emit_str_fn(p,line,cnt);

  cnt = sprintf(line,"* Settings rev:     0x%04x",settings.id_rev);
  p->emit_str_fn(p,line,cnt);
}


boolean  Parse_Number_In_Range (TOK *t, uint32_t min, uint32_t max, uint32_t *res)
{
  // Expect number token
  if (!Tok_Fetch(t)) return false;
  if (t->id != TOK_ID_NUMBER) return false;
  if (t->val < min) return false;
  if (t->val > max) return false;

  (*res) = t->val;
  return true;
}


boolean  Parse_RGB_Solid (TOK *t, RGB_SOLID *solid)
{
  uint32_t  u;

  // Parse pulse width value for red channel (0..4095)
  if (!Parse_Number_In_Range(t,0,4095,&u)) return false;
  solid->red = (uint16_t)u;

  // Parse pulse width value for green channel (0..4095)
  if (!Parse_Number_In_Range(t,0,4095,&u)) return false;
  solid->green = (uint16_t)u;

  // Parse pulse width value for blue channel (0..4095)
  if (!Parse_Number_In_Range(t,0,4095,&u)) return false;
  solid->blue = (uint16_t)u;

  return true;
}


boolean  Parse_HMS (TOK *t, byte *res_hour, byte *res_min, byte *res_sec)
{
  uint32_t  u;

  // Parse hour, expect value 0..23
  if (!Parse_Number_In_Range(t,0,23,&u)) return false;
  (*res_hour) = (byte)u;

  // Parse minute, expect value 0..59
  if (!Parse_Number_In_Range(t,0,59,&u)) return false;
  (*res_min) = (byte)u;

  // Parse second, expect value 0..59
  if (!Parse_Number_In_Range(t,0,59,&u)) return false;
  (*res_sec) = (byte)u;

  return true;
}


boolean  Parse_Char (TOK *t, uint8_t c)
{
  // Expect character token
  if (!Tok_Fetch(t)) return false;
  if (t->id != TOK_ID_CHAR) return false;
  if (t->buf[0] != c) return false;
  return true;
}


boolean  Parse_YN (TOK *t, boolean *res)
{
  // Fetch label token
  if (!Tok_Fetch(t)) return false;
  if (t->id != TOK_ID_LABEL) return false;

  if ((strcasecmp(t->buf,"yes") == 0) ||(strcasecmp(t->buf,"y") == 0)) (*res) = true; else
  if ((strcasecmp(t->buf,"no") == 0) || (strcasecmp(t->buf,"n") == 0)) (*res) = false; else
    return false;

  return true;
}


boolean  Parse_IP (TOK *t, uint32_t *addr)
{
  uint32_t  res_addr;
  uint32_t  u;

  // Expect value 0..255
  if (!Parse_Number_In_Range(t,0,255,&res_addr)) return false;

  // Expect '.' character
  if (!Parse_Char(t,'.')) return false;

  // Expect value 0..255
  if (!Parse_Number_In_Range(t,0,255,&u)) return false;
  res_addr |= (u << 8);

  // Expect '.' character
  if (!Parse_Char(t,'.')) return false;

  // Expect value 0..255
  if (!Parse_Number_In_Range(t,0,255,&u)) return false;
  res_addr |= (u << 16);

  // Expect '.' character
  if (!Parse_Char(t,'.')) return false;

  // Expect value 0..255
  if (!Parse_Number_In_Range(t,0,255,&u)) return false;
  res_addr |= (u << 24);

  (*addr) = res_addr;
  return true;
}


// Tokenizer
TOK   tok;


typedef union  _CMD_PROC_TEMP_DATA
{
  char  ntp_hostname[NTP_HOSTNAME_MAX_LEN+1];
  char  wifi_ssid[WIFI_SSID_MAX_LEN+1];
  char  wifi_password[WIFI_PASSWORD_MAX_LEN+1];
  char  wifi_hostname[WIFI_HOSTNAME_MAX_LEN+1];
}
  CMD_PROC_TEMP_DATA;


boolean  Cmd_Proc_Parse_Cmd (CMD_PROC *p, TOK *t)
{
  static  CMD_PROC_TEMP_DATA  temp;

  if ((strcasecmp(t->buf,"close") == 0) || (strcasecmp(t->buf,"quit") == 0) || (strcasecmp(t->buf,"q") == 0))
  {
    // Expect EOL
    if (!Tok_Fetch_EOL(t)) return false;

    // Request closing of the command client
    p->close_fn(p);
  }
  else
  if ((strcasecmp(t->buf,"help") == 0) || (strcasecmp(t->buf,"usage") == 0) || (strcasecmp(t->buf,"?") == 0))
  {
    // Expect EOL
    if (!Tok_Fetch_EOL(t)) return false;

    // Print usage
    p->emit_str_fn(p,"Commands:\n",0);
    p->emit_str_fn(p,(char*)usage,0);
  }
  else
  if ((strcasecmp(t->buf,"echocmd") == 0) || (strcasecmp(t->buf,"ec") == 0))
  {
    boolean   yn;

    // Parse yes/no
    if (!Parse_YN(t,&yn)) return false;

    // Expect EOL
    if (!Tok_Fetch_EOL(t)) return false;

    // Set flag in command processor
    p->echo_commands = yn;
  }
  else
  if (strcasecmp(t->buf,"sys") == 0)
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if (strcasecmp(t->buf,"reset") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      esp_restart();
    }
    else if ((strcasecmp(t->buf,"status") == 0) || (strcasecmp(t->buf,"s") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Sys_Status_Dump(p);
    }
    else
    {
      // Unknown system command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"wifi") == 0) || (strcasecmp(t->buf,"w") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"start") == 0) || (strcasecmp(t->buf,"s") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      WIFI_Connect();
    }
    else
    if ((strcasecmp(t->buf,"stop") == 0) || (strcasecmp(t->buf,"p") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      WIFI_Disconnect();
    }
    else
    if ((strcasecmp(t->buf,"dump") == 0) || (strcasecmp(t->buf,"d") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      WIFI_Dump(p);
    }
    else
    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if (strcasecmp(t->buf,"ssid") == 0)
      {
        // Expect string token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_STRING) return false;
        if (t->buf_si > WIFI_SSID_MAX_LEN) return false;

        // Copy now, since fetching the next token will overwrite the string
        strcpy(temp.wifi_ssid,t->buf);

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        strcpy(settings.wifi_ssid,temp.wifi_ssid);
      }
      else
      if (strcasecmp(t->buf,"password") == 0)
      {
        // Expect string token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_STRING) return false;
        if (t->buf_si > WIFI_PASSWORD_MAX_LEN) return false;

        // Copy now, since fetching the next token will overwrite the string
        strcpy(temp.wifi_password,t->buf);

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        strcpy(settings.wifi_password,temp.wifi_password);
      }
      else
      if (strcasecmp(t->buf,"local") == 0)
      {
        uint32_t  addr;

        // Parse IP address
        if (!Parse_IP(t,&addr)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.wifi_local_ip = addr;
      }
      else
      if (strcasecmp(t->buf,"subnet") == 0)
      {
        uint32_t  addr;

        // Parse IP address
        if (!Parse_IP(t,&addr)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.wifi_subnet = addr;
      }
      else
      if (strcasecmp(t->buf,"gateway") == 0)
      {
        uint32_t  addr;

        // Parse IP address
        if (!Parse_IP(t,&addr)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.wifi_gateway = addr;
      }
      else
      if (strcasecmp(t->buf,"dns") == 0)
      {
        uint32_t  index;
        uint32_t  addr;

        // Parse DNS server number (1..max)
        if (!Parse_Number_In_Range (t,1,SETTINGS_WIFI_DNS_CNT,&index)) return false;
        index--;

        // Parse IP address
        if (!Parse_IP(t,&addr)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.wifi_dns_list[index] = addr;
      }
      else
      if ((strcasecmp(t->buf,"method") == 0) || (strcasecmp(t->buf,"m") == 0))
      {
        // Fetch label token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;

        if ((strcasecmp(t->buf,"dynamic") == 0) || (strcasecmp(t->buf,"dyn") == 0))
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;
  
          settings.wifi_use_static = false;
        }
        else
        if ((strcasecmp(t->buf,"static") == 0) || (strcasecmp(t->buf,"st") == 0))
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;
  
          settings.wifi_use_static = true;
        }
        else
        {
          // Unknown wifi set method command
          return false;
        }
      }
      else
      if (strcasecmp(t->buf,"hostname") == 0)
      {
        // Expect string token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_STRING) return false;
        if (t->buf_si > WIFI_HOSTNAME_MAX_LEN) return false;

        // Copy now, since fetching the next token will overwrite the string
        strcpy(temp.wifi_hostname,t->buf);

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        strcpy(settings.wifi_hostname,temp.wifi_hostname);
      }
      else
      {
        // Unknown wifi set command
        return false;
      }
    }
    else
    {
      // Unknown wifi command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"serial") == 0) || (strcasecmp(t->buf,"ser") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"echocmd") == 0) || (strcasecmp(t->buf,"ec") == 0))
      {
          boolean   yn;

          // Parse yes/no
          if (!Parse_YN(t,&yn)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.serial_echo_commands = yn;
      }
      else
      {
        // Unknown serial set command
        return false;
      }
    }
    else
    {
      // Unknown serial command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"cmdsrv") == 0) || (strcasecmp(t->buf,"cs") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"start") == 0) || (strcasecmp(t->buf,"s") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Cmd_Server_Start();
    }
    else
    if ((strcasecmp(t->buf,"stop") == 0) || (strcasecmp(t->buf,"p") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Cmd_Server_Stop();
    }
    else
    if ((strcasecmp(t->buf,"dump") == 0) || (strcasecmp(t->buf,"d") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Cmd_Server_Dump(p);
    }
    else
    if ((strcasecmp(t->buf,"close") == 0) || (strcasecmp(t->buf,"c") == 0))
    {
      // Fetch token
      if (!Tok_Fetch(t)) return false;
      if (t->id == TOK_ID_LABEL)
      {
        if (strcasecmp(t->buf,"all") == 0)
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;
    
          Cmd_Server_Client_Close_All();
        }
        else
        {
          // Unknown command server close command
          return false;
        }
      }
      else
      if (t->id == TOK_ID_NUMBER)
      {
        uint32_t  client_index;

        client_index = t->val;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;
    
        Cmd_Server_Client_Close(client_index);
      }
      else
      {
        // Unexpected token
        return false;
      }
    }
    else
    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"echocmd") == 0) || (strcasecmp(t->buf,"ec") == 0))
      {
          boolean   yn;

          // Parse yes/no
          if (!Parse_YN(t,&yn)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.cs_echo_commands = yn;
      }
      else
      {
        // Unknown serial set command
        return false;
      }
    }
    else
    {
      // Unknown command server command
      return false;
    }
  }
  else
  if (strcasecmp(t->buf,"ntp") == 0)
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"start") == 0) || (strcasecmp(t->buf,"s") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      NTP_Start(Clock_NTP_Report_Time);
    }
    else
    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"method") == 0) || (strcasecmp(t->buf,"m") == 0))
      {
        // Fetch label token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;

        if ((strcasecmp(t->buf,"hostname") == 0) || (strcasecmp(t->buf,"h") == 0))
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.ntp_use_addr = false;
        }
        else
        if ((strcasecmp(t->buf,"addr") == 0) || (strcasecmp(t->buf,"a") == 0))
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.ntp_use_addr = true;
        }
        else
        {
          // Unknown ntp set method command
          return false;
        }
      }
      else
      if (strcasecmp(t->buf,"hostname") == 0)
      {
        // Expect string token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_STRING) return false;
        if (t->buf_si > NTP_HOSTNAME_MAX_LEN) return false;

        // Copy now, since fetching the next token will overwrite the string
        strcpy(temp.ntp_hostname,t->buf);

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        strcpy(settings.ntp_hostname,temp.ntp_hostname);
      }
      else
      if (strcasecmp(t->buf,"addr") == 0)
      {
        uint32_t  addr;

        // Parse IP address
        if (!Parse_IP(t,&addr)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.ntp_addr = addr;
      }
      else
      {
        // Unknown ntp set command
        return false;
      }
    }
    else
    {
      // Unknown ntp command
      return false;
    }
  }
  else
  if (strcasecmp(t->buf,"owtemp") == 0)
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"enabled") == 0) || (strcasecmp(t->buf,"ena") == 0))
      {
          boolean   yn;

          // Parse yes/no
          if (!Parse_YN(t,&yn)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.owtemp_enabled = yn;
      }
      else
      {
        // Unknown owtemp set command
        return false;
      }
    }
    else
    if ((strcasecmp(t->buf,"read") == 0) || (strcasecmp(t->buf,"r") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;
      TS_Read(p);
    }
    else
    {
      // Unknown owtemp command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"lm75") == 0) || (strcasecmp(t->buf,"se95d") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"enabled") == 0) || (strcasecmp(t->buf,"ena") == 0))
      {
          boolean   yn;

          // Parse yes/no
          if (!Parse_YN(t,&yn)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.lm75_enabled = yn;
      }
      else
      if (strcasecmp(t->buf,"adsel") == 0)
      {
          uint32_t  u;
          byte      adsel;

          // Parse address selection (0..7)
          if (!Parse_Number_In_Range(t,0,7,&u)) return false;
          adsel = (byte)u;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.lm75_adsel = adsel;
      }
      else
      {
        // Unknown lm75 set command
        return false;
      }
    }
    else
    if ((strcasecmp(t->buf,"read") == 0) || (strcasecmp(t->buf,"r") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;
      TS_Read(p);
    }
    else
    {
      // Unknown lm75 command
      return false;
    }
  }
  else
  if (strcasecmp(t->buf,"bme280") == 0)
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"enabled") == 0) || (strcasecmp(t->buf,"ena") == 0))
      {
          boolean   yn;

          // Parse yes/no
          if (!Parse_YN(t,&yn)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.bme_enabled = yn;
      }
      else
      {
        // Unknown bme set command
        return false;
      }
    }
    else
    if ((strcasecmp(t->buf,"read") == 0) || (strcasecmp(t->buf,"r") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;
      TS_Read(p);
    }
    else
    if ((strcasecmp(t->buf,"history") == 0) || (strcasecmp(t->buf,"h") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;
      TS_Read_History(p);
    }
    else
    if ((strcasecmp(t->buf,"clear") == 0) || (strcasecmp(t->buf,"c") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;
      TS_Clear_History();
    }
    else
    {
      // Unknown bme command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"clock") == 0) || (strcasecmp(t->buf,"c") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"write") == 0) || (strcasecmp(t->buf,"w") == 0))
    {
      TimeElements    tm;
      uint32_t        u;

      // Parse year (1970..2225)
      if (!Parse_Number_In_Range(t,1970,2225,&u)) return false;
      u -= 1970;
      tm.Year = (uint8_t)u;

      // Parse month (1..12)
      if (!Parse_Number_In_Range(t,1,12,&u)) return false;
      tm.Month = (uint8_t)u;

      // Parse day (1..31)
      if (!Parse_Number_In_Range(t,1,31,&u)) return false;
      tm.Day = (uint8_t)u;

      // Parse hour (0..23)
      if (!Parse_Number_In_Range(t,0,23,&u)) return false;
      tm.Hour = (uint8_t)u;

      // Parse minute (0..59)
      if (!Parse_Number_In_Range(t,0,59,&u)) return false;
      tm.Minute = (uint8_t)u;

      // Parse second (0..59)
      if (!Parse_Number_In_Range(t,0,59,&u)) return false;
      tm.Second = (uint8_t)u;

      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Clock_Write(makeTime(tm));
    }
    else
    if ((strcasecmp(t->buf,"read") == 0) || (strcasecmp(t->buf,"r") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Clock_Read(p);
    }
    else
    if (strcasecmp(t->buf,"suspend") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Clock_Suspend();
    }
    else
    if (strcasecmp(t->buf,"resume") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Clock_Resume();
    }
    else
    if ((strcasecmp(t->buf,"cycle") == 0) || (strcasecmp(t->buf,"c") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      // Need Display
      if( Display_Is_Suspended() ) return false;

      Clock_Start_Cycling();
    }
    else
    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if (strcasecmp(t->buf,"adjust") == 0)
      {
        boolean   adjust_sub;
        byte      adjust_hour;
        byte      adjust_min;
        byte      adjust_sec;
        byte      adjust_index;

        // Parse token
        if (!Tok_Fetch(t)) return false;
        if (t->id == TOK_ID_NUMBER)
        {
          // Expect value 1..4
          if (!((t->val >= 1) && (t->val <= 4))) return false;
          adjust_index = (byte)t->val - 1;

          // Parse token
          if (!Tok_Fetch(t)) return false;
        }
        else
        {
          // Default index
          adjust_index = 0;
        }

        // Expect '+' or '-' character
        if (t->id != TOK_ID_CHAR) return false;
        if (t->buf[0] == '+') adjust_sub = false; else
        if (t->buf[0] == '-') adjust_sub = true; else
          return false;

        // Parse hour, minute, second
        if (!Parse_HMS(t,&adjust_hour,&adjust_min,&adjust_sec)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        switch (adjust_index)
        {
          case 0:
          {
            settings.clock_adjust_hour = adjust_hour;
            settings.clock_adjust_min  = adjust_min;
            settings.clock_adjust_sec  = adjust_sec;
            settings.clock_adjust_sub  = adjust_sub;
            break;
          }

          case 1:
          {
            settings.clock_adjust_hour_2 = adjust_hour;
            settings.clock_adjust_min_2  = adjust_min;
            settings.clock_adjust_sec_2  = adjust_sec;
            settings.clock_adjust_sub_2  = adjust_sub;
            break;
          }

          case 2:
          {
            settings.clock_adjust_hour_3 = adjust_hour;
            settings.clock_adjust_min_3  = adjust_min;
            settings.clock_adjust_sec_3  = adjust_sec;
            settings.clock_adjust_sub_3  = adjust_sub;
            break;
          }

          case 3:
          {
            settings.clock_adjust_hour_4 = adjust_hour;
            settings.clock_adjust_min_4  = adjust_min;
            settings.clock_adjust_sec_4  = adjust_sec;
            settings.clock_adjust_sub_4  = adjust_sub;
            break;
          }
        }
      }
      else
      if ((strcasecmp(t->buf,"selectadjust") == 0) || (strcasecmp(t->buf,"sa") == 0))
      {
        uint32_t  u;
        byte      cur_adjust;
      
        // Parse identifier
        if (!Parse_Number_In_Range(t,1,SETTINGS_CLOCK_ADJUST_CNT,&u)) return false;
        cur_adjust = (byte)u - 1;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        Clock_Set_Cur_Adjust(cur_adjust);
      }
      else
      if (strcasecmp(t->buf,"usedst") == 0)
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_use_dst = yn;
      }
      else
      if ((strcasecmp(t->buf,"showdate") == 0) || (strcasecmp(t->buf,"sd") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_show_date = yn;
      }
      else
      if ((strcasecmp(t->buf,"dateformat") == 0) || (strcasecmp(t->buf,"df") == 0))
      {
        byte  date_format;

        // Fetch label token, expect formatting identifier
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;
        if (strcasecmp(t->buf,"dmy") == 0) date_format = CLOCK_DATE_FORMAT_DMY; else
        if (strcasecmp(t->buf,"mdy") == 0) date_format = CLOCK_DATE_FORMAT_MDY; else
        if (strcasecmp(t->buf,"ymd") == 0) date_format = CLOCK_DATE_FORMAT_YMD; else
          return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_date_format = date_format;
      }
      else
      if ((strcasecmp(t->buf,"datesepmode") == 0) || (strcasecmp(t->buf,"dsm") == 0))
      {
        byte  date_sep_mode;

        // Fetch label token, expect mode identifier
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;
        if (strcasecmp(t->buf,"on") == 0)    date_sep_mode = CLOCK_DATE_SEP_MODE_ON; else
        if (strcasecmp(t->buf,"off") == 0)   date_sep_mode = CLOCK_DATE_SEP_MODE_OFF; else
          return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_date_sep_mode = date_sep_mode;
      }
      else
      if ((strcasecmp(t->buf,"hourformat") == 0) || (strcasecmp(t->buf,"hf") == 0))
      {
        byte  hour_format;

        // Fetch number token, expect 24 or 12
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_NUMBER) return false;
        if (t->val == 24) hour_format = CLOCK_HOUR_FORMAT_24; else
        if (t->val == 12) hour_format = CLOCK_HOUR_FORMAT_12; else
          return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_hour_format = hour_format;
      }
      else
      if ((strcasecmp(t->buf,"hourlz") == 0) || (strcasecmp(t->buf,"hlz") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_hour_lz = yn;
      }
      else
      if ((strcasecmp(t->buf,"timesepmode") == 0) || (strcasecmp(t->buf,"tsm") == 0))
      {
        byte  time_sep_mode;

        // Fetch label token, expect mode identifier
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;
        if (strcasecmp(t->buf,"blink") == 0) time_sep_mode = CLOCK_TIME_SEP_MODE_BLINK; else
        if (strcasecmp(t->buf,"ampm") == 0)  time_sep_mode = CLOCK_TIME_SEP_MODE_AMPM; else
        if (strcasecmp(t->buf,"on") == 0)    time_sep_mode = CLOCK_TIME_SEP_MODE_ON; else
        if (strcasecmp(t->buf,"off") == 0)   time_sep_mode = CLOCK_TIME_SEP_MODE_OFF; else
          return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_time_sep_mode = time_sep_mode;
      }
      else
      if ((strcasecmp(t->buf,"showtemp") == 0) || (strcasecmp(t->buf,"st") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_show_temp = yn;
      }
      else
      if ((strcasecmp(t->buf,"tempformat") == 0) || (strcasecmp(t->buf,"tf") == 0))
      {
        byte  temp_format;

        // Fetch label token, expect formatting identifier
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;
        if ((strcasecmp(t->buf,"celsius") == 0) || (strcasecmp(t->buf,"c") == 0))    temp_format = CLOCK_TEMP_FORMAT_CELSIUS; else
        if ((strcasecmp(t->buf,"fahrenheit") == 0) || (strcasecmp(t->buf,"f") == 0)) temp_format = CLOCK_TEMP_FORMAT_FAHRENHEIT; else
          return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.clock_temp_format = temp_format;
      }
      else
      {
        // Unknown clock set command
        return false;
      }
    }
    else
    {
      // Unknown clock command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"display") == 0) || (strcasecmp(t->buf,"d") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if (strcasecmp(t->buf,"suspend") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Display_Suspend();
    }
    else
    if (strcasecmp(t->buf,"resume") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Display_Resume();
    }
    else
    {
      // Unknown display command
      return false;
    }
  }
  else
  if (strcasecmp(t->buf,"rgb") == 0)
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"solid") == 0) || (strcasecmp(t->buf,"s") == 0))
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if (strcasecmp(t->buf,"id") == 0)
      {
        uint32_t  u;
        byte      id;
      
        // Parse identifier
        if (!Parse_Number_In_Range(t,0,RGB_SOLID_ID_CNT-1,&u)) return false;
        id = (byte)u;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        RGB_Select_Solid_Id(id);
      }
      else
      if (strcasecmp(t->buf,"raw") == 0)
      {
        uint32_t    u;
        RGB_SOLID   solid;

        // Parse RGB color values
        if (!Parse_RGB_Solid(t,&solid)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        RGB_Select_Solid(&solid);
      }
      else
      {
        // Unknown rgb solid command
        return false;
      }
    }
    else
    if ((strcasecmp(t->buf,"grad") == 0) || (strcasecmp(t->buf,"g") == 0))
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if (strcasecmp(t->buf,"id") == 0)
      {
        uint32_t  u;
        byte      id;
      
        // Parse identifier
        if (!Parse_Number_In_Range(t,0,RGB_GRAD_SET_ID_CNT-1,&u)) return false;
        id = (byte)u;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        RGB_Select_Grad_Set_Id(id);
      }
      else
      {
        // Unknown rgb grad command
        return false;
      }
    }
    else
    if (strcasecmp(t->buf,"suspend") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      RGB_Suspend();
    }
    else
    if (strcasecmp(t->buf,"resume") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      RGB_Resume();
    }
    else
    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"method") == 0) || (strcasecmp(t->buf,"m") == 0))
      {
        // Fetch label token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;

        if ((strcasecmp(t->buf,"solid") == 0) || (strcasecmp(t->buf,"s") == 0))
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;
  
          settings.rgb_method = RGB_METHOD_SOLID;
        }
        else
        if ((strcasecmp(t->buf,"grad") == 0) || (strcasecmp(t->buf,"g") == 0))
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;
  
          settings.rgb_method = RGB_METHOD_GRAD;
        }
        else
        {
          // Unknown rgb set method command
          return false;
        }
      }
      else
      if ((strcasecmp(t->buf,"solid") == 0) || (strcasecmp(t->buf,"s") == 0))
      {
        // Fetch label token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;

        if (strcasecmp(t->buf,"id") == 0)
        {
          uint32_t  u;
          byte      id;

          // Parse identifier
          if (!Parse_Number_In_Range(t,0,RGB_SOLID_ID_CNT-1,&u)) return false;
          id = (byte)u;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Copy solid color values to settings
          RGB_Solid_Id_To_Settings(id);
        }
        else
        if (strcasecmp(t->buf,"raw") == 0)
        {
          uint32_t    u;
          RGB_SOLID   solid;

          // Parse RGB color values
          if (!Parse_RGB_Solid(t,&solid)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.rgb_solid.red   = solid.red;
          settings.rgb_solid.green = solid.green;
          settings.rgb_solid.blue  = solid.blue;
        }
        else
        if (strcasecmp(t->buf,"use") == 0)
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Copy active solid color values to settings
          RGB_Solid_To_Settings();
        }
        else
        if (strcasecmp(t->buf,"apply") == 0)
        {
          RGB_SOLID   solid;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Apply solid color in settings as active solid colors
          solid.red   = settings.rgb_solid.red;
          solid.green = settings.rgb_solid.green;
          solid.blue  = settings.rgb_solid.blue;
          RGB_Select_Solid(&solid);
        }
        else
        {
          // Unknown rgb set solid command
          return false;
        }
      }
      else
      if ((strcasecmp(t->buf,"grad") == 0) || (strcasecmp(t->buf,"g") == 0))
      {
        // Fetch label token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;

        if (strcasecmp(t->buf,"id") == 0)
        {
          uint32_t  u;
          byte      id;

          // Parse identifier
          if (!Parse_Number_In_Range(t,0,RGB_GRAD_SET_ID_CNT-1,&u)) return false;
          id = (byte)u;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          RGB_Grad_Set_Id_To_Settings(id);
        }
        else
        if (strcasecmp(t->buf,"use") == 0)
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Copy gradient set values to settings
          RGB_Grad_Set_To_Settings();
        }
        else
        if (strcasecmp(t->buf,"apply") == 0)
        {
          RGB_SOLID   solid;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Apply gradient set in settings as active gradient set
          RGB_Select_Grad_Set(settings.rgb_grad);
        }
        else
        {
          // Unknown rgb set grad command
          return false;
        }
      }
      else
      if (strcasecmp(t->buf,"sleep") == 0)
      {
        // Fetch label token
        if (!Tok_Fetch(t)) return false;
        if (t->id != TOK_ID_LABEL) return false;

        if (strcasecmp(t->buf,"id") == 0)
        {
          uint32_t  u;
          byte      id;

          // Parse identifier
          if (!Parse_Number_In_Range(t,0,RGB_SOLID_ID_CNT-1,&u)) return false;
          id = (byte)u;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Copy solid color values to settings
          RGB_Solid_Id_To_Settings_Sleep(id);
        }
        else
        if (strcasecmp(t->buf,"raw") == 0)
        {
          uint32_t    u;
          RGB_SOLID   solid;

          // Parse RGB color values
          if (!Parse_RGB_Solid(t,&solid)) return false;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          settings.rgb_sleep_solid.red   = solid.red;
          settings.rgb_sleep_solid.green = solid.green;
          settings.rgb_sleep_solid.blue  = solid.blue;
        }
        else
        if (strcasecmp(t->buf,"use") == 0)
        {
          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Copy active solid color values to settings
          RGB_Solid_To_Settings_Sleep();
        }
        else
        if (strcasecmp(t->buf,"apply") == 0)
        {
          RGB_SOLID   solid;

          // Expect EOL
          if (!Tok_Fetch_EOL(t)) return false;

          // Apply sleep solid color in settings as active solid colors
          solid.red   = settings.rgb_sleep_solid.red;
          solid.green = settings.rgb_sleep_solid.green;
          solid.blue  = settings.rgb_sleep_solid.blue;
          RGB_Select_Solid(&solid);
        }
        else
        {
          // Unknown rgb set sleep command
          return false;
        }
      }
      else
      {
        // Unknown rgb set command
        return false;
      }
    }
    else
    {
      // Unknown rgb command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"main") == 0) || (strcasecmp(t->buf,"m") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"ntprefresh") == 0) || (strcasecmp(t->buf,"nrf") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Main_NTP_Refresh();
    }
    else
    if (strcasecmp(t->buf,"set") == 0)
    {
      // Fetch label token
      if (!Tok_Fetch(t)) return false;
      if (t->id != TOK_ID_LABEL) return false;

      if ((strcasecmp(t->buf,"wificonnect") == 0) || (strcasecmp(t->buf,"wc") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_wifi_connect = yn;
      }
      else
      if ((strcasecmp(t->buf,"ntprequest") == 0) || (strcasecmp(t->buf,"nrq") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_ntp_request = yn;
      }
      else
      if ((strcasecmp(t->buf,"ntprefresh") == 0) || (strcasecmp(t->buf,"nrf") == 0))
      {
        uint32_t  u;
        byte      ntp_refresh_hour;
        byte      ntp_refresh_min;
        byte      ntp_refresh_sec;

        // Parse hour, expect value 0..255
        if (!Parse_Number_In_Range(t,0,255,&u)) return false;
        ntp_refresh_hour = (byte)u;

        // Parse minute, expect value 0..59
        if (!Parse_Number_In_Range(t,0,59,&u)) return false;
        ntp_refresh_min = (byte)u;

        // Parse second, expect value 0..59
        if (!Parse_Number_In_Range(t,0,59,&u)) return false;
        ntp_refresh_sec = (byte)u;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_ntp_refresh_hour = ntp_refresh_hour;
        settings.main_ntp_refresh_min  = ntp_refresh_min;
        settings.main_ntp_refresh_sec  = ntp_refresh_sec;
      }
      else
      if ((strcasecmp(t->buf,"cmdsrvstart") == 0) || (strcasecmp(t->buf,"css") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_cmd_server_start = yn;
      }
      else
      if ((strcasecmp(t->buf,"sleeptime") == 0) || (strcasecmp(t->buf,"st") == 0))
      {
        byte  sleep_hour;
        byte  sleep_min;
        byte  sleep_sec;

        // Parse hour, minute, second
        if (!Parse_HMS(t,&sleep_hour,&sleep_min,&sleep_sec)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_sleep_hour = sleep_hour;
        settings.main_sleep_min  = sleep_min;
        settings.main_sleep_sec  = sleep_sec;
      }
      else
      if ((strcasecmp(t->buf,"wakeuptime") == 0) || (strcasecmp(t->buf,"wut") == 0))
      {
        byte  wakeup_hour;
        byte  wakeup_min;
        byte  wakeup_sec;

        // Parse hour, minute, second
        if (!Parse_HMS(t,&wakeup_hour,&wakeup_min,&wakeup_sec)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_wakeup_hour = wakeup_hour;
        settings.main_wakeup_min  = wakeup_min;
        settings.main_wakeup_sec  = wakeup_sec;
      }
      else
      if ((strcasecmp(t->buf,"sleepmode") == 0) || (strcasecmp(t->buf,"sm") == 0))
      {
        boolean   yn;

        // Parse yes/no
        if (!Parse_YN(t,&yn)) return false;

        // Expect EOL
        if (!Tok_Fetch_EOL(t)) return false;

        settings.main_sleep_enabled = yn;
      }
      else
      {
        // Unknown main set command
        return false;
      }
    }
    else
    {
      // Unknown main command
      return false;
    }
  }
  else
  if ((strcasecmp(t->buf,"settings") == 0) || (strcasecmp(t->buf,"s") == 0))
  {
    // Fetch label token
    if (!Tok_Fetch(t)) return false;
    if (t->id != TOK_ID_LABEL) return false;

    if ((strcasecmp(t->buf,"write") == 0) || (strcasecmp(t->buf,"w") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Settings_Write();
    }
    else
    if ((strcasecmp(t->buf,"read") == 0) || (strcasecmp(t->buf,"r") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Settings_Read();
    }
    else
    if (strcasecmp(t->buf,"reset") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Settings_Reset();
    }
    else
    if (strcasecmp(t->buf,"clear") == 0)
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Settings_Clear();
    }
    else
    if ((strcasecmp(t->buf,"dump") == 0) || (strcasecmp(t->buf,"d") == 0))
    {
      // Expect EOL
      if (!Tok_Fetch_EOL(t)) return false;

      Settings_Dump(p);
    }
    else
    {
      // Unknown settings command
      return false;
    }
  }
  else
  {
    // Unknown command
    return false;
  }

  // Success
  return true;
}


boolean  Cmd_Proc_Parse (CMD_PROC *p)
{
  TOK  *t;

  t = &tok;

  // Arm the tokenizer
  Tok_Init(t,p->rcv_buf);

  // Fetch token
  if (!Tok_Fetch(t)) return false;

  if (t->id == TOK_ID_LABEL)
  {
    boolean   res;

    res = Cmd_Proc_Parse_Cmd(p,t);
    if (res)
      p->emit_str_fn(p,"OK",0);
    else
      p->emit_str_fn(p,"Error in command!",0);

    return res;
  }
  if (t->id == TOK_ID_EOL)
  {
      // No command
  }
  else
  {
    // Unexpected token
    return false;
  }

  return true;
}


void  Cmd_Proc_Reset_Rcv_Buf (CMD_PROC *p)
{
  p->rcv_si    = 0;
  p->rcv_error = false;
  p->rcv_fi    = 0;
}


void  Cmd_Proc_Reset (CMD_PROC *p)
{
  Cmd_Proc_Reset_Rcv_Buf(p);
}


void  Cmd_Proc_Feed (CMD_PROC *p, char *s, uint32_t cnt)
{
  char  c;

  while (cnt > 0)
  {
    cnt--;
    c = (*s);
    s++;

    if ((c == 13) || (c == 10))
    {
      if (p->rcv_error == false)
      {
        // Store terminating zero
        p->rcv_buf[p->rcv_si] = 0;

        if (p->echo_commands) p->emit_str_fn(p,p->rcv_buf,p->rcv_si);
  
        Cmd_Proc_Parse(p);
      }

      // Reset the receive buffer
      Cmd_Proc_Reset_Rcv_Buf(p);
    }
    else
    if ((c < 32) || (c > 126))
    {
      // Invalid character received
      p->rcv_error = true;
    }
    else
    if (p->rcv_si == CMD_PROC_RCV_BUF_LEN)
    {
      // Buffer overflow
      p->rcv_error = true;
    }
    else
    {
      // Store character in receive buffer
      p->rcv_buf[p->rcv_si] = c;
      p->rcv_si++;
    }
  }
}
