import pyvisa
import json
import os

CONFIG_FILE = "pulse_config.json"

def save_config(config):
    with open(CONFIG_FILE, "w") as f:
        json.dump(config, f, indent=4)
    print(f"✅ Configuration enregistrée dans {CONFIG_FILE}")

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, "r") as f:
            return json.load(f)
    else:
        return {

            "ip": "134.158.138.199",
            "voltage_low": 0,
            "voltage_high": 0.65,
            "width": 1e-07,
            "delay": 2e-7,
            "polarity": "INV",
            "rise_time": 2e-09,
            "fall_time": 2e-09,
            "trigger_mode": "EXT",
            "trigger_slope": "POS",
            "trigger_level": 0.7
        }

def connect(ip):
    rm = pyvisa.ResourceManager("@py")
    try:
        inst = rm.open_resource(f"TCPIP::{ip}::INSTR")
        print(f"🖧 Connecté à: {inst.query('*IDN?').strip()}")
        return inst
    except Exception as e:
        print(f"❌ Erreur de connexion : {e}")
        return None

def configure_pulse(inst, cfg):
    print("⚙️ Configuration de l'impulsion (mode burst, 1 pulse par trigger)...")
    inst.write("*RST")
    inst.write(":OUTPut1 OFF")   
    inst.write(":SOURce1:FUNCtion PULSe")
    inst.write(f":VOLTage1:LOW {cfg['voltage_low']}")
    inst.write(f":VOLTage1:HIGH {cfg['voltage_high']}")
    inst.write(f":SOURce1:PULSe:WIDTh {cfg['width']}")
    inst.write(f":SOURce1:PULSe:DElay {cfg['delay']}")
    inst.write(f":OUTPut1:POLarity {cfg['polarity']}")
    inst.write(f":SOURce1:PULSe:TRANsition:LEADing {cfg['rise_time']}")
    inst.write(f":SOURce1:PULSe:TRANsition:TRAiling {cfg['fall_time']}")
    inst.write(":SOURce1:BURSt:STATe ON")
    inst.write(":SOURce1:BURSt:NCYCles 1")
    


def configure_trigger(inst, cfg):
    print("🎯 Configuration du TRIGGER...")
    inst.write(f":ARM:SOURce1 {cfg['trigger_mode']}")
    inst.write(f":ARM:SLOPe1 {cfg.get('trigger_slope', 'POS')}")
    inst.write(f":ARM:LEVel1 {cfg.get('trigger_level', 1.0)}")

def print_status(inst):
    print("\n🔍 Lecture des paramètres actuels :")
    params = {
        # ~ "Freq (Hz)": inst.query(":SOURce1:FREQuency?").strip(),
        "Voltage low": inst.query(":VOLTage1:LOW?").strip(),
        "Voltage high": inst.query(":VOLTage1:HIGH?").strip(),
        "Width (s)": inst.query(":SOURce1:PULSe:WIDTh?").strip(),
        "Delay (s)": inst.query(":SOURce1:PULSe:DElay?").strip(),
        "Polarity": inst.query(":OUTPut1:POLarity?").strip(),
        "Rise Time (s)": inst.query(":SOURce1:PULSe:TRANsition:LEADing?").strip(),
        "Fall Time (s)": inst.query(":SOURce1:PULSe:TRANsition:TRAiling?").strip(),
        "Trigger Source": inst.query(":ARM:SOURce1?").strip(),
        "Trigger Slope": inst.query(":ARM:SLOPe1?").strip(),
        "Trigger Level": inst.query(":ARM:LEVel1?").strip(),
        "Output State": inst.query(":OUTPut1?").strip()
    }
    for label, value in params.items():
        print(f"  {label:20s} : {value}")

def main():
    config = load_config()
    inst = connect(config["ip"])
    if inst:
        print_status(inst)
        """
        configure_pulse(inst, config)
        configure_trigger(inst, config)
        # ~ save_config(config)
        print_status(inst)
        """
        inst.close()

if __name__ == "__main__":
    main()
