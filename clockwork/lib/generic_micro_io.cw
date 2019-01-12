DIGITALIO MACHINE pin { idle INITIAL; }
DIGITALIN MACHINE pin { idle INITIAL; }
ADC MACHINE pin { idle INITIAL; }
DAC MACHINE pin { idle INITIAL; }
PIN MACHINE pin { idle INITIAL; }
CPU MACHINE { idle INITIAL; }
BOARD MACHINE cpu { idle INITIAL; }
ETHERNET MACHINE cpu { idle INITIAL; }
UART MACHINE cpu { idle INITIAL; }
SPI MACHINE cpu { idle INITIAL; }
SDIO MACHINE cpu { idle INITIAL; }
TOUCH MACHINE cpu { idle INITIAL; }
JTAG MACHINE cpu { idle INITIAL; }

INPUT MACHINE module, item {
  off INITIAL;
  on STATE;
  COMMAND turnOn { SET SELF TO on; }
  COMMAND turnOff { SET SELF TO off; }
}

OUTPUT MACHINE module, item {
  off INITIAL;
  on STATE;
  COMMAND turnOn { SET SELF TO on; }
  COMMAND turnOff { SET SELF TO off; }
}
