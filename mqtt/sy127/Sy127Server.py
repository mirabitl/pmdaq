import json
import time
import threading
import queue

import paho.mqtt.client as mqtt
import Sy127Driver

class SY127MQTTBridge:
    def __init__(self, crate, broker="localhost", port=1883):
        self.crate = crate

        self.lock = threading.Lock()   # 🔥 protection série
        self.cmd_queue = queue.Queue()

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        self.client.connect(broker, port, 60)

    # ---------------- MQTT ----------------

    def on_connect(self, client, userdata, flags, rc):
        print("MQTT connected")
        client.subscribe("sy127/cmd")

    def on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode())
            self.cmd_queue.put(payload)
        except Exception as e:
            print("MQTT parse error:", e)

    def publish_status(self, status):
        self.client.publish("sy127/status", json.dumps(status), retain=False)

    # ---------------- Worker threads ----------------

    def status_loop(self):
        while True:
            try:
                with self.lock:
                    self.crate.status()
                    status = self.crate.channels.copy()

                self.publish_status(status)

            except Exception as e:
                print("Status error:", e)

            time.sleep(15)  # 🔥 SY127 slow

    def command_loop(self):
        while True:
            cmd = self.cmd_queue.get()

            try:
                with self.lock:
                    self.execute_command(cmd)
            except Exception as e:
                print("Command error:", e)

    # ---------------- Command dispatcher ----------------

    def execute_command(self, cmd):
        ctype = cmd.get("cmd")
        ch = cmd.get("channel")
        val = cmd.get("value")

        if ctype == "set_v0":
            self.crate.set_v0(ch, float(val))

        elif ctype == "set_i0":
            self.crate.set_i0(ch, float(val))

        elif ctype == "set_rup":
            self.crate.set_rup(ch, float(val))

        elif ctype == "set_rdw":
            self.crate.set_rdw(ch, float(val))

        elif ctype == "toggle":
            self.crate.toggle(ch)

        else:
            print("Unknown command:", ctype)

    # ---------------- Run ----------------

    def run(self):
        threading.Thread(target=self.status_loop, daemon=True).start()
        threading.Thread(target=self.command_loop, daemon=True).start()

        self.client.loop_forever()

if __name__ == '__main__':
    crate = Sy127Driver.Sy127Access(mode=0)
    bridge = SY127MQTTBridge(crate, broker="lyoilcdaq01")
    bridge.run()
