MQTT_FOLLER MACHINE MQTT, A_Output {
	update WHEN MQTT.message != A_Output.VALUE;
	idle DEFAULT;
	ENTER update { A_Output.VALUE := MQTT.message; }
}

MQTT_MONITOR MACHINE MQTT, A_Input {
	OPTION rate 500;
	update WHEN SELF IS idle && MQTT.message != A_Input.VALUE && TIMER >= rate;
	idle DEFAULT;
	idle INITIAL;

	ENTER update { MQTT.message := A_Input.VALUE; }
}

MQTT_INPUT_FOLLOWER MACHINE MQTT, Output {
	OPTION last 0;
	on WHEN MQTT.message == 1;
	off DEFAULT;
	ENTER off { SEND turnOn TO Output; }
	ENTER on { SEND turnOff TO Output; }
}
