ALWAYSOFF MACHINE out {

  fix WHEN out IS on;
  idle DEFAULT;

  ENTER fix { SEND turnOff TO out; }
  ENTER INIT { SEND turnOff TO out; }
}

ALWAYSON MACHINE out {

  fix WHEN out IS off;
  idle DEFAULT;

  ENTER fix { SEND turnOn TO out; }
  ENTER INIT { SEND turnOn TO out; }
}

PULSE MACHINE out {
  OPTION delay 100;

  on WHEN SELF IS off AND TIMER >= delay;
  off DEFAULT;

  ENTER on { LOG " on"; SEND turnOn TO out; }
  ENTER off { LOG " off"; SEND turnOff TO out; }

  RECEIVE toggle_speed { delay := 1100 - delay; }
}
