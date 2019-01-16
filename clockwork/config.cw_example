# this is how we can instantiate objects for the esp32 demo

esp32 ESP32; # CPU defined in the runtime support
gateway32 OLIMEX_GATEWAY32 esp32; # board defined in the runtime support

button INPUT gateway32, gateway32.BUT1;
led OUTPUT gateway32, gateway32.LED;
aout ANALOGOUTPUT gateway32, gateway32.GPIO16;
direction OUTPUT gateway32, gateway32.GPIO17;

pulser PULSE (delay:50) led;
ramp RAMP pulser, aout, direction;
d_button DONINPUT button;
speed_select SPEEDSELECT d_button, pulser;

