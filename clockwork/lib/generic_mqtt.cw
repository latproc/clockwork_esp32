MQTT_FOLLER MACHINE MQTT, A_Output {
  update WHEN MQTT.message != A_Output.VALUE;
  idle DEFAULT;
  ENTER update { A_Output.VALUE := MQTT.message; }
}
