
/*
  WIFI
*/


byte      wifi_exec_state     = 0;
boolean   wifi_req_connect    = false;
boolean   wifi_req_disconnect = false;
boolean   wifi_is_connected   = false;
boolean   wifi_is_connecting  = false;


boolean  WIFI_Is_Disconnected ()
{
  return (wifi_exec_state == 0);
}


boolean  WIFI_Is_Connecting ()
{
  return (wifi_exec_state == 1);
}


boolean  WIFI_Is_Connected ()
{
  return (wifi_exec_state == 2);
}


boolean  WIFI_Is_Disconnecting ()
{
  return (wifi_exec_state == 3);
}


char  *WIFI_Get_Event_Name (WiFiEvent_t event)
{
  switch (event)
  {
    case SYSTEM_EVENT_WIFI_READY:          return "SYSTEM_EVENT_WIFI_READY";
    case SYSTEM_EVENT_SCAN_DONE:           return "SYSTEM_EVENT_SCAN_DONE";
    case SYSTEM_EVENT_STA_START:           return "SYSTEM_EVENT_STA_START";
    case SYSTEM_EVENT_STA_STOP:            return "SYSTEM_EVENT_STA_STOP";
    case SYSTEM_EVENT_STA_CONNECTED:       return "SYSTEM_EVENT_STA_CONNECTED";
    case SYSTEM_EVENT_STA_DISCONNECTED:    return "SYSTEM_EVENT_STA_DISCONNECTED";
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: return "SYSTEM_EVENT_STA_AUTHMODE_CHANGE";
    case SYSTEM_EVENT_STA_GOT_IP:          return "SYSTEM_EVENT_STA_GOT_IP";
    case SYSTEM_EVENT_STA_LOST_IP:         return "SYSTEM_EVENT_STA_LOST_IP";
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:  return "SYSTEM_EVENT_STA_WPS_ER_SUCCESS";
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:   return "SYSTEM_EVENT_STA_WPS_ER_FAILED";
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:  return "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT";
    case SYSTEM_EVENT_STA_WPS_ER_PIN:      return "SYSTEM_EVENT_STA_WPS_ER_PIN";
    case SYSTEM_EVENT_AP_START:            return "SYSTEM_EVENT_AP_START";
    case SYSTEM_EVENT_AP_STOP:             return "SYSTEM_EVENT_AP_STOP";
    case SYSTEM_EVENT_AP_STACONNECTED:     return "SYSTEM_EVENT_AP_STACONNECTED";
    case SYSTEM_EVENT_AP_STADISCONNECTED:  return "SYSTEM_EVENT_AP_STADISCONNECTED";
    case SYSTEM_EVENT_AP_PROBEREQRECVED:   return "SYSTEM_EVENT_AP_PROBEREQRECVED";
    case SYSTEM_EVENT_AP_STA_GOT_IP6:      return "SYSTEM_EVENT_AP_STA_GOT_IP6";
    case SYSTEM_EVENT_ETH_START:           return "SYSTEM_EVENT_ETH_START";
    case SYSTEM_EVENT_ETH_STOP:            return "SYSTEM_EVENT_ETH_STOP";
    case SYSTEM_EVENT_ETH_CONNECTED:       return "SYSTEM_EVENT_ETH_CONNECTED";
    case SYSTEM_EVENT_ETH_DISCONNECTED:    return "SYSTEM_EVENT_ETH_DISCONNECTED";
    case SYSTEM_EVENT_ETH_GOT_IP:          return "SYSTEM_EVENT_ETH_GOT_IP";
    default:                               return "???";
  }
}


char  *WIFI_Get_Auth_Mode_Descr (wifi_auth_mode_t u)
{
  switch (u)
  {
    case WIFI_AUTH_OPEN:            return "OPEN";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA PSK";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2 PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA WPA2 PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 ENTERPRISE";
    default:                        return "???";
  }
}


char  *WIFI_Get_Status_Descr (wl_status_t st)
{
  switch (st)
  {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO SSID AVAILABLE";
    case WL_SCAN_COMPLETED:  return "SCAN COMPLETED";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECTION FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    case WL_NO_SHIELD:       return "NO SHIELD";
    default:                 return "???";
  }
}


void  WIFI_Dump (CMD_PROC *p)
{
  char   line[80];
  int    cnt;

  cnt = sprintf
        (
          line,
          "WiFi: %d RSSI, %s",
          WiFi.RSSI(),
          wifi_is_connected ? wifi_is_connecting ? "connecting" : "connected" : "disconnected"
        );
  p->emit_str_fn(p,line,cnt);

  wl_status_t st = WiFi.status();
  cnt = sprintf(line,"* status:   %s (%u)",WIFI_Get_Status_Descr(st),st);
  p->emit_str_fn(p,line,cnt);

  if ((wifi_is_connected) && (!wifi_is_connecting))
  {
    cnt = sprintf(line,"* local:    %s",WiFi.localIP().toString().c_str());
    p->emit_str_fn(p,line,cnt);

    cnt = sprintf(line,"* Subnet:   %s",WiFi.subnetMask().toString().c_str());
    p->emit_str_fn(p,line,cnt);

    cnt = sprintf(line,"* Gateway:  %s",WiFi.gatewayIP().toString().c_str());
    p->emit_str_fn(p,line,cnt);

    for (uint8_t u = 0; u < 8; u++)
    {
      IPAddress ad = WiFi.dnsIP(u);
      if (ad != 0)
      {
        cnt = sprintf(line,"* DNS #%u:   %s",u+1,WiFi.dnsIP(u).toString().c_str());
        p->emit_str_fn(p,line,cnt);
      }
    }

    cnt = sprintf(line,"* MAC:      %s",WiFi.macAddress().c_str());
    p->emit_str_fn(p,line,cnt);

    cnt = sprintf(line,"* Hostname: %s",WiFi.getHostname());
    p->emit_str_fn(p,line,cnt);

    cnt = sprintf(line,"* SSID:     %s",WiFi.SSID().c_str());
    p->emit_str_fn(p,line,cnt);

    cnt = sprintf(line,"* BSSID:    %s",WiFi.BSSIDstr().c_str());
    p->emit_str_fn(p,line,cnt);

    // Print password: WiFi.psk()
  }
}


// The event handler is called from a FreeRTOS task that dispatches the events from
// a queue. It's okay to invoke methods of the WiFi object from here.

void  WIFI_Event_Handler (WiFiEvent_t event)
{
#ifdef  WIFI_DEBUG
  Serial.printf("WiFi event %2d - %s\n",event,WIFI_Get_Event_Name(event));
  wl_status_t st = WiFi.status();
  Serial.printf("WiFi status %u - %s\n",st,WIFI_Get_Status_Descr(st));
#endif

  switch (event)
  {
    case SYSTEM_EVENT_STA_CONNECTED:
    {
#ifdef  WIFI_DEBUG
      Serial.println("WiFi connected");
#endif

      break;
    }

    case SYSTEM_EVENT_STA_GOT_IP:
    {
#ifdef  WIFI_DEBUG
      Serial.print("WiFi got IP: ");
      Serial.println(WiFi.localIP());
#endif

      wifi_is_connected  = true;
      wifi_is_connecting = false;

      break;
    }

    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
#ifdef  WIFI_DEBUG
      Serial.println("WiFi lost connection");
#endif

      wifi_is_connected  = false;
      wifi_is_connecting = false;

      break;
    }
  }
}


void  WIFI_Exec_Set_State (byte s)
{
  byte  prev_s;

  prev_s = wifi_exec_state;

  wifi_exec_state = s;

#ifdef  WIFI_DEBUG
  //Serial.printf("wifi_exec_state %u->%u\n",prev_s,s);
#endif
}


boolean  WIFI_Chk_Conn_Settings ()
{
  // SSID expected
  if (strlen(settings.wifi_ssid) == 0) return false;

  if (settings.wifi_use_static)
  {
    // Using static IP - expect local IP and subnet in settings
    if (settings.wifi_local_ip == 0) return false;
    if (settings.wifi_subnet == 0) return false;
  }

  return true;
}


void  WIFI_Exec ()
{
  switch (wifi_exec_state)
  {
    // WiFi is disconnected from the network. Wait for a request to connect.
    case 0:
    {
      if (wifi_req_connect)
      {
        boolean   ok;

        // Check WIFI connection settings
        if (!WIFI_Chk_Conn_Settings())
        {
            WIFI_Exec_Set_State(3);
            return;
        }

        if (settings.wifi_use_static)
        {
          // Configure for static IP (by specifying local IP as non-zero)
          ok = WiFi.config
                (
                  settings.wifi_local_ip,
                  settings.wifi_gateway,
                  settings.wifi_subnet,
                  settings.wifi_dns_list[0],
                  settings.wifi_dns_list[1]
                );
          if (ok == false)
          {
#ifdef  WIFI_DEBUG
            Serial.println("Error: can't configure WiFi for static IP");
#endif

            return;
          }
        }
        else
        {
          // Configure for DHCP (by specifying local IP as zero)
          ok = WiFi.config((uint32_t)0,(uint32_t)0,(uint32_t)0,(uint32_t)0,(uint32_t)0);
          if (ok == false)
          {
#ifdef  WIFI_DEBUG
            Serial.println("Error: can't configure WiFi for dynamic IP");
#endif

            return;
          }
        }

        // Set the hostname for the WIFI device. The hostname is usually retrievable when
        // DHCP is used.
        WiFi.setHostname(settings.wifi_hostname);

        // Begin connecting. This function returns a wl_status_t value. The code ignores
        // this value.
        WiFi.begin(settings.wifi_ssid,settings.wifi_password);

        wifi_is_connecting = true;

        WIFI_Exec_Set_State(1);
      }

      return;
    }

    // WiFi is connecting to the network. Wait until the connecting phase is done.
    case 1:
    {
      if (!wifi_is_connecting)
      {
        if (wifi_is_connected)
        {
          // WiFi is successfully connected
          WIFI_Exec_Set_State(2);
        }
        else
        {
          // WiFi connection has failed
          WIFI_Exec_Set_State(3);
        }
      }

      return;
    }

    // WiFi is connected to the network.
    case 2:
    {
      if (wifi_req_disconnect)
      {
          WiFi.disconnect();
          WIFI_Exec_Set_State(3);
      }
      else
      if (!wifi_is_connected)
      {
          WIFI_Exec_Set_State(3);
      }

      return;
    }

    // WiFi is disconnecting from the network. Wait until the disconnecting phase is done.
    case 3:
    {
      if (!wifi_is_connected)
      {
        // Clear the connection and disconnection request flags
        wifi_req_disconnect = false;
        wifi_req_connect    = false;

        WIFI_Exec_Set_State(0);
      }

      return;
    }
  }
}


void  WIFI_Connect ()
{
  wifi_req_connect = true;
}


void  WIFI_Disconnect ()
{
  wifi_req_disconnect = true;
}


void  WIFI_Init ()
{
  // Disconnect from an AP if it was previously connected
  WiFi.disconnect();

  // Set event handler
  WiFi.onEvent(WIFI_Event_Handler);

  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
}
