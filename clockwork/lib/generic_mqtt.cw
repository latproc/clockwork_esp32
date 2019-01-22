MQTT_FOLLOWER MACHINE MQTT, A_Output {
	update WHEN SELF IS idle && MQTT.message != A_Output.VALUE && TIMER > 100;
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

MQTT_AIN_LINK MACHINE r, g, b, out {
    OPTION last_r 0;
    OPTION last_g 0;
    OPTION last_b 0;
    update_r WHEN SELF IS idle AND last_r != r.message;
    update_g WHEN SELF IS idle AND last_g != g.message;
    update_b WHEN SELF IS idle AND last_b != b.message;
    sync WHEN SELF IS idle AND TIMER > 100;
    idle DEFAULT;
    ENTER update_r { 
        last_r := r.message;
        out.r := last_r / 8;
    }
    ENTER update_g { 
        last_g := g.message;
        out.g := last_g / 8;
    }
    ENTER update_b { 
        last_b := b.message;
        out.b := last_b / 8;
    }
    ENTER sync {
        out.r := last_r / 8;
        out.g := last_g / 8;
        out.b := last_b / 8;
    }
}
