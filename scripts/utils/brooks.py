import time
import RPi.GPIO as GPIO
import serial
import hart_protocol as hp
import struct
import bparsing
class brooks:
    def __init__(self,portname="/dev/ttyAMA0",rst_pin=4,**kwargs):
        GPIO.cleanup()
        GPIO.setmode(GPIO.BCM)
        self.rst=rst_pin
        GPIO.setup(self.rst, GPIO.OUT)
        GPIO.cleanup()
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.rst, GPIO.OUT)

        self.port = serial.Serial(portname, baudrate=19200,parity='O',stopbits=1,timeout=0.1)
        self.unpacker = hp.Unpacker(self.port)
        # Get the address
        cmd=hp.universal.read_unique_identifier(0)
        self.writeCommand(cmd)
        mn=self.readAnswer()
        print(mn.full_response)
        self.l_address= hp.tools.calculate_long_address(mn.manufacturer_id,mn.manufacturer_device_type,mn.device_id.to_bytes(3, 'big'))
        print(self.l_address)
        print("Device found %d %d %d %x\n" % (mn.manufacturer_id,mn.manufacturer_device_type,mn.device_id,int.from_bytes(self.l_address,"big")))

    def read_primary(self):
        cmd=hp.universal.read_primary_variable(self.l_address)
        self.writeCommand(cmd)
        self.primary=self.readAnswer()
        print(self.primary)
        
    def read_set_point(self):
        cmd=hp.tools.pack_command(self.l_address, command_id=235)
        self.writeCommand(cmd)
        ra=self.readAnswer()
        self.read_setpoint=bparsing.bparse(ra.full_response)
        print(self.rea_setpoint)
        
    def write_set_point(self,percent):
        code = 57
        value = percent
        pdata = struct.pack(">Bf", code, value)
        #print(pdata)
        cmd = hp.tools.pack_command(self.l_address, command_id=236, data=pdata)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        self.setpoint=bparsing.bparse(rc.full_response)
        print(self.set_point)
        
    def read_gas_type(self,g_code):
        code = g_code
        pdata = struct.pack(">B")
        #print(pdata)
        cmd = hp.tools.pack_command(self.l_address, command_id=150, data=pdata)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        self.gas_type=bparsing.bparse(rc.full_response)
        print(self.gas_type)
        
    def read_gas_params(self,g_code):
        code = g_code
        pdata = struct.pack(">B")
        #print(pdata)
        cmd = hp.tools.pack_command(self.l_address, command_id=151, data=pdata)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        self.gas_params=bparsing.bparse(rc.full_response)
        print(self.gas_params)
        
    def writeCommand(self,cmd,tempo=0.01):
        #print("calling ",cmd)
        tmax=len(cmd)*10/19200.
        
        s=""
        for i in range(len(cmd)):
            s=s+"%.2x " % cmd[i]
        print("calling ",tmax,s)
        GPIO.output(self.rst,1);
        n=self.port.write(cmd);
        time.sleep(tmax*1.1);
        GPIO.output(self.rst,0);

    def readAnswer(self):
        time.sleep(0.2)
        for msg in self.unpacker:
            return (msg)	

def main():
    bkfm=brooks()
    v=input("Ok ?")
    bkfm.read_primary()
    v=input("Ok ?")
    bkfm.read_set_point()
    v=input("Set point (%%) ?")
    bkfm.write_set_point(float(v))
    v=input("Ok ?")
    
    bkfm.read_set_point()
    bkfm.read_primary()
    v=input("Ok ?")
    
if __name__ == "__main__":
    main()
