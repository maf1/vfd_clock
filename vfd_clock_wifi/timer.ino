/*
 * Helper functions to implement timers using the revolving unisinged int counter
 */


void Timer_Start( TIMER &t, unsigned long duration, unsigned long *now )
{
  t.start      = now ? *now : millis();
  t.end        = t.start + duration;
  t.isRunning  = true;
}


void Timer_Restart( TIMER &t, unsigned long increment )
{
  t.start      = t.end;
  t.end        = t.start + increment;
  t.isRunning  = true;
}


void Timer_Clear( TIMER &t, unsigned long *now )
{
  t.end        = now ? *now : millis();
  t.isRunning  = false;
}


boolean Timer_IsRunning( TIMER &t, unsigned long *now )
{
  unsigned long ref = now ? *now : millis();
  
  if( t.isRunning ) {
    if( t.start<=t.end )
      t.isRunning = (ref>=t.start) && (ref<t.end);
    else
      t.isRunning = (ref>=t.start) || (ref<t.end);
  }

  return t.isRunning;
}


unsigned long Timer_GetResidual( TIMER &t, unsigned long *now )
{
  unsigned long ref = now ? *now : millis();
  
  if( !Timer_IsRunning(t,&ref) )
    return 0;
    
  if( t.start<=t.end || ref<=t.end )
    return t.end - ref;
  else 
    return t.end + (0xFFFFFFFF-ref);
}
