
/*
  Serial interface
*/


CMD_PROC  serial_rcv_cmd_proc;
char      serial_rcv_buf[16];


CMD_PROC  *Serial_Get_Cmd_Proc ()
{
  return &serial_rcv_cmd_proc;
}


// Receive characters from the serial port in non-blocking fashion.

void  Serial_Exec ()
{
  char      c;
  uint32_t  cnt;

  cnt = 0;
  while (Serial.available() > 0)
  {
    serial_rcv_buf[cnt] = Serial.read();
    cnt++;
    if (cnt == sizeof(serial_rcv_buf))
    {
      Cmd_Proc_Feed(&serial_rcv_cmd_proc,serial_rcv_buf,cnt);
      cnt = 0;
    }
  }

  if (cnt > 0) Cmd_Proc_Feed(&serial_rcv_cmd_proc,serial_rcv_buf,cnt);
}


void  Serial_Emit_Str (CMD_PROC *p, char *s, uint32_t cnt)
{
  Serial.println(s);
}


void  Serial_Close (CMD_PROC *p)
{
  // N.A. - can't close the serial interface
}


void  Serial_Init ()
{
  // Initialize serial interface
  Serial.begin(115200);

  serial_rcv_cmd_proc.emit_str_fn = Serial_Emit_Str;
  serial_rcv_cmd_proc.close_fn    = Serial_Close;

  serial_rcv_cmd_proc.echo_commands = settings.serial_echo_commands;
}

