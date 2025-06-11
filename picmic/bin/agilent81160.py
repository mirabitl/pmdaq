import pyvisa
import json
import os

class mod81160:
    """ Handlers of thecsv objects per board
    """
    def __init__(self,config):
        self.config=config
        self.cfg=self.load_config()
        self.inst=self.connect()

    def load_config(self):
        if os.path.exists(self.config):
                with open(self.config, "r") as f:
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

  def connect(self):
    rm = pyvisa.ResourceManager("@py")
    try:
      self.inst = rm.open_resource(f'TCPIP::{self.self.cfg["ip"]}::INSTR')
      print(f"🖧 Connecté à: {self.inst.query('*IDN?').strip()}")
      return inst
    except Exception as e:
      print(f"❌ Erreur de connexion : {e}")
      return None
        

  def save_config(self,config_file):
    with open(config_file, "w") as f:
      json.dump(self.cfg, f, indent=4)
    print(f"✅ Configuration enregistrée dans {config_file}")


  def configure_pulse(self):
    print("⚙️ Configuration de l'impulsion (mode burst, 1 pulse par trigger)...")
    self.inst.write("*RST")
    self.inst.write(":OUTPut1 OFF")   
    self.inst.write(":SOURce1:FUNCtion PULSe")
    self.inst.write(f":VOLTage1:LOW {self.cfg['voltage_low']}")
    self.inst.write(f":VOLTage1:HIGH {self.cfg['voltage_high']}")
    self.inst.write(f":SOURce1:PULSe:WIDTh {self.cfg['width']}")
    self.inst.write(f":SOURce1:PULSe:DElay {self.cfg['delay']}")
    self.inst.write(f":OUTPut1:POLarity {self.cfg['polarity']}")
    self.inst.write(f":SOURce1:PULSe:TRANsition:LEADing {self.cfg['rise_time']}")
    self.inst.write(f":SOURce1:PULSe:TRANsition:TRAiling {self.cfg['fall_time']}")
    self.inst.write(":SOURce1:BURSt:STATe ON")
    self.inst.write(":SOURce1:BURSt:NCYCles 1")
    


  def configure_trigger(self):
    print("🎯 Configuration du TRIGGER...")
    self.inst.write(f":ARM:SOURce1 {self.cfg['trigger_mode']}")
    self.inst.write(f":ARM:SLOPe1 {cfg.get('trigger_slope', 'POS')}")
    self.inst.write(f":ARM:LEVel1 {cfg.get('trigger_level', 1.0)}")

  def print_status(self):
    print("\n🔍 Lecture des paramètres actuels :")
    params = {
      # ~ "Freq (Hz)": self.inst.query(":SOURce1:FREQuency?").strip(),
      "Voltage low": self.inst.query(":VOLTage1:LOW?").strip(),
      "Voltage high": self.inst.query(":VOLTage1:HIGH?").strip(),
      "Width (s)": self.inst.query(":SOURce1:PULSe:WIDTh?").strip(),
      "Delay (s)": self.inst.query(":SOURce1:PULSe:DElay?").strip(),
      "Polarity": self.inst.query(":OUTPut1:POLarity?").strip(),
      "Rise Time (s)": self.inst.query(":SOURce1:PULSe:TRANsition:LEADing?").strip(),
      "Fall Time (s)": self.inst.query(":SOURce1:PULSe:TRANsition:TRAiling?").strip(),
      "Trigger Source": self.inst.query(":ARM:SOURce1?").strip(),
      "Trigger Slope": self.inst.query(":ARM:SLOPe1?").strip(),
      "Trigger Level": self.inst.query(":ARM:LEVel1?").strip(),
      "Output State": self.inst.query(":OUTPut1?").strip()
    }
    for label, value in params.items():
      print(f"  {label:20s} : {value}")
  def close(self):
    if self.inst:
      self.inst.close()
def main():
    agp=mod81160("../etc/pulse_config.json")
    if agp.inst:
        agp.print_status()
        """
        configure_pulse(inst, config)
        configure_trigger(inst, config)
        # ~ save_config(config)
        print_status(inst)
        """
        agp.close()

if __name__ == "__main__":
    main()
