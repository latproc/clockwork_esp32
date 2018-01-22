# Clockwork for the ESP32

This is a project repository for the esp-idf build system
at https://github.com/espressif/esp-idf.git

Currently this implements the basic framework for a runtine
environment for running clockwork programs within a FreeRTOS
environment. This framework provides for 

* defining clockwork MACHINEs using the C language
* scheduling MACHINEs to sleep and wake up at a given future time
* executing clockwork actions over many processing cycles
* reading ESP32 digital input to cause state changes of a clockwork MACHINE
* changing ESP32 digital outputs based on state changes of a clockwork MACHINE

This is a long way from being a complete system. Ultimately, the 
clockwork language interpreter (cw) will provide an automatic
export of a program into code that can be built using this framework.

