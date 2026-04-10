const mqttData = {};



function connectMQTT() {
  if (STATE.mqttClient) {
    STATE.mqttClient.end();
  }

  STATE.data = {};

  const topic = `pmdaq/${STATE.name}/#`;
  const client = mqtt.connect(CONFIG.MQTT_URL);

  STATE.mqttClient = client;

  client.on('connect', () => {
    console.log("MQTT connected");
    client.subscribe(topic);
  });

  client.on('message', (topic, message) => {
    try {
      const payload = JSON.parse(message.toString());
      parseTopic(topic, payload);
      renderMQTT(); // ui.js
    } catch (e) {
      console.error("MQTT parse error", e);
    }
  });

  client.on('error', console.error);
}


// -------- PARSING HIERARCHIQUE --------
function parseTopic(topic, payload) {
  const parts = topic.split('/');

  // pmdaq / name / app / instance / type
  if (parts.length < 5) return;

  const app = parts[2];
  const instance = parts[3];
  const type = parts[4];

  if (!STATE.data[app]) STATE.data[app] = {};
  if (!STATE.data[app][instance]) STATE.data[app][instance] = {};

  STATE.data[app][instance][type] = payload;
}




