#!/usr/bin/python3
import time
import serial
import os,sys

from zupInterface import abstractZup as zI

class zupSerial(zI):
    def __init__(self,device,number):
        self.writer=serial.Serial(device)
        print("Serial readout on ",self.writer.portstr)       # check which port was really used
        self.writer.baudrate = 9600

        self.writer.bytesize = serial.EIGHTBITS #number of bits per bytes
        
        self.writer.parity = serial.PARITY_NONE #set parity check: no parity
        
        self.writer.stopbits = serial.STOPBITS_ONE #number of stop bits

        #block read

        self.writer.timeout = 1            #non-block read, None to block
        

        zI.__init__(self,self.number)
    def write(self,s):
        self.writer.write(s)
    def readline(self):
        return self.writer.readline()
