LEDCONFIGURATION MACHINE {
  # default settings suit SK6812W RGBW LEDs
  # for WS2812, instantiate this as:
  # WS2812 LEDCONFIGURATION (bytesPerPixel:3, T0H:350, T1H:700, T0L:800, T1L:600, TRS:50000)
  OPTION bytesPerPixel 4;
  OPTION T0H 300;
  OPTION T1H 600;
  OPTION T0L 900;
  OPTION T1L 600;
  OPTION TRS 80000;
}

LEDSTRIP MACHINE out, led_type {
    OPTION channel 1;
    OPTION pin out;
    OPTION max_output 32;
    OPTION num_pixels 8;
}

DIGITALLED MACHINE strip, position {
    OPTION max 32;
    OPTION r 0;
    OPTION g 0;
    OPTION b 0;
    COMMAND black { r := 0; g := 0; b:= 0; }
    COMMAND white { r := max; g := max; b:= max; }
    COMMAND red { r := max; g := 0; b:= 0; }
    COMMAND green { r := max; g := max; b:= 0; }
    COMMAND blue { r := 0; g := 0; b:= max; }
}

