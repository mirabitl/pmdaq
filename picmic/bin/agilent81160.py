import pyvisa
import json
import os


class mod81160:
    """Handlers of thecsv objects per board"""

    def __init__(self, config):
        self.config = config
        self.cfg = self.load_config()
        self.connect()

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
                "trigger_level": 0.7,
            }

    def connect(self):
        rm = pyvisa.ResourceManager("@py")
        try:
            self.inst = rm.open_resource(f'TCPIP::{self.cfg["ip"]}::INSTR')
            print(f"🖧 Connecté à: {self.inst.query('*IDN?').strip()}")
            return 
        except Exception as e:
            print(f"❌ Erreur de connexion : {e}")
            self.inst=None
            return 

    def save_config(self, config_file):
        with open(config_file, "w") as f:
            json.dump(self.cfg, f, indent=4)
        print(f"✅ Configuration enregistrée dans {config_file}")

    def configure_pulse(self):
        print("⚙️ Configuration de l'impulsion (mode burst, 1 pulse par trigger)...")
        self.inst.write("*RST")
        self.inst.write(":OUTPut1 OFF")
        self.inst.write(":SOURce:FUNCtion1 PULSe")
        self.inst.write(f":VOLTage1:LOW {self.cfg['voltage_low']}")
        self.inst.write(f":VOLTage1:HIGH {self.cfg['voltage_high']}")
        self.inst.write(f":SOURce1:PULSe:WIDTh {self.cfg['width']}")
        self.inst.write(f":SOURce1:PULSe:DElay {self.cfg['delay']}")
        self.inst.write(f":OUTPut1:POLarity {self.cfg['polarity']}")
        self.inst.write(f":SOURce1:PULSe:TRANsition:LEADing {self.cfg['rise_time']}")
        self.inst.write(f":SOURce1:PULSe:TRANsition:TRAiling {self.cfg['fall_time']}")
        self.inst.write(":SOURce1:BURSt:STATe ON")
        self.inst.write(":SOURce1:BURSt:NCYCles 1")
        
        self.inst.write(":OUTPut2 OFF")
        self.inst.write(":SOURce:FUNCtion2 PULSe")
        self.inst.write(f":VOLTage2:LOW 0")
        self.inst.write(f":VOLTage2:HIGH 3.3")
        #self.inst.write(f":SOURce:PULSe:WIDTh2 5.6e-8")
        self.inst.write(f":FUNC2:PULSe:WIDTh  5.9e-8")

        self.inst.write(f":FUNC2:PULSe:DElay 100.0e-9")
        self.inst.write(f":OUTPut2:POLarity NORM")
        self.inst.write(f":FUNC2:PULSe:TRANsition:LEADing 1.0e-9")
        self.inst.write(f":FUNC2:PULSe:TRANsition:TRAiling 1.0e-9")
        self.inst.write(":SOURce:BURSt:STATe2 ON")
        self.inst.write(":SOURce:BURSt:NCYCles2 1")
        
    def setVoltage(self,channel,low,high):
        self.inst.write(f":VOLTage{channel}:LOW {low}")
        self.inst.write(f":VOLTage{channel}:HIGH {high}")

    def setON(self,channel):
        self.inst.write(f":OUTPut{channel} ON")
        
    def setOFF(self,channel):
        self.inst.write(f":OUTPut{channel} OFF")
        
    def setRiseTime(self,channel,rise_time):
        self.inst.write(f":SOURce{channel}:PULSe:TRANsition:LEADing {rise_time}")
    
    def setFallTime(self,channel,fall_time):
        self.inst.write(f":SOURce{channel}:PULSe:TRANsition:TRAiling {fall_time}")

    def setDelay(self,channel,delay):
        self.inst.write(f":SOURce{channel}:PULSe:DElay {delay}")
    def configure_trigger(self):
        print("🎯 Configuration du TRIGGER...")
        self.inst.write(f":ARM:SOURce1 {self.cfg['trigger_mode']}")
        self.inst.write(f":ARM:SLOPe1 {self.cfg.get('trigger_slope', 'POS')}")
        self.inst.write(f":ARM:LEVel1 {self.cfg.get('trigger_level', 1.0)}")
        self.inst.write(f":ARM:SOURce2 {self.cfg['trigger_mode']}")
        self.inst.write(f":ARM:SLOPe2 {self.cfg.get('trigger_slope', 'POS')}")
        self.inst.write(f":ARM:LEVel2 {self.cfg.get('trigger_level', 1.0)}")
    def print_status(self):
        print("\n🔍 Lecture des paramètres actuels :")
        params = {
            # ~ "Freq (Hz)": self.inst.query(":SOURce1:FREQuency?").strip(),
            "Voltage low": self.inst.query(":VOLTage1:LOW?").strip(),
            "Voltage high": self.inst.query(":VOLTage1:HIGH?").strip(),
            "Width (s)": self.inst.query(":SOURce1:PULSe:WIDTh?").strip(),
            "Delay (s)": self.inst.query(":SOURce1:PULSe:DElay?").strip(),
            "Polarity": self.inst.query(":OUTPut1:POLarity?").strip(),
            "Rise Time (s)": self.inst.query(
                ":SOURce1:PULSe:TRANsition:LEADing?"
            ).strip(),
            "Fall Time (s)": self.inst.query(
                ":SOURce1:PULSe:TRANsition:TRAiling?"
            ).strip(),
            "Voltage low2": self.inst.query(":VOLTage2:LOW?").strip(),
            "Voltage high2": self.inst.query(":VOLTage2:HIGH?").strip(),
            #"Width2 (s)": self.inst.query(":SOURce2:PULSe:WIDTh?").strip(),
            #"Delay2 (s)": self.inst.query(":SOURce2:PULSe:DElay?").strip(),
            "Polarity2": self.inst.query(":OUTPut2:POLarity?").strip(),
            #"Rise Time2 (s)": self.inst.query(
            #    ":SOURce2:PULSe:TRANsition:LEADing?"
            #).strip(),
            #"Fall Time2 (s)": self.inst.query(
            #    ":SOURce2:PULSe:TRANsition:TRAiling?"
            #).strip(),
            "Trigger Source1": self.inst.query(":ARM:SOURce1?").strip(),
            "Trigger Slope1": self.inst.query(":ARM:SLOPe1?").strip(),
            "Trigger Level1": self.inst.query(":ARM:LEVel1?").strip(),
            "Output State1": self.inst.query(":OUTPut1?").strip(),
            "Trigger Source2": self.inst.query(":ARM:SOURce2?").strip(),
            "Trigger Slope2": self.inst.query(":ARM:SLOPe2?").strip(),
            #"Trigger Level2": self.inst.query(":ARM:LEVel2?").strip(),
            "Output State2": self.inst.query(":OUTPut2?").strip(),
        }
        for label, value in params.items():
            print(f"  {label:20s} : {value}")

    def close(self):
        if self.inst:
            self.inst.close()


def main():
    agp = mod81160("../etc/pulse_config.json")
    #agp.print_status()
    if agp.inst!=None:
        #agp.print_status()
        agp.configure_pulse()
        agp.configure_trigger()
        #agp.setON(1)
        agp.print_status()
        agp.close()


if __name__ == "__main__":
    main()
