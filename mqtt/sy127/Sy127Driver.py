import sys
import time

import serial # so we can talk over serial
import re


class SY127Parser:
    def __init__(self):
        self.last_frame = None
        self.pattern = re.compile(
            r"^(CH\d+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"([\d.]+)\s+"
            r"(\d+)\s+"
            r"(\w+)$")

    def clean(self, text):
        # remove ANSI escape codes
        text = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", text)
        text = text.replace("\r", "")
        return text

    def parse_line(self,line):
        m = self.pattern.match(line.strip())
        if not m:
            return None

        return {
            "channel": m.group(1),
            "VMON": float(m.group(2)),
            "IMON": float(m.group(3)),
            "V0": float(m.group(4)),
            "V1": float(m.group(5)),
            "I0": float(m.group(6)),
            "I1": float(m.group(7)),
            "RUP": float(m.group(8)),
            "RDW": float(m.group(9)),
            "TRIP": int(m.group(10)),
            "STATUS": m.group(11),
        }
    def parse_block(self,raw):
        clean = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", raw)

        result = {}

        for line in clean.split("\n"):
            m = self.pattern.match(line.strip())
            if m:
                result[m.group(1)] = {
                    "VMON": float(m.group(2)),
                    "IMON": float(m.group(3)),
                    "V0": float(m.group(4)),
                    "V1": float(m.group(5)),
                    "I0": float(m.group(6)),
                    "I1": float(m.group(7)),
                    "RUP": float(m.group(8)),
                    "RDW": float(m.group(9)),
                    "TRIP": int(m.group(10)),
                    "STATUS": m.group(11),
                }

        return result
        return channels

class Sy127Access:
    def __init__(self,device:str='/dev/ttyUSB0',baudrate:int=9600,timeout:int=2,mode=1):
        self.ser = serial.Serial(device,baudrate, timeout=timeout)
        self.ser.setDTR(False) #needed to keep data coming over serial without timeout     
        self.mode=mode
        print("Serial Line Open")
        self.parser = SY127Parser()

        time.sleep(0.15)                                                     
        self.ser.write("1".encode('utf-8')) #get to top menu  
        time.sleep(0.15)
        self.display()  
        print("Serial Data Sent")
        
    def display(self):
        raw = self.ser.read(8192).decode(errors="ignore")
        print(raw)
    def cratemap(self):
        time.sleep(0.15)
        self.ser.write("1".encode('utf-8'))
        time.sleep(0.15)
        self.ser.write("G".encode('utf-8'))
        time.sleep(0.15)
        self.display()
    def status(self,debug:bool=False):
        time.sleep(0.15)
        self.ser.write("1".encode('utf-8'))
        time.sleep(0.15)
        self.ser.write("A".encode('utf-8'))
        time.sleep(0.15)
        self.channels={}
        chread=True
        while chread:
            #print("reading")
            raw = self.ser.read(8192).decode(errors="ignore")
            #print(raw)
            pchannels = self.parser.parse_block(raw)

    
            for ch, data in pchannels.items():
                if not ch in self.channels.keys():
                    self.channels[ch]=data
                    #print(ch, data)
                else:
                    chread=False
                    break
            if chread:
                if self.mode==1:
                    self.ser.write("p".encode('utf-8')) #refresh params
                else:
                    self.ser.write("q".encode('utf-8')) #refresh params
        #print("\n \n",self.channels)
        print("\033[2J\033[H", end="")
        for ch, data in self.channels.items():
            print(ch, data)
    def write_utf8(self,c):
        self.ser.write(c.encode('utf-8'))
        time.sleep(0.25)
    def line_feed(self):
        self.ser.write("\r\n".encode('ascii'))
        time.sleep(0.25)
    def write_string(self,SV:str):
        values=list(SV)
        for k in range(0,len(values)):
            self.write_utf8(values[k])
        self.line_feed()
    def write_float(self,V:float):
        SV=f"{V:.1f}"
        self.write_string(SV)
    def write_int(self,V:int):
        SV=f"{V}"
        self.write_string(SV)
        
    def modify_channel(self,chname:str)->bool:
        if not chname in self.channels.keys():
            return False
        # Top Menu
        self.write_utf8('1')
        # Change
        self.write_utf8('B')
        # Single channel
        self.write_utf8('A')
        # Set Channel Name
        self.write_utf8('A')
        self.write_string(chname)
        return True
    def set_v0(self,chname:str,val:float):
        if not self.modify_channel(chname):
            return
        # Set V0
        print(f"Setting V0 {val} on channel {chname}") 
        self.write_utf8('C')
        self.write_float(val)
    def set_i0(self,chname:str,val:float):
        if not self.modify_channel(chname):
            return
        # Set I0
        print(f"Setting I0 {val} on channel {chname}") 
        self.write_utf8('F')
        self.write_float(val)
    def set_rup(self,chname:str,val:float):
        if not self.modify_channel(chname):
            return
        # Set V0
        print(f"Setting Ramp up {val} on channel {chname}") 
        self.write_utf8('I')
        self.write_float(val)
    def set_rdw(self,chname:str,val:float):
        if not self.modify_channel(chname):
            return
        # Set V0
        print(f"Setting Ramp down {val} on channel {chname}") 
        self.write_utf8('J')
        self.write_float(val)
        
    def toggle(self,chname:str):
        if not self.modify_channel(chname):
            return
        # Set V0
        print(f"Toggle ON/OFF channel {chname}")
        self.write_utf8('N')

if __name__ == '__main__':        
    crate=Sy127Access()
    #crate.cratemap()
    #time.sleep(5)
    crate.status(debug=True)
    print("\033[2J\033[H", end="")
    crate.set_v0("CH24",123.0)
    crate.set_i0("CH24",12.0)
    crate.set_rup("CH24",11)
    crate.set_rdw("CH24",33)
    crate.status(debug=True)
