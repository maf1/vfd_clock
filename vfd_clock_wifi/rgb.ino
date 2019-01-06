
/*
  RGB driver
*/


boolean         rgb_suspended = false;
RGB_SOLID       rgb_solid;
RGB_GRAD        rgb_grad_set[3];
RGB_GRAD        rgb_grad_set_init[3];
unsigned long   rgb_ms;
RGB_METHOD      rgb_method;


RGB_SOLID  RGB_Solid_List[RGB_SOLID_ID_CNT] =
{
  { .red   = 1365, .green =    0, .blue =    0 },   // Dark red
  { .red   = 2730, .green =    0, .blue =    0 },   // Medium red
  { .red   = 4095, .green =    0, .blue =    0 },   // Light red
  { .red   = 1365, .green = 1365, .blue =    0 },   // Dark yellow
  { .red   = 2730, .green = 2730, .blue =    0 },   // Medium yellow
  { .red   = 4095, .green = 4095, .blue =    0 },   // Light yellow
  { .red   =    0, .green = 1365, .blue =    0 },   // Dark green
  { .red   =    0, .green = 2730, .blue =    0 },   // Medium green
  { .red   =    0, .green = 4095, .blue =    0 },   // Light green
  { .red   =    0, .green = 1365, .blue = 1365 },   // Dark cyan
  { .red   =    0, .green = 2730, .blue = 2730 },   // Medium cyan
  { .red   =    0, .green = 4095, .blue = 4095 },   // Light cyan
  { .red   =    0, .green =    0, .blue = 1365 },   // Dark blue
  { .red   =    0, .green =    0, .blue = 2730 },   // Medium blue
  { .red   =    0, .green =    0, .blue = 4095 },   // Light blue
  { .red   = 1365, .green =    0, .blue = 1365 },   // Dark magenta
  { .red   = 2730, .green =    0, .blue = 2730 },   // Medium magenta
  { .red   = 4095, .green =    0, .blue = 4095 }    // Light magenta
};


RGB_GRAD  RGB_Grad_Set_List[RGB_GRAD_SET_ID_CNT][3] =
{
  // Cycling through primary colors
  {
    { .val =     0, .add =  48, .left =  86, .cnt = 171 },
    { .val =  2730, .add = -48, .left = 142, .cnt = 171 },
    { .val = -2730, .add = -48, .left =  28, .cnt = 171 }
  },

  // Pastel colors
  {
    { .val =         2200, .add =  48, .left =  86, .cnt = 171 },
    { .val =  2730 + 2200, .add = -48, .left = 142, .cnt = 171 },
    { .val = -2730 + 2200, .add = -48, .left =  28, .cnt = 171 }
  },

  // Green colors, kind of Matrix theme
  {
      { .val =  1000, .add =  -7, .left = 100, .cnt = 100 },
      { .val =  4096, .add = -15, .left = 100, .cnt = 100 },
      { .val =  1000, .add =  -7, .left = 100, .cnt = 100 }
  },

  // Cycling around brown colors
  {
      { .val =  2720, .add =  12, .left =  25, .cnt =  50 },
      { .val =  1200, .add = -10, .left =  25, .cnt =  50 },
      { .val =     0, .add =   5, .left =  35, .cnt =  50 }
  },

  // Cyan en blue
  {
      { .val =     0, .add =   0, .left =  25, .cnt =  50 },
      { .val =  4095, .add = -45, .left = 117, .cnt = 117 },
      { .val =  4095, .add =   0, .left =  50, .cnt =  50 }
  },

  // Purple and pink
  {
      { .val =  2048, .add =  40, .left =  50, .cnt =  50 },
      { .val =     0, .add =  24, .left =  50, .cnt =  50 },
      { .val =  2048, .add = -45, .left =  50, .cnt =  50 }
  }
};


void  RGB_Grad_Write_Channel (RGB_GRAD *grad, byte channel)
{
  int16_t   val;

  // Derive the 12-bit color value (0..4095) from the 12-bit gradient value
  val = grad->val;
  if (val < 0) val = 0; else
  if (val > 4095) val = 4095;

  // Write 12-bit pulse width
  ledcWrite(channel,val);
}


void  RGB_Render ()
{
  if (rgb_method == RGB_METHOD_SOLID)
  {
    // Write 12-bit pulse width values
    ledcWrite(1,rgb_solid.red);
    ledcWrite(2,rgb_solid.green);
    ledcWrite(3,rgb_solid.blue);
  }
  else
  if (rgb_method == RGB_METHOD_GRAD)
  {
    RGB_Grad_Write_Channel(&rgb_grad_set[0],1);
    RGB_Grad_Write_Channel(&rgb_grad_set[1],2);
    RGB_Grad_Write_Channel(&rgb_grad_set[2],3);
  }
}


void  RGB_Start_Solid ()
{
  RGB_Render();
}


void  RGB_Select_Solid (RGB_SOLID *solid)
{
  rgb_method = RGB_METHOD_SOLID;
  memcpy(&rgb_solid,solid,sizeof(rgb_solid));

  if (!rgb_suspended) RGB_Start_Solid();
}


void  RGB_Select_Solid_Id (byte id)
{
  if (id >= RGB_SOLID_ID_CNT) return;

  RGB_Select_Solid(&RGB_Solid_List[id]);
}


void  RGB_Solid_Id_To_Settings (byte id)
{
  settings.rgb_solid.red   = RGB_Solid_List[id].red;
  settings.rgb_solid.green = RGB_Solid_List[id].green;
  settings.rgb_solid.blue  = RGB_Solid_List[id].blue;
}


void  RGB_Solid_To_Settings ()
{
  settings.rgb_solid.red   = rgb_solid.red;
  settings.rgb_solid.green = rgb_solid.green;
  settings.rgb_solid.blue  = rgb_solid.blue;
}


void  RGB_Solid_Id_To_Settings_Sleep (byte id)
{
  settings.rgb_sleep_solid.red   = RGB_Solid_List[id].red;
  settings.rgb_sleep_solid.green = RGB_Solid_List[id].green;
  settings.rgb_sleep_solid.blue  = RGB_Solid_List[id].blue;
}


void  RGB_Solid_To_Settings_Sleep ()
{
  settings.rgb_sleep_solid.red   = rgb_solid.red;
  settings.rgb_sleep_solid.green = rgb_solid.green;
  settings.rgb_sleep_solid.blue  = rgb_solid.blue;
}


void  RGB_Grad_Step (RGB_GRAD *grad)
{
  // Step the gradient value
  grad->val += grad->add;

  // Count
  grad->left--;
  if (grad->left == 0)
  {
      grad->add  = -grad->add;
      grad->left = grad->cnt;
  }
}


void  RGB_Start_Grad_Set ()
{
  rgb_ms = millis();

  RGB_Render();
}


void  RGB_Select_Grad_Set (RGB_GRAD *grad_set)
{
  rgb_method = RGB_METHOD_GRAD;
  memcpy(rgb_grad_set,grad_set,sizeof(rgb_grad_set));
  memcpy(rgb_grad_set_init,grad_set,sizeof(rgb_grad_set_init));

  if (!rgb_suspended) RGB_Start_Grad_Set();
}


void  RGB_Select_Grad_Set_Id (byte id)
{
  if (id >= RGB_GRAD_SET_ID_CNT) return;

  RGB_Select_Grad_Set(RGB_Grad_Set_List[id]);
}


void  RGB_Grad_Set_Id_To_Settings (byte id)
{
  memcpy(settings.rgb_grad,RGB_Grad_Set_List[id],sizeof(settings.rgb_grad));
}


void  RGB_Grad_Set_To_Settings ()
{
  memcpy(settings.rgb_grad,rgb_grad_set_init,sizeof(settings.rgb_grad));
}

boolean  RGB_Is_Suspended ()
{
  return rgb_suspended;
}


void  RGB_Suspend ()
{
#ifdef  RGB_DEBUG
  Serial.println("RGB_Suspend");
#endif

  if (!rgb_suspended)
  {
#ifdef  RGB_DEBUG
    Serial.println("-> suspending");
#endif

    rgb_suspended = true;

    // Write 12-bit pulse width values
    ledcWrite(1,settings.rgb_sleep_solid.red);
    ledcWrite(2,settings.rgb_sleep_solid.green);
    ledcWrite(3,settings.rgb_sleep_solid.blue);
  }
}


void  RGB_Resume ()
{
#ifdef  RGB_DEBUG
  Serial.println("RGB_Resume");
#endif

  if (rgb_suspended)
  {
#ifdef  RGB_DEBUG
    Serial.println("-> resuming");
#endif

    rgb_suspended = false;

    if (rgb_method == RGB_METHOD_SOLID) RGB_Start_Grad_Set(); else
    if (rgb_method == RGB_METHOD_GRAD)  RGB_Start_Solid();
  }
}


#define RGB_GRAD_DELTA_MS   100


void  RGB_Exec ()
{
  if (!rgb_suspended)
  {
    if (rgb_method == RGB_METHOD_GRAD)
    {
      if ((millis() - rgb_ms) > RGB_GRAD_DELTA_MS)
      {
        rgb_ms += RGB_GRAD_DELTA_MS;
    
        RGB_Grad_Step(&rgb_grad_set[0]);
        RGB_Grad_Step(&rgb_grad_set[1]);
        RGB_Grad_Step(&rgb_grad_set[2]);
    
        RGB_Render();
      }
    }
  }
}


void  RGB_Init ()
{
  // Assign RGB LED pins to channels
  ledcAttachPin(PIN_RED,  1);
  ledcAttachPin(PIN_GREEN,2);
  ledcAttachPin(PIN_BLUE, 3);

  // Set up RGB channels: 1200 Hz PWM, 12-bit resolution
  ledcSetup(1,1200,12);
  ledcSetup(2,1200,12);
  ledcSetup(3,1200,12);

  // Select initial RGB method and scheme from settings
  if (settings.rgb_method == RGB_METHOD_GRAD)
  {
    RGB_Select_Grad_Set(settings.rgb_grad);
  }
  else  // SOLID
  {
    RGB_Select_Solid(&settings.rgb_solid);
  }
}

