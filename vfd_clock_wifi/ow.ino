
/*
  1-Wire master

  Notes:

  * Don't activate DQ pin pull-up in he microcontroller. An external pull-up resistor must
    be provided.

  * The code keeps locking to a minimum. Part of timing that may take longer than specified
    (like the last part of a reset or a bit transfer) and will be executed unlocked. Anyway
    there's other code inbetween bus transactions so it's useless to lock a complete bus
    transaction.
*/


byte            ow_pin_dq;
OW_LOCK_FN     *ow_lock_fn;
OW_UNLOCK_FN   *ow_unlock_fn;
OW_ENUM         ow_enum;
OW_TOUCH_BITS   ow_touch_bits;


// Notes:
// * Delay G: Delay before start of RESET signaling. Not applicable for 1-Wire
//   standard speed.

#define OW_DELAY_A_US       6
#define OW_DELAY_B_US      64
#define OW_DELAY_C_US      60
#define OW_DELAY_D_US      10
#define OW_DELAY_E_US       9
#define OW_DELAY_F_US      55
#define OW_DELAY_H_US     480
#define OW_DELAY_I_US      70
#define OW_DELAY_J_US     410


void  OW_Lock ()
{
  if (ow_lock_fn) ow_lock_fn();
}


void  OW_Unlock ()
{
  if (ow_unlock_fn) ow_unlock_fn();
}


#define OW_CRC8_MASK    0x8C


byte  OW_CRC8_Byte (byte crc, byte data_byte)
{
  byte    u;
  byte    data_bit;
  byte    xor_ctrl_bit;
  byte    out_bit;

  for (u = 0; u < 8; u++)
  {
    data_bit = (data_byte & 0x01) ? 1 : 0;
    data_byte = data_byte >> 1;

    // Bit 0 will be shifted out
    if (crc & 0x01) out_bit = 1; else out_bit = 0;

    // Bit to be shifted in
    if (data_bit ^ out_bit) xor_ctrl_bit = 1; else xor_ctrl_bit = 0;

    // Shift right and XOR if needed
    crc = crc >> 1;
    if (xor_ctrl_bit) crc = crc ^ OW_CRC8_MASK;
  }

  return crc;
}


byte  OW_CRC8_Buf (byte crc, byte *buf, byte bytes)
{
  while (bytes > 0)
  {
    crc = OW_CRC8_Byte(crc,*buf);
    buf++;
    bytes--;
  }

  return crc;
}


void  OW_Bus_Release ()
{
  // Set as input
  pinMode(ow_pin_dq,INPUT);
}


void  OW_Bus_Pull_Low ()
{
  // Set low
  digitalWrite(ow_pin_dq,LOW);

  // Set as output
  pinMode(ow_pin_dq,OUTPUT);
}


byte  OW_Bus_Sample ()
{
  return digitalRead(ow_pin_dq);
}


boolean  OW_Bus_Reset ()
{
  int   state;

  // Locked: drive bus low, delay H, release bus, delay I and sample
  OW_Lock();
  OW_Bus_Pull_Low();
  delayMicroseconds(OW_DELAY_H_US);
  OW_Bus_Release();
  delayMicroseconds(OW_DELAY_I_US);
  state = OW_Bus_Sample();
  OW_Unlock();

  // Unlocked: delay J
  delayMicroseconds(OW_DELAY_J_US);

  // Report presence (state=0 indicate presence)
  return (state == 0);
}


void  OW_Bus_Write_Bit_Zero ()
{
  // Locked: drive bus low, delay C, release bus, delay D
  OW_Lock();
  OW_Bus_Pull_Low();
  delayMicroseconds(OW_DELAY_C_US);
  OW_Bus_Release();
  OW_Unlock();

  // Unlocked: delay D
  delayMicroseconds(OW_DELAY_D_US);
}


byte  OW_Bus_Read_Bit ()
{
  byte  state;

  // Locked: drive bus low, delay A, release bus, delay E and sample
  OW_Lock();
  OW_Bus_Pull_Low();
  delayMicroseconds(OW_DELAY_A_US);
  OW_Bus_Release();
  delayMicroseconds(OW_DELAY_E_US);
  state = OW_Bus_Sample();
  OW_Unlock();

  // Unlocked: delay F
  delayMicroseconds(OW_DELAY_F_US);

  return state;
}


void  OW_Enum_Set_Exec_State (byte state)
{
  ow_enum.exec_state = state;
}


void  OW_Enum_Exec ()
{
  switch (ow_enum.exec_state)
  {
    case 0:
    {
      // Idle
      break;
    }

    case 1:
    {
      boolean   presence;

      presence = OW_Bus_Reset();
      if (presence)
      {
        ow_enum.xfr_bit_index = 0;

        OW_Enum_Set_Exec_State(2);
      }
      else
      {
        // No 1-Wire slaves present. Complete command without resulting ROM code.
        ow_enum.rom_code_found = false;
        OW_Enum_Set_Exec_State(0);
      }

      break;
    }

    case 2:
    {
      if (ow_enum.xfr_byte & (1 << ow_enum.xfr_bit_index))
      {
#ifdef  OW_DEBUG
        Serial.println("OW ENUM: writing 1");
#endif

        // Write one-bit
        OW_Bus_Read_Bit();
      }
      else
      {
#ifdef  OW_DEBUG
        Serial.println("OW ENUM: writing 0");
#endif

        // Write zero-bit
        OW_Bus_Write_Bit_Zero();
      }

      // Next bit
      ow_enum.xfr_bit_index++;
      if (ow_enum.xfr_bit_index < 8) break;

      // Start transferring the triplets
      ow_enum.xfr_byte_index = 0;
      ow_enum.xfr_bit_index  = 0;
      ow_enum.prev_diff_nbr  = 0;
      ow_enum.bit_nbr        = 1;

      OW_Enum_Set_Exec_State(3);
      break;
    }

    case 3:
    {
      // Read first bit
      ow_enum.res_bit_1 = OW_Bus_Read_Bit();

      OW_Enum_Set_Exec_State(4);
      break;
    }

    case 4:
    {
      // Read second bit
      ow_enum.res_bit_2 = OW_Bus_Read_Bit();

      OW_Enum_Set_Exec_State(5);
      break;
    }

    case 5:
    {
      byte  mask;
      byte  bit_dir;
      byte  res_bit_dir;

      mask = 1 << ow_enum.xfr_bit_index;

      // Determine the direction bit we want to use when 1-wire devices
      // report different bits. The decision depends on the current bit
      // position and the last difference position.
      //
      // Cases:
      // * bit_nbr<last_diff_nbr: choose from previously found ROM code.
      // * bit_nbr=last_diff_nbr: choose one, revert last_diff_nbr to
      //     prev_diff_nbr.
      // * last_diff_nbr<bit_nbr: (includes last_diff_nbr=0): choose
      //     zero.

      if (ow_enum.bit_nbr < ow_enum.last_diff_nbr)
      {
        //        choose from previously found ROM code
        //      /
        // <------------------->
        // 0X11X0010101_........X
        //     |       |        |
        //     |       |  last_diff_nbr
        //     |    bit_nbr
        // prev_diff_nbr

        bit_dir = ow_enum.prev_rom_code[ow_enum.xfr_byte_index] & mask;
      }
      else
      if (ow_enum.bit_nbr == ow_enum.last_diff_nbr)
      {
        // We've reached the last difference position of the previous scan

        bit_dir = 1;

        ow_enum.last_diff_nbr = ow_enum.prev_diff_nbr;
        ow_enum.prev_diff_nbr = 0;
      }
      else
      {
        // last_diff_nbr < bit_nbr
        //
        //                choose direction zero
        //              /
        //           <--->
        // 0X0011X00X1101_
        //       |  |    |
        //       |  |  bit_nbr
        //       | last_diff_nbr
        //      prev_diff_nbr

        bit_dir = 0;
      }

      // Make bit_dir 0 or 1, not 0 or any non-zero value. We're going to compare
      // bit_dir and res_bit_dir later on so both must be either 0 or 1.

      if (bit_dir) bit_dir = 1;


      // Write the direction bit
  
      // read slot 1   read slot 2   write slot   Meaning
      // -----------   -----------   ----------   -----------------------------
      //      0             0           dir       Devices report different bits
      //      0             1            0        All devices report 0
      //      1             0            1        All devices report 1
      //      1             1            1        No device present
  
      if (ow_enum.res_bit_1 == 0)
      {
        if (ow_enum.res_bit_2 == 0)
        {
          // res_bit_1,res_bit_2 = 0,0
          res_bit_dir = bit_dir;
        }
        else
        {
          // res_bit_1,res_bit_2 = 0,1
          res_bit_dir = 0;
        }
      }
      else
      {
        // res_bit_1,res_bit_2 = 1,0
        // res_bit_1,res_bit_2 = 1,1
        res_bit_dir = 1;
      }

      if (res_bit_dir)
      {
        // Write one-bit
        OW_Bus_Read_Bit();
      }
      else
      {
        // Write zero-bit
        OW_Bus_Write_Bit_Zero();
      }

#ifdef  OW_DEBUG
      Serial.printf("OW ENUM: %u %u => %u\n",ow_enum.res_bit_1,ow_enum.res_bit_2,res_bit_dir);
#endif


      // Process the results of the Wire Triplet command. Resulting bits:
      //
      //  1   2  dir
      // --- --- ---
      //  0   0   X  The devices reported different bits.
      //  0   1   0  All devices reported zero.
      //  1   0   1  All devices reported one.
      //  1   1   1  No response from any device.

      if ((ow_enum.res_bit_1 != 0) && (ow_enum.res_bit_2 != 0))
      {
        // No response from any 1-wire device

        // No 1-Wire slaves present. Complete command without resulting ROM code.
        ow_enum.rom_code_found = false;
        OW_Enum_Set_Exec_State(0);
        return;
      }
      else
      {
        if ((ow_enum.res_bit_1 == 0) && (ow_enum.res_bit_2 == 0))
        {
          // Different bits reported

          if (res_bit_dir == 0)
          {
            // Keep track of the last and previous bit locations when
            // direction zero is taken.
            //
            // Note: last_diff_nbr <> bit_nbr at this point (see above).

            if (ow_enum.last_diff_nbr < ow_enum.bit_nbr)
            {
              ow_enum.prev_diff_nbr = ow_enum.last_diff_nbr;
              ow_enum.last_diff_nbr = ow_enum.bit_nbr;
            }
            else
            {
              // bit_nbr < last_diff_nbr

              ow_enum.prev_diff_nbr = ow_enum.bit_nbr;
            }
          }
          // else: Direction one was taken. The bit position is resolved.
        }
      }

      // Store the found bit value
      if (res_bit_dir == 0)
        ow_enum.rom_code[ow_enum.xfr_byte_index] &= ~mask;
      else
        ow_enum.rom_code[ow_enum.xfr_byte_index] |= mask;


      // Triplet loop control

      ow_enum.xfr_bit_index++;
      if (ow_enum.xfr_bit_index == 8)
      {
        ow_enum.xfr_bit_index = 0;
        ow_enum.xfr_byte_index++;
      }

      if (ow_enum.xfr_byte_index == 8)
      {
        // Found a ROM code

        // Iteration control (determine whether there is a next iteration)
        if (ow_enum.last_diff_nbr == 0)
        {
#ifdef  OW_DEBUG
          Serial.println("OW ENUM: last one");
#endif

          // No different bits were reported (no discrepancies). All ROM
          // codes have been enumerated. This iteration was the last one.
        }
        else
        {
#ifdef  OW_DEBUG
          Serial.println("OW ENUM: more slaves out there");
#endif

          // Keep a copy of the found ROM code as the previous ROM code.
          // We'll need it for handling the next iteration.
          memcpy(ow_enum.prev_rom_code,ow_enum.rom_code,sizeof(ow_enum.rom_code));
        }

        byte crc = OW_CRC8_Buf(0,ow_enum.rom_code,8);

        // Complete command, report ROM coded found
        ow_enum.rom_code_found = true;
        ow_enum.crc_valid      = (crc == 0x00);
        OW_Enum_Set_Exec_State(0);
        return;
      }
      else
      {
        // Issue the next triplet

        ow_enum.bit_nbr++;

        OW_Enum_Set_Exec_State(3);
        return;
      }
    }
  }
}


boolean  OW_Enum_Is_Busy ()
{
  return (ow_enum.exec_state != 0);
}


boolean  OW_Enum_Is_CRC_Valid ()
{
  return ow_enum.crc_valid;
}


// Return value:
// * =0: No ROM code was enumerated.
// * >0: Pointer to 8-byte ROM code.

byte  *OW_Enum_Get_ROM_Code ()
{
  return ow_enum.rom_code_found ? ow_enum.rom_code : 0;
}


void  OW_Enum_Start_Main ()
{
  ow_enum.xfr_byte = 0xF0;  // SEARCH ROM command

  OW_Enum_Set_Exec_State(1);
}


void  OW_Enum_First_Start ()
{
  memset(ow_enum.prev_rom_code,0,sizeof(ow_enum.prev_rom_code));
  ow_enum.last_diff_nbr = 0;

  OW_Enum_Start_Main();
}


void  OW_Enum_Next_Start ()
{
  if (ow_enum.last_diff_nbr == 0)
  {
    ow_enum.rom_code_found = false;
  }
  else
  {
    OW_Enum_Start_Main();
  }
}


void  OW_Touch_Bits_Set_Exec_State (byte state)
{
  ow_touch_bits.exec_state = state;
}


void  OW_Touch_Bits_Exec ()
{
  switch (ow_touch_bits.exec_state)
  {
    case 0:
    {
      // Idle
      break;
    }

    case 1:
    {
      byte  mask;
      byte  state;

      mask = 1 << ow_touch_bits.bit_index;

      state = ow_touch_bits.buf[ow_touch_bits.byte_index] & mask;

#ifdef  OW_DEBUG
      Serial.printf("OW TOUCH BITS: %u->",state);
#endif

      if (state)
      {
        // Write one-bit, read back bit
        state = OW_Bus_Read_Bit();

        if (!ow_touch_bits.buf_ro)
        {
          if (!state) ow_touch_bits.buf[ow_touch_bits.byte_index] &= ~mask;
        }
      }
      else
      {
        // Write zero-bit
        OW_Bus_Write_Bit_Zero();
      }

#ifdef  OW_DEBUG
      Serial.printf("%u\n",state);
#endif

      ow_touch_bits.bits_left--;
      if (ow_touch_bits.bits_left == 0)
      {
        // Complete the TOUCH BITS command
        OW_Touch_Bits_Set_Exec_State(0);
        return;
      }

      ow_touch_bits.bit_index++;
      if (ow_touch_bits.bit_index == 8)
      {
        ow_touch_bits.bit_index = 0;
        ow_touch_bits.byte_index++;
      }

      return;
    }
  }
}


boolean  OW_Touch_Bits_Is_Busy ()
{
  return (ow_touch_bits.exec_state != 0);
}


void  OW_Touch_Bits_Start (byte *buf, uint16_t bits, boolean buf_ro)
{
  if (bits == 0) return;

#ifdef  OW_DEBUG
  Serial.printf("OW_Touch_Bits_Start: %u bit(s), buffer %s\n",
                bits,
                buf_ro ? "read-only" : "read-write");
#endif

  ow_touch_bits.buf        = buf;
  ow_touch_bits.bits_left  = bits;
  ow_touch_bits.byte_index = 0;
  ow_touch_bits.bit_index  = 0;
  ow_touch_bits.buf_ro     = buf_ro;

  OW_Touch_Bits_Set_Exec_State(1);
}


void  OW_Exec ()
{
  if (OW_Enum_Is_Busy()) OW_Enum_Exec(); else
  if (OW_Touch_Bits_Is_Busy()) OW_Touch_Bits_Exec();
}


void  OW_Init (byte pin_dq, OW_LOCK_FN *lock_fn, OW_UNLOCK_FN *unlock_fn)
{
  ow_pin_dq    = pin_dq;
  ow_lock_fn   = lock_fn;
  ow_unlock_fn = unlock_fn;

  OW_Bus_Release();
}

