#!/usr/bin/python3
import time
import serial
import os,sys
import zup
class zupSerial(zup.zup):
    def __init__(self,device,number):
        self.writer=serial.Serial(device)
        print("Serial readout on ",self.writer.portstr)       # check which port was really used
        self.writer.baudrate = 9600

        self.writer.bytesize = serial.EIGHTBITS #number of bits per bytes
        
        self.writer.parity = serial.PARITY_NONE #set parity check: no parity
        
        self.writer.stopbits = serial.STOPBITS_ONE #number of stop bits

        #block read

        self.writer.timeout = 1            #non-block read, None to block
        

        zup.zup.__init__(self,self.number)
    def write(self,s):
        self.writer.write(s)
    def readline(self):
        return self.writer.readline()
