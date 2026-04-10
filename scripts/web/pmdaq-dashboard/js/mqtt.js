const mqttData = {};

function initMQTT() {
  const client = mqtt.connect(CONFIG.MQTT_URL);

  client.on('connect', () => {
    console.log("MQTT connected");
    client.subscribe(CONFIG.MQTT_TOPIC);
  });

  client.on('message', (topic, message) => {
    try {
      mqttData[topic] = JSON.parse(message.toString());
      renderMQTT(); // ui.js
    } catch (e) {
      console.error("JSON error", e);
    }
  });

  client.on('error', console.error);
}
