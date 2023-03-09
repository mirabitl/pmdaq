#!/usr/bin/python3
import time
import serial
import os,sys
from genesysInterface import abstractGenesys as gI
class genesysSerial(gI):
    def __init__(self,device,number):
        self.ser=serial.Serial(device)
        self.writer=serial.Serial(device)
        print("Serial readout on ",self.writer.portstr)       # check which port was really used
        self.writer.baudrate = 9600

        self.writer.bytesize = serial.EIGHTBITS #number of bits per bytes
        
        self.writer.parity = serial.PARITY_NONE #set parity check: no parity
        
        self.writer.stopbits = serial.STOPBITS_ONE #number of stop bits

        #self.dsrdtr=True

        #block read

        self.writer.timeout = 1            #non-block read, None to block
        # configure the serial connections (the parameters differs on the device you are connecting to)

        gI.__init__(self,number)
    def write(self,s):
        ss=s+"\r"
        print(ss)
        self.writer.flushInput()
        time.sleep(0.1)
        self.writer.write(ss.encode())
        time.sleep(0.3)
    def readline(self):
        nb=self.writer.in_waiting
        print("Bytes to read ",nb)
        # meme probleme avec self.writer.readline()
        r= self.writer.read(nb)
        #print(r)
        if (nb<1):
            return "no data"
        if (r[0]==0xFF):
            r=r[1:len(r)-1]
        print(len(r),r)
        s=""
        try:
            s=r.decode()
        except:
            s="Decode error"
        return s
