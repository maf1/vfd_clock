
/*
  Main
*/


byte            main_wifi_exec_state  = 0;
unsigned long   main_wifi_delay_ms;


void  Main_WIFI_Exec_Set_State (byte s)
{
  byte  prev_s;

  prev_s = main_wifi_exec_state;

  main_wifi_exec_state = s;

#ifdef  MAIN_DEBUG
  //Serial.printf("main_wifi_exec_state %u->%u\n",prev_s,s);
#endif
}


void  Main_WIFI_Exec ()
{
  switch (main_wifi_exec_state)
  {
    // WIFI is supposed to be disconnected at this point.
    case 0:
    {
      if (WIFI_Is_Connected())
      {
        // WIFI was connected outside the scope of this task

#ifdef  MAIN_DEBUG
        Serial.println("Main_WIFI_Exec: wifi connection detected");
#endif

        Main_WIFI_Exec_Set_State(2);
        return;
      }

      if ((settings.main_wifi_connect) && (WIFI_Is_Disconnected()))
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_WIFI_Exec: start connecting to wifi");
#endif

        WIFI_Connect();

        Main_WIFI_Exec_Set_State(1);
        return;
      }

      break;
    }

    case 1:
    {
      if (WIFI_Is_Connected())
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_WIFI_Exec: wifi connection established");
#endif

        Main_WIFI_Exec_Set_State(2);
        return;
      }

      if (WIFI_Is_Disconnected())
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_WIFI_Exec: wifi connection failed");
#endif

        Main_WIFI_Exec_Set_State(3);
        return;
      }

      break;
    }

    case 2:
    {
      if (WIFI_Is_Disconnected())
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_WIFI_Exec: wifi disconnection occured");
#endif

        Main_WIFI_Exec_Set_State(3);
        return;
      }

      break;
    }

    case 3:
    {
#ifdef  MAIN_DEBUG
      Serial.println("Main_WIFI_Exec: delay initiated");
#endif

      main_wifi_delay_ms = millis();
      Main_WIFI_Exec_Set_State(4);
      return;
    }

    case 4:
    {
      if ((millis() - main_wifi_delay_ms) > 2000)
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_WIFI_Exec: delay completed");
#endif

        Main_WIFI_Exec_Set_State(0);
        return;
      }

      break;
    }
  }
}


void  Main_WIFI_Init ()
{
}


byte            main_ntp_exec_state     = 0;
boolean         main_ntp_force_request  = false;
unsigned long   main_ntp_delay_ms;
unsigned long   main_ntp_refresh_ms;


void  Main_NTP_Exec_Set_State (byte s)
{
  byte  prev_s;

  prev_s = main_ntp_exec_state;

  main_ntp_exec_state = s;

#ifdef  MAIN_DEBUG
  //Serial.printf("main_ntp_exec_state %u->%u\n",prev_s,s);
#endif
}


void  Main_NTP_Report_Time (time_t t)
{
#ifdef  MAIN_DEBUG
  Serial.println("Main_NTP_Report_Time");
#endif

  Clock_NTP_Report_Time(t);

  main_ntp_force_request = false;
}


void  Main_NTP_Refresh ()
{
  main_ntp_force_request = true;
}


void  Main_NTP_Check_Refresh ()
{
  time_t          secs;
  unsigned long   cur_ms;
  unsigned long   timeout_ms;

#ifdef  MAIN_DEBUG
  //Serial.println("Main_NTP_Check_Refresh");
#endif

  secs = Clock_HMS_To_Secs(settings.main_ntp_refresh_hour,settings.main_ntp_refresh_min,settings.main_ntp_refresh_sec);

  // Convert to milliseconds. Note: max. value is 921,599 sec = 921,599,000 ms
  timeout_ms = secs * 1000;

  cur_ms = millis();

#ifdef  MAIN_DEBUG
  //Serial.printf("* main_ntp_refresh_ms %u, cur_ms %u, timeout_ms %u\n",main_ntp_refresh_ms,cur_ms,timeout_ms);
#endif

  if ((cur_ms - main_ntp_refresh_ms) >= timeout_ms)
  {
    // Restart refresh timer
    main_ntp_refresh_ms = cur_ms;

    main_ntp_force_request = true;
  }
}


void  Main_NTP_Exec ()
{
  switch (main_ntp_exec_state)
  {
    case 0:
    {
      if (NTP_Is_Busy())
      {
        // NTP request was initiated outside the scope of this task.
        //
        // Note: WIFI isn't necessarily connected as the NTP request may be
        // initiated at any time.

#ifdef  MAIN_DEBUG
        Serial.println("Main_NTP_Exec: request initiation detected");
#endif

        Main_NTP_Exec_Set_State(1);
        return;
      }

      if ((settings.main_ntp_request) && (WIFI_Is_Connected()))
      {
        if ((!Clock_Is_Time_Set()) || (main_ntp_force_request))
        {
#ifdef  MAIN_DEBUG
          Serial.println("Main_NTP_Exec: starting NTP request");
#endif

          main_ntp_force_request = true;
          NTP_Start(Main_NTP_Report_Time);

          Main_NTP_Exec_Set_State(1);
          return;
        }
      }

      break;
    }

    // Wait for completion of NTP request. Whether the NTP request was initiated by
    // this task or externally, this code always applies.
    case 1:
    {
      if (!NTP_Is_Busy())
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_NTP_Exec: NTP request has completed");
#endif

        // Restart refresh timer
        main_ntp_refresh_ms = millis();

        Main_NTP_Exec_Set_State(2);
        return;
      }

      break;
    }

    case 2:
    {
#ifdef  MAIN_DEBUG
      Serial.println("Main_NTP_Exec: delay initiated");
#endif

      main_ntp_delay_ms = millis();
      Main_NTP_Exec_Set_State(3);
      return;
    }

    case 3:
    {
      if ((millis() - main_ntp_delay_ms) > 2000)
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_NTP_Exec: delay completed");
#endif

        Main_NTP_Exec_Set_State(0);
        return;
      }

      break;
    }
  }
}


void  Main_NTP_Init ()
{
  // Restart refresh timer
  main_ntp_refresh_ms = millis();
}


boolean main_cmd_server_start_failed  = false;


void  Main_Cmd_Server_Exec ()
{
  if (Cmd_Server_Has_Started())
  {
    if (main_cmd_server_start_failed)
    {
      // Clear the start failure flag
      main_cmd_server_start_failed = false;
    }
  }
  else
  if ((settings.main_cmd_server_start) && (!main_cmd_server_start_failed) && (!Cmd_Server_Has_Started()))
  {
    // Start the command server
    Cmd_Server_Start();

    // If the command server failed to start, mark the event so the code won't try
    // to start the server over and over again.
    if (!Cmd_Server_Has_Started()) main_cmd_server_start_failed = true;
  }
}


void  Main_Cmd_Server_Init ()
{
}


#define MAIN_CLOCK_RAISE_MS         60000
#define MAIN_CLOCK_HOLD_START_MS     3000
#define MAIN_CLOCK_HOLD_REPEAT_MS    1500
#define MAIN_DEBOUNCE_MS             100



boolean         main_clock_suspended    = false;    // Clock is suspended or resumed
boolean         main_clock_pin_raise    = false;    // Last sampled state of RAISE pin
boolean         main_clock_raised       = false;
boolean         main_clock_hold_repeat  = false;
boolean         main_clock_hold_applied = false;
unsigned long   main_clock_raise_ms;
unsigned long   main_clock_hold_ms;
TIMER           main_pin_timer;
boolean         main_suspended_press    = false;

// Check clock sleeping mode.
//
// This function is called:
// (a) Every second.
// (b) When the button is pressed (transition to pressed state, not currently pressed state).

void  Main_Clock_Check_Sleep_Mode (boolean time_set)
{
  time_t    sleep_secs;
  time_t    wakeup_secs;
  time_t    now_secs;
  boolean   can_sleep;

  if ((settings.main_sleep_enabled) && (time_set))
  {
    sleep_secs  = Clock_HMS_To_Secs(settings.main_sleep_hour,settings.main_sleep_min,settings.main_sleep_sec);
    wakeup_secs = Clock_HMS_To_Secs(settings.main_wakeup_hour,settings.main_wakeup_min,settings.main_wakeup_sec);
    now_secs    = Clock_HMS_To_Secs(clock_tm.Hour,clock_tm.Minute,clock_tm.Second);

    // sleep_secs <= wakeup_secs:
    //           awake            sleep           awake
    //   |<----------------->|<------------>|<------------->|
    //   |...................|..............|..............>| now_secs
    //   |                   |              |               |
    //   00:00:00       sleep_secs     wakeup_secs          23:59:59
    //
    // sleep_secs > wakeup_secs:
    //
    //           sleep            awake           sleep
    //   |<----------------->|<------------>|<------------->|
    //   |...................|..............|..............>| now_secs
    //   |                   |              |               |
    //   00:00:00       wakeup_secs    sleep_secs           23:59:59
    //
    // In case sleep_secs equals wakeup_secs, we want the clock to be awake. That's why
    // the next statement checks (sleep_secs <= wakeup_secs). If the code would check
    // (sleep_secs < wakeup_secs), the clock would always sleep.

    can_sleep = (sleep_secs <= wakeup_secs)
                  ? ((sleep_secs <= now_secs) && (now_secs < wakeup_secs))
                  : ((now_secs < wakeup_secs) || (sleep_secs <= now_secs));
  }
  else
  {
    can_sleep = false;
  }

  if (can_sleep)
  {
    // Start or restart the raise timer when the RAISE pin is low
    if (!main_clock_pin_raise)
    {
      main_clock_raised = true;

      // Start raise timer
      main_clock_raise_ms = millis();

      // Entered raised state, can't sleep now
      can_sleep = false;
    }
    else
    if (main_clock_raised)
    {
      if ((millis() - main_clock_raise_ms) > MAIN_CLOCK_RAISE_MS)
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_Clock_Check_Sleep_Mode: raised state timeout");
#endif
  
        main_clock_raised = false;
      }
      else
      {
        // Still raised, can't sleep now
        can_sleep = false;
      }
    }
  }
  else
  {
    if (main_clock_raised) main_clock_raised = false;
  }

#ifdef  MAIN_DEBUG
/*
  // Debug-print
  if ((settings.main_sleep_enabled) && (time_set))
  {
    Serial.printf("Sleep: S=%u s, WUP=%u s, NOW=%u s\n"
                  "       timeset=%u, enabled=%u, raised=%u -> %s\n",
                  sleep_secs,
                  wakeup_secs,
                  now_secs,
                  time_set,
                  settings.main_sleep_enabled,
                  main_clock_raised,
                  can_sleep ? "yes" : "no");
  }
  else
  {
    Serial.printf("Sleep: timeset=%u, enabled=%u, raised=%u -> %s\n",
                  time_set,
                  settings.main_sleep_enabled,
                  main_clock_raised,
                  can_sleep ? "yes" : "no");
  }
*/
#endif

  // Handle transitions between suspended and resumed state
  if ((!main_clock_suspended) && (can_sleep))
  {
#ifdef  MAIN_DEBUG
    Serial.println("Main_Clock_Check_Sleep_Mode: suspending clock");
#endif

    Clock_Suspend();
    main_clock_suspended = true;
  }
  else
  if ((main_clock_suspended) && (!can_sleep))
  {
#ifdef  MAIN_DEBUG
    Serial.println("Main_Clock_Check_Sleep_Mode: resuming clock");
#endif

    Clock_Resume();
    main_clock_suspended = false;
  }
}


void  Main_Clock_Report_Sec (TimeElements *tm, boolean time_set)
{
  if (time_set) Main_NTP_Check_Refresh();

  Main_Clock_Check_Sleep_Mode(time_set);

  if( time_set )
    TS_Store_History(); 
}


void  Main_Clock_Exec ()
{
  boolean   pin_raise;

  pin_raise = (digitalRead(PIN_RAISE) != LOW);

  if (main_clock_pin_raise != pin_raise && !Timer_IsRunning(main_pin_timer) )
  {
    // There was a transition in the state of pin RAISE

    Timer_Start( main_pin_timer, MAIN_DEBOUNCE_MS );

#ifdef  MAIN_DEBUG
    //Serial.printf("Main_Clock_Exec: pin RAISE %u->%u\n",main_clock_pin_raise,pin_raise);
#endif

    main_clock_pin_raise = pin_raise;

    if (!pin_raise)
    {
      // Pin RAISE 1->0 means button pushed: handle now instead of every second

      // Store suspense state at push
      main_suspended_press = main_clock_suspended;

      // Check for wakeup
      Main_Clock_Check_Sleep_Mode(Clock_Is_Time_Set());

      // Start hold timer
      main_clock_hold_ms = millis();
    }
    else
    {
      // Pin RAISE 0->1 means button released

      if (main_clock_hold_applied)
      {
        main_clock_hold_applied = false;

        Settings_Write();
      }

      if (main_clock_hold_repeat)
      {
        // Don't show on tube display
        Clock_Set_Adjusting(false);

        main_clock_hold_repeat = false;
      }

      // Start cyclong only when released not in adjust mode and not directly after wakeup
      else if( !main_suspended_press )
        Clock_Start_Cycling();
    }
  }
  else
  if (!pin_raise)
  {
    if (!main_clock_hold_repeat)
    {
      if ((millis() - main_clock_hold_ms) > MAIN_CLOCK_HOLD_START_MS)
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_Clock_Exec: hold start timeout");
#endif

        // Go to repeat state
        main_clock_hold_repeat = true;

        // Show on tube display
        Clock_Set_Adjusting(true);

        // Restart hold timer
        main_clock_hold_ms = millis();
      }
    }
    else
    {
      if ((millis() - main_clock_hold_ms) > MAIN_CLOCK_HOLD_REPEAT_MS)
      {
#ifdef  MAIN_DEBUG
        Serial.println("Main_Clock_Exec: hold repeat timeout");
#endif

        // Apply the next set of adjustment values
        Clock_Apply_Next_Adjust();
        main_clock_hold_applied = true;
  
        // Restart hold timer
        main_clock_hold_ms = millis();
      }
    }
  }
}


void  Main_Clock_Init ()
{
  pinMode(PIN_RAISE,INPUT);
  main_clock_pin_raise = true;

  Clock_Set_Report_Sec(Main_Clock_Report_Sec);
}


void  Main_Exec ()
{
  Main_WIFI_Exec();
  Main_NTP_Exec();
  Main_Cmd_Server_Exec();
  Main_Clock_Exec();
}


void  Main_Init ()
{
  Main_WIFI_Init();
  Main_NTP_Init();
  Main_Cmd_Server_Init();
  Main_Clock_Init();
}
