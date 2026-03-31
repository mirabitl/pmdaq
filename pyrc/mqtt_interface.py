import threading
import time
import json
import queue
import paho.mqtt.client as mqtt


class MQTTInterface:
    def __init__(
        self,
        host="localhost",
        port=1883,
        root_topic="racine/#",
        client_id=None,
        use_json=True,
        auto_reconnect=True,
    ):
        self.host = host
        self.port = port
        self.root_topic = root_topic
        self.use_json = use_json
        self.auto_reconnect = auto_reconnect

        self.client = mqtt.Client(client_id=client_id)

        # état interne
        self._lock = threading.Lock()
        self._data = {}          # cache dernier message par topic
        self._tree = {}          # structure hiérarchique
        self._queue = queue.Queue()  # event stream

        self._connected = False
        self._running = False

        # callbacks
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        
        #Configuration to update
        self.app_config=None
    # -------------------------
    # MQTT CALLBACKS
    # -------------------------
    def _on_connect(self, client, userdata, flags, rc):
        self._connected = True
        print(f"[MQTT] connecté (rc={rc})")

        client.subscribe(self.root_topic)

    def _on_disconnect(self, client, userdata, rc):
        self._connected = False
        print(f"[MQTT] déconnecté (rc={rc})")

        if self.auto_reconnect and self._running:
            self._reconnect_loop()

    def _on_message(self, client, userdata, msg):
        raw_payload = msg.payload.decode()

        payload = raw_payload
        #print("MQTT->",msg.topic,msg.payload)
        # Decode PMDAQ messages and update config
        if self.app_config:        
            v=msg.topic.split("/")
            if (v[0]=="pmdaq" and len(v)==5):
                with self._lock:
                    [role,session,name,instance,mtype]=v
                    print("MQTT pmdaq-> ",role,session,name,instance,mtype)
                    the_app=next((d for d in self.app_config.apps if (d.name == name) and  (d.instance == int(instance)) ), None)
                    if the_app:
                        o_payload=json.loads(raw_payload)
                        #print("MQTT pmdaq-> ",o_payload)
                        if mtype=="info":
                            the_app.info= o_payload
                            # creer les listes commands,allowed and transitions
                            the_app.commands=[]
                            the_app.allowed=[]
                            the_app.transitions=[]
                            for x in the_app.info['COMMANDS']:
                                v=x.split("/")
                                the_app.commands.append(v[len(v)-1])
                            for x in the_app.info['TRANSITIONS']:
                                v=x.split("/")
                                the_app.transitions.append(v[len(v)-1])
                            for x in the_app.info['ALLOWED']:
                                v=x.split("/")
                                the_app.allowed.append(v[len(v)-1])
                        if mtype=="params":
                            the_app.params= o_payload
                        if mtype=="state":
                            the_app.state=o_payload["value"]
                        if mtype=="status":
                            the_app.status= o_payload
            elif (v[0]=="pmdaq" and v[2]=="rc"):
                with self._lock:
                    [role,session,name,mtype]=v
                    if mtype=="state":                        
                        self.app_config.state=payload
        """ 
        # Pour l'instant on ne gere que les messages PMDAQ pour mettre à jour la config, mais on pourrait aussi les publier dans le cache et la queue d'événements                
        if self.use_json:
            try:
                payload = json.loads(raw_payload)
            except Exception:
                pass

        with self._lock:
            # cache simple
            self._data[msg.topic] = payload

            # structure hiérarchique
            self._update_tree(msg.topic, payload)

        # push événement
        self._queue.put((msg.topic, payload))
        """
    # -------------------------
    # INTERNALS
    # -------------------------
    def _update_tree(self, topic, payload):
        parts = topic.split("/")
        node = self._tree

        for p in parts[:-1]:
            node = node.setdefault(p, {})

        node[parts[-1]] = payload

    def _reconnect_loop(self):
        print("[MQTT] tentative reconnexion...")
        while self._running and not self._connected:
            try:
                self.client.reconnect()
                return
            except Exception:
                time.sleep(2)

    # -------------------------
    # API PUBLIQUE
    # -------------------------
    def start(self,app_config):
        self.app_config=app_config
        self._running = True
        self.client.connect(self.host, self.port, 60)
        self.client.loop_start()

    def stop(self):
        self._running = False
        self.client.loop_stop()
        self.client.disconnect()

    def publish(self, topic, payload, qos=0, retain=False):
        if isinstance(payload, (dict, list)):
            payload = json.dumps(payload)

        self.client.publish(topic, payload, qos=qos, retain=retain)

    # --- accès données ---
    def get(self, topic, default=None):
        with self._lock:
            return self._data.get(topic, default)

    def get_tree(self):
        with self._lock:
            return dict(self._tree)

    # --- event-driven ---
    def get_event(self, timeout=None):
        try:
            return self._queue.get(timeout=timeout)
        except queue.Empty:
            return None

    def wait_for(self, topic, timeout=5):
        start = time.time()

        while time.time() - start < timeout:
            with self._lock:
                if topic in self._data:
                    return self._data[topic]
            time.sleep(0.05)

        return None
