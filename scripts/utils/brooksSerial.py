import time
import RPi.GPIO as GPIO
import serial

from brooksInterface import abstractBrooks as bI
#import bparsing

class brooksSerial(bI):
    def __init__(self,device_id=0,portname="/dev/ttyAMA0",rst_pin=4,**kwargs):
        #GPIO.cleanup()
        GPIO.setwarnings(False) 
        GPIO.setmode(GPIO.BCM)
        self.rst=rst_pin
        GPIO.setup(self.rst, GPIO.OUT)
        GPIO.cleanup()
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.rst, GPIO.OUT)

        self.port = serial.Serial(portname, baudrate=19200,parity='O',stopbits=1,timeout=0.1)
        bI.__init__(self,device_id)
    def writeCommand(self,cmd,tempo=0.01):
        #print("calling ",cmd)
        tmax=len(cmd)*10/19200.
        if (self.DEBUG):
            s=""
            for i in range(len(cmd)):
                s=s+"%.2x " % cmd[i]
        
            print("calling ",tmax,s)
        
        GPIO.output(self.rst,1);
        n=self.port.write(cmd);
        time.sleep(tmax*1.1);
        GPIO.output(self.rst,0);

    def read_one_byte(self):
        return self.port.read()
    
