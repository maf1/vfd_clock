
/*
  Display driver

  Notes:

 * An ISR and all functions called from an ISR must be declared IRAM_ATTR. If not, the
   processor may generate a fatal exception.
*/


#if 1

void  IRAM_ATTR  io_delay ()
{
  delayMicroseconds(1);
}

#else

// No delay. We've observed glitches when no delay is used.
#define io_delay()

#endif


void  IRAM_ATTR  emit_bit (byte u)
{
  digitalWrite(PIN_DATA,u);
  io_delay();
  digitalWrite(PIN_CLK,HIGH);
  io_delay();
  digitalWrite(PIN_CLK,LOW);
  io_delay();
}


void  IRAM_ATTR  latch_idle ()
{
  digitalWrite(PIN_LE,LOW);
  io_delay();
}


void  IRAM_ATTR  latch_data ()
{
  digitalWrite(PIN_LE,HIGH);
  io_delay();
}


// Timer interrupt
//
//         |                    3000 us                    |
//       --+-------+-------+-------+-------+-------+-------+-------+--> time
// us      0      500    1000    1500    2000    2500      0      500
// step    0       1       2       3       4       5       0       1
//         | BLANK                                 | GRID  | BLANK

#define DISPLAY_TIMER_INTERVAL        500
#define DISPLAY_TIMER_STEP_BLANK        0
#define DISPLAY_TIMER_STEP_GRID         5
#define DISPLAY_TIMER_STEPS             6


// Line of display data: 21 bits
//
//       DOT SEP1    DOT SEP2
// TUBE L. | | TUBE R. | | GRID1/2/3
// +-----+ | | +-----+ | | +-+
// 0110000_1_0_0110011_0_0_100_b

boolean         display_suspended  = false;
portMUX_TYPE    display_timer_mux  = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t     *display_timer      = NULL;
byte            display_timer_step = 0;
byte            display_line_index = 0;
uint32_t        display_lines[3];         // Line of display data, grid 1-3
uint32_t        display_line_blank;       // Line of display data, grids off


void  IRAM_ATTR  Display_Timer_ISR ()
{
  uint32_t  line;
  byte      left;


  portENTER_CRITICAL_ISR(&display_timer_mux);

  if (display_timer_step == DISPLAY_TIMER_STEP_BLANK)
  {
    // Data for tube blanking. Only separators are driven.
    line = display_line_blank;
  }
  else
  if (display_timer_step == DISPLAY_TIMER_STEP_GRID)
  {
    line = display_lines[display_line_index];

    display_line_index++;
    if (display_line_index == 3) display_line_index = 0;
  }
  else
  {
    // Don't emit display line
    line = 0xFFFFFFFF;
  }

  display_timer_step++;
  if (display_timer_step == DISPLAY_TIMER_STEPS) display_timer_step = 0;

  portEXIT_CRITICAL_ISR(&display_timer_mux);


  // Transfer line of display data to the HV518

  if (line != 0xFFFFFFFF)
  {
    latch_idle();

    // Transfer data into shift register
    left = 21;
    for (;;)
    {
      emit_bit(line & 1 ? HIGH : LOW);
      left--;
      if (left == 0) break;
      line >>= 1;
    }

    // Transfer data to output buffers
    latch_data();
  }
}


boolean  Display_Is_Suspended ()
{
  return display_suspended;
}


void  Display_Suspend ()
{
#ifdef  DISPLAY_DEBUG
  //Serial.println("Display_Suspend");
#endif

  if (!display_suspended)
  {
#ifdef  DISPLAY_DEBUG
    //Serial.println("-> suspending");
#endif

    // Disable the timer
    timerAlarmDisable(display_timer);

    display_suspended = true;

    // Set all used outputs of the HV518 to zero
    latch_idle();
    for (byte u = 0; u < 21; u++) emit_bit(LOW);
    latch_data();

    // Enable standby
    digitalWrite(PIN_STANDBY,HIGH);
  }
}


void  Display_Resume ()
{
#ifdef  DISPLAY_DEBUG
  //Serial.println("Display_Resume");
#endif

  if (display_suspended)
  {
#ifdef  DISPLAY_DEBUG
    //Serial.println("-> resuming");
#endif

    display_suspended = false;

    // Disable standby
    digitalWrite(PIN_STANDBY,LOW);

    // Enable the timer
    timerAlarmEnable(display_timer);
  }
}


enum  _DISPLAY_ROW_INDEX
{
  DISPLAY_ROW_INDEX_T1,
  DISPLAY_ROW_INDEX_T1B,
  DISPLAY_ROW_INDEX_T1C,
  DISPLAY_ROW_INDEX_T1D,
  DISPLAY_ROW_INDEX_T1E,
  DISPLAY_ROW_INDEX_T1F,
  DISPLAY_ROW_INDEX_T1G,
  DISPLAY_ROW_INDEX_T2,
  DISPLAY_ROW_INDEX_T2B,
  DISPLAY_ROW_INDEX_T2C,
  DISPLAY_ROW_INDEX_T2D,
  DISPLAY_ROW_INDEX_T2E,
  DISPLAY_ROW_INDEX_T2F,
  DISPLAY_ROW_INDEX_T2G,
  DISPLAY_ROW_INDEX_T3,
  DISPLAY_ROW_INDEX_T3B,
  DISPLAY_ROW_INDEX_T3C,
  DISPLAY_ROW_INDEX_T3D,
  DISPLAY_ROW_INDEX_T3E,
  DISPLAY_ROW_INDEX_T3F,
  DISPLAY_ROW_INDEX_T3G,
  DISPLAY_ROW_INDEX_T4,
  DISPLAY_ROW_INDEX_T4B,
  DISPLAY_ROW_INDEX_T4C,
  DISPLAY_ROW_INDEX_T4D,
  DISPLAY_ROW_INDEX_T4E,
  DISPLAY_ROW_INDEX_T4F,
  DISPLAY_ROW_INDEX_T4G,
  DISPLAY_ROW_INDEX_T5,
  DISPLAY_ROW_INDEX_T5B,
  DISPLAY_ROW_INDEX_T5C,
  DISPLAY_ROW_INDEX_T5D,
  DISPLAY_ROW_INDEX_T5E,
  DISPLAY_ROW_INDEX_T5F,
  DISPLAY_ROW_INDEX_T5G,
  DISPLAY_ROW_INDEX_T6,
  DISPLAY_ROW_INDEX_T6B,
  DISPLAY_ROW_INDEX_T6C,
  DISPLAY_ROW_INDEX_T6D,
  DISPLAY_ROW_INDEX_T6E,
  DISPLAY_ROW_INDEX_T6F,
  DISPLAY_ROW_INDEX_T6G,
  DISPLAY_ROW_INDEX_DOT1,
  DISPLAY_ROW_INDEX_DOT2,
  DISPLAY_ROW_INDEX_DOT3,
  DISPLAY_ROW_INDEX_DOT4,
  DISPLAY_ROW_INDEX_DOT5,
  DISPLAY_ROW_INDEX_DOT6,
  DISPLAY_ROW_INDEX_SEP1,
  DISPLAY_ROW_INDEX_SEP2,
  DISPLAY_ROW_INDEX_CNT
};


byte  Display_Tube_Row_Index[6] =
{
  DISPLAY_ROW_INDEX_T1,
  DISPLAY_ROW_INDEX_T2,
  DISPLAY_ROW_INDEX_T3,
  DISPLAY_ROW_INDEX_T4,
  DISPLAY_ROW_INDEX_T5,
  DISPLAY_ROW_INDEX_T6
};


// The display row is an array representing the ON or OFF state of each separator and
// each element of the tubes (segments and dots).
//
// Each byte is either zero or one, and is ORed while the display data is shifted in
// the line variables during display rendering.

byte      display_row[DISPLAY_ROW_INDEX_CNT];
boolean   display_row_changed = false;


void  Display_Dump_Line (char *s, uint32_t line)
{
  byte  index;

  index = 20;
  for (;;)
  {
    Serial.printf("%u",line & (1 << 20) ? 1 : 0);

    if ((index == 14) || (index == 13) || (index == 12) || (index == 5) || (index == 4) || (index == 3))
      Serial.print("-");

    if (index == 0) break;
    index--;
    line <<= 1;
  }

  Serial.println();
}


void  Display_Render ()
{
  uint32_t  w;
  uint32_t  line_0;
  uint32_t  line_1;
  uint32_t  line_2;
  uint32_t  line_blank;

  // Line 0
  w = 0;
  w |= display_row[DISPLAY_ROW_INDEX_T1];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T1B];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T1C];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T1D];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T1E];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T1F];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T1G];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_DOT1];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_SEP1];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4B];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4C];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4D];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4E];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4F];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T4G];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_DOT4];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_SEP2];
  w <<= 1;
  w |= 1;   // GRID1=1
  w <<= 2;  // GRID2=0, GRID3=0
  line_0 = w;

  // Line 1
  w = 0;
  w |= display_row[DISPLAY_ROW_INDEX_T2];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T2B];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T2C];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T2D];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T2E];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T2F];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T2G];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_DOT2];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_SEP1];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5B];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5C];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5D];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5E];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5F];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T5G];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_DOT5];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_SEP2];
  w <<= 2;  // GRID1=0
  w |= 1;   // GRID2=1
  w <<= 1;  // GRID3=0
  line_1 = w;

  // Line 2
  w = 0;
  w |= display_row[DISPLAY_ROW_INDEX_T3];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T3B];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T3C];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T3D];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T3E];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T3F];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T3G];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_DOT3];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_SEP1];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6B];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6C];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6D];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6E];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6F];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_T6G];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_DOT6];
  w <<= 1;
  w |= display_row[DISPLAY_ROW_INDEX_SEP2];
  w <<= 3;  // GRID1=0, GRID2=0
  w |= 1;   // GRID3=1
  line_2 = w;

  // Blanking line
  w = 0;
  if (display_row[DISPLAY_ROW_INDEX_SEP1]) w |= 0b000000001000000000000;
  if (display_row[DISPLAY_ROW_INDEX_SEP2]) w |= 0b000000000000000001000;
  line_blank = w;

  portENTER_CRITICAL(&display_timer_mux);

  display_line_blank = line_blank;
  display_lines[0]   = line_0;
  display_lines[1]   = line_1;
  display_lines[2]   = line_2;

  portEXIT_CRITICAL(&display_timer_mux);

/*
  // Debug-print
  Display_Dump_Line("line0: ",line_0);
  Display_Dump_Line("line1: ",line_1);
  Display_Dump_Line("line2: ",line_2);
  Display_Dump_Line("blank: ",line_blank);
*/
}


void  Display_Enter_Timer_Mux ()
{
  portENTER_CRITICAL(&display_timer_mux);
}


void  Display_Exit_Timer_Mux ()
{
  portEXIT_CRITICAL(&display_timer_mux);
}


// Segments (Position in symbol array)
//      0
//    5   1
//      6 
//    4   2
//      3

byte  Display_Symbols[DISPLAY_SYMBOL_CNT][7] =
{
  { 1, 1, 1, 1, 1, 1, 0 },  // Zero - 0
  { 0, 1, 1, 0, 0, 0, 0 },  // One - 1
  { 1, 1, 0, 1, 1, 0, 1 },  // Two - 2
  { 1, 1, 1, 1, 0, 0, 1 },  // Three - 3
  { 0, 1, 1, 0, 0, 1, 1 },  // Four - 4
  { 1, 0, 1, 1, 0, 1, 1 },  // Five - 5
  { 1, 0, 1, 1, 1, 1, 1 },  // Six - 6
  { 1, 1, 1, 0, 0, 0, 0 },  // Seven - 7
  { 1, 1, 1, 1, 1, 1, 1 },  // Eight - 8
  { 1, 1, 1, 1, 0, 1, 1 },  // Nine - 9
  { 0, 0, 0, 0, 0, 0, 1 },  // Hyphen
  { 1, 1, 0, 0, 0, 1, 1 },  // Degrees
  { 1, 0, 0, 1, 1, 1, 0 },  // C
  { 1, 0, 0, 0, 1, 1, 1 },  // F
  { 0, 0, 0, 0, 0, 0, 0 },  // Space
  { 0, 0, 1, 1, 1, 0, 1 },  // o
  { 0, 1, 1, 1, 1, 1, 0 },  // V
  { 0, 1, 0, 0, 0, 0, 0 },  // up
  { 0, 0, 1, 0, 0, 0, 0 },  // down
  { 1, 1, 0, 0, 0, 0, 0 },  // fast up
  { 0, 0, 1, 1, 0, 0, 0 }   // fast down

};


void  Display_Row_Write_Symbol (byte row_index, byte symbol)
{
  if (symbol >= DISPLAY_SYMBOL_CNT) return;
  if (row_index >= (DISPLAY_ROW_INDEX_CNT - 7)) return;

  for (byte symbol_index = 0; symbol_index < 7; symbol_index++, row_index++)
  {
    display_row[row_index] = Display_Symbols[symbol][symbol_index];
  }

  display_row_changed = true;
}


// Arguments:
// * tube_index: 0..5 corresponding with tubes left->right.
// * digit: 0..9.

void  Display_Write_Tube_Digit (byte tube_index, byte digit)
{
  if (tube_index >= 6) return;
  if (digit >= 10) return;

  Display_Row_Write_Symbol(Display_Tube_Row_Index[tube_index],digit + DISPLAY_SYMBOL_0);
}


void  Display_Write_Tube_Symbol (byte tube_index, byte symbol)
{
  if (tube_index >= 6) return;
  if (symbol >= DISPLAY_SYMBOL_CNT) return;

  Display_Row_Write_Symbol(Display_Tube_Row_Index[tube_index],symbol);
}


void  Display_Write_Dot (byte dot_index, boolean state)
{
  if (dot_index >= 6) return;

  display_row[DISPLAY_ROW_INDEX_DOT1 + dot_index] = state ? 1 : 0;

  display_row_changed = true;
}


void  Display_Clear_All_Dots ()
{
  display_row[DISPLAY_ROW_INDEX_DOT1] = 0;
  display_row[DISPLAY_ROW_INDEX_DOT2] = 0;
  display_row[DISPLAY_ROW_INDEX_DOT3] = 0;
  display_row[DISPLAY_ROW_INDEX_DOT4] = 0;
  display_row[DISPLAY_ROW_INDEX_DOT5] = 0;
  display_row[DISPLAY_ROW_INDEX_DOT6] = 0;

  display_row_changed = true;
}


void  Display_Write_Sep_Left (boolean state)
{
  display_row[DISPLAY_ROW_INDEX_SEP1] = state ? 1 : 0;

  display_row_changed = true;
}


void  Display_Write_Sep_Right (boolean state)
{
  display_row[DISPLAY_ROW_INDEX_SEP2] = state ? 1 : 0;

  display_row_changed = true;
}


void  Display_Exec ()
{
  if (display_row_changed)
  {
    display_row_changed = false;

    Display_Render();
  }
}


void  Display_Init ()
{
  pinMode(PIN_CLK,OUTPUT);
  digitalWrite(PIN_CLK,LOW);

  pinMode(PIN_LE,OUTPUT);
  digitalWrite(PIN_LE,LOW);

  pinMode(PIN_STANDBY,OUTPUT);
  digitalWrite(PIN_STANDBY,LOW);

  pinMode(PIN_DATA,OUTPUT);

  io_delay();

  // Get a handle to the timer:
  // Arg. 1: Timer index.
  // Arg. 2: Base clock divider value.
  // Arg. 3: Count up (true) or down (false).
  // We want 1 us timer ticks. The base clock is 80 MHz, so divide by 80.
  display_timer = timerBegin(0,80,true);

  // Attach an interrupt service routine:
  // Arg. 1: Timer handle.
  // Arg. 2: Address of routine.
  // Arg. 3: Edge (true) or level (false) triggered.
  timerAttachInterrupt(display_timer,Display_Timer_ISR,true);

  // Set timer interval:
  // Arg. 1: Timer handle.
  // Arg. 2: Interval ticks.
  // Arg. 3: Reload (true) or don't reload (false).
  // The interval is specified as 1 us ticks, providing the clock divider is set correctly.
  timerAlarmWrite(display_timer,DISPLAY_TIMER_INTERVAL,true);

  // Enable the timer
  timerAlarmEnable(display_timer);

  // Trigger initial display rendering
  display_row_changed = true;
}
