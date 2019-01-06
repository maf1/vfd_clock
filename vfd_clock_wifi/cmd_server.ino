
/*
  Command server
*/


#define CMD_SERVER_MAX_CLIENTS    4

WiFiServer          cmd_server(5010,CMD_SERVER_MAX_CLIENTS);
CMD_SERVER_CLIENT   cmd_server_clients[CMD_SERVER_MAX_CLIENTS];


// Print connection information

void  Cmd_Server_Client_Dump (CMD_PROC *p, uint32_t client_index)
{
  CMD_SERVER_CLIENT  *client;
  char                line[80];
  int                 cnt;

  client = &cmd_server_clients[client_index];

  cnt = sprintf(line,"Client slot #%u: ",client_index);
  if (client->client.connected())
  {
    cnt += sprintf(line+cnt,
                   "%s:%u <-> %s:%u",
                   client->client.localIP().toString().c_str(),
                   client->client.localPort(),
                   client->client.remoteIP().toString().c_str(),
                   client->client.remotePort());
  }
  else
  {
    cnt += sprintf(line+cnt,"not connected");
  }

  p->emit_str_fn(p,line,cnt);
}


boolean  Cmd_Server_Is_Listening ()
{
  // DBG.
  {
    static boolean started = false;
    if (cmd_server)
    {
      if (!started)
      {
        started = true;

#ifdef  CMD_SERVER_DEBUG
        Serial.println("Command server has started");
#endif
      }
    }
    else
    {
      if (started)
      {
        started = false;

#ifdef  CMD_SERVER_DEBUG
        Serial.println("Command server has stopped");
#endif
      }
    }
  }
  return cmd_server;
}


uint32_t  Cmd_Server_Get_Free_Client_Slot ()
{
  uint32_t  client_index;

  for (client_index = 0; client_index < CMD_SERVER_MAX_CLIENTS; client_index++)
  {
    if (!cmd_server_clients[client_index].client.connected())
    {
      //cmd_server_clients[client_index].client.stop();
      break;
    }
  }

  return client_index;
}


void  Cmd_Server_Exec ()
{
  uint32_t  client_index;

  if (Cmd_Server_Is_Listening())
  {
    // The server port has an incoming client connection
    if (cmd_server.hasClient())
    {
      // Look up a free slot for handling the new client connection. There is only room for a
      // fixed number of client connections.
      client_index = Cmd_Server_Get_Free_Client_Slot();
      if (client_index == CMD_SERVER_MAX_CLIENTS)
      {
        // Can't take another incoming connection, instead shut it down
        cmd_server.available().stop();

#ifdef  CMD_SERVER_DEBUG
        Serial.println("Maximum number of clients already reached");
#endif
      }
      else
      {
        // Transfer client context to our own storage
        cmd_server_clients[client_index].client = cmd_server.available();

        // Set up client connection
        cmd_server_clients[client_index].client.setNoDelay(true);
  
        Cmd_Proc_Reset(&cmd_server_clients[client_index].cmd_proc);

        cmd_server_clients[client_index].cmd_proc.echo_commands = settings.cs_echo_commands;

        // Debug-printing
        Cmd_Server_Client_Dump(Serial_Get_Cmd_Proc(),client_index);
      }
    }
  }

  for (client_index = 0; client_index < CMD_SERVER_MAX_CLIENTS; client_index++)
  {
    if (cmd_server_clients[client_index].client.connected())
    {
      // The client is connected. If the connection gets broken, the client object will revert
      // to the disconnected state which this code will pick up during the next iteration. There's
      // no need to do anything when the client gets disconnected, the client object takes care of
      // everything. Also, on disconnect, the slot in our list of client revert to the free state
      // as well, no need to do anything special there either.

      // Check for received data
      while (cmd_server_clients[client_index].client.available() > 0)
      {
        static  uint8_t   buf[16];

        int   xfrd;

        xfrd = cmd_server_clients[client_index].client.read(buf,sizeof(buf));
        if (xfrd > 0)
        {
          // Feed the received data bytes to the command processor
          Cmd_Proc_Feed(&cmd_server_clients[client_index].cmd_proc,(char*)buf,xfrd);
        }
      }
    }
  }
}


void  Cmd_Server_Client_Close (uint32_t client_index)
{
  if (client_index >= CMD_SERVER_MAX_CLIENTS) return;

  cmd_server_clients[client_index].client.stop();
}


void  Cmd_Server_Client_Close_All ()
{
  for (uint32_t client_index = 0; client_index < CMD_SERVER_MAX_CLIENTS; client_index++)
  {
    cmd_server_clients[client_index].client.stop();
  }
}


void  Cmd_Server_Dump (CMD_PROC *p)
{
  char   line[80];
  int    cnt;

  cnt = sprintf(line,"Server: %s",cmd_server ? "started (listening)" : "stopped (not listening)");
  p->emit_str_fn(p,line,cnt);

  for (uint32_t client_index = 0; client_index < CMD_SERVER_MAX_CLIENTS; client_index++)
  {
      Cmd_Server_Client_Dump(p,client_index);
  }
}


void  Cmd_Server_Client_Emit_Str (CMD_PROC *p, char *s, uint32_t cnt)
{
  CMD_SERVER_CLIENT  *client;

  client = GetContAd(p,CMD_SERVER_CLIENT,cmd_proc);

  if (cnt == 0) cnt = strlen(s);

  // String characters
  client->client.write(s,cnt);

  // EOL CR->LF
  client->client.write(13);
  client->client.write(10);
}


void  Cmd_Server_Client_Close (CMD_PROC *p)
{
  CMD_SERVER_CLIENT  *client;

  client = GetContAd(p,CMD_SERVER_CLIENT,cmd_proc);

  // Shut down the client connection
  client->client.stop();
}


boolean  Cmd_Server_Has_Started ()
{
  return !!cmd_server;
}


// Note: if the server is stopped while client connections are existing or after client
// connections have been closed, the server won't restart again. To fix this situation,
// disconnect and reconnect WIFI.

void  Cmd_Server_Stop ()
{
  // Close all client connections
  //Cmd_Server_Client_Close_All();

  // Stop the server
  cmd_server.end();
}


void  Cmd_Server_Start ()
{
  // Start the server (unless it's already listening)
  cmd_server.begin();

  // Don't delay the emission of packets
  cmd_server.setNoDelay(true);

  if (!cmd_server)
  {
#ifdef  CMD_SERVER_DEBUG
    Serial.println("Command server failed to start");
#endif
  }
}


void  Cmd_Server_Init ()
{
  uint32_t  client_index;

  // Set up the client structures
  for (client_index = 0; client_index < CMD_SERVER_MAX_CLIENTS; client_index++)
  {
    cmd_server_clients[client_index].cmd_proc.emit_str_fn = Cmd_Server_Client_Emit_Str;
    cmd_server_clients[client_index].cmd_proc.close_fn    = Cmd_Server_Client_Close;
  }
}

