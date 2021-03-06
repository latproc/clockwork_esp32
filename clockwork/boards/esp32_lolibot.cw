LOLIBOT BOARD cpu {
	StatusLED PIN cpu.GPIO22;
	NeoPixel PIN cpu.GPIO2;
#	NeoPixel_j2 NEOPIXEL NeoPixel, 1;
#	NeoPixel_j3 NEOPIXEL NeoPixel, 2;
#	NeoPixel_j4 NEOPIXEL NeoPixel, 3;

# J20 Expansion
	J20_PIN2 PIN cpu.GPIO33;
	J20_PIN4 PIN cpu.GPIO32;
	J20_PIN6 PIN cpu.GPIO35;
	J20_PIN7 PIN cpu.GPIO34;

# J21 Expansion
	J21_PIN2 PIN cpu.GPIO12;
	J21_PIN3 PIN cpu.GPIO25;
	J21_PIN4 PIN cpu.GPIO14;
	J21_PIN5 PIN cpu.GPIO26;
	J21_PIN6 PIN cpu.GPIO27;

# J11 Light Sensor
	J11_PIN3 PIN cpu.GPIO4;

# J3 Servo
	J3_PIN3 PIN cpu.GPIO16;

# U3 Right Motor
	U3_PIN7_IB PIN cpu.GPIO23;
	U3_PIN6_IA PIN cpu.GPIO5;
 
# U4 Left Motor
	U4_PIN7_IB PIN cpu.GPIO13;
	U4_PIN6_IA PIN cpu.GPIO15;

# J6 MPU-9250 9-Axis Gyroscope + Accelerometer + Magnetometer
	J6_SCL_PIN8 PIN cpu.VSPIQ;
	J6_SDA_PIN7 PIN cpu.VSPICLK;
	J6_INT_PIN3 PIN cpu.GPIO17;

}
