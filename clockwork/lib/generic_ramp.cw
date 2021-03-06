# ramp - output an increasing analogue value up to a set point 
#  and then decrease the value back to a starting point

RAMP MACHINE clock, output, forward {
  OPTION start 1000;
  OPTION end 30000;
  OPTION direction 0;
  OPTION step 800;

  top WHEN output.VALUE >= end AND direction > 0;
  bottom_reverse WHEN output.VALUE <= start AND direction < 0 && forward IS off;
  bottom_forward WHEN output.VALUE <= start AND direction < 0 && forward IS on;
  rising WHEN output.VALUE < end AND direction > 0;
  falling WHEN output.VALUE > start AND direction < 0;
  stopped DEFAULT;
  ENTER top { output.VALUE := end; direction := -1;  }
  ENTER bottom_reverse { output.VALUE := start; direction := 1; SEND turnOn TO forward;}
  ENTER bottom_forward { output.VALUE := start; direction := 1; SEND turnOff TO forward;}

  ENTER INIT { output.VALUE := start; direction := 1; SEND turnOn TO forward; }

  RECEIVE clock.on_enter {
    output.VALUE := output.VALUE + direction * step;
  }
}

