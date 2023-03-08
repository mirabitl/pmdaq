#!/usr/bin/python3
import time
import os,sys

class abstractZup:
    def __init__(self,number):
        self.board=number
    
        # configure the serial connections (the parameters differs on the device you are connecting to)


        self.write(":ADR%.2d" % self.board)

        #self.write("RMT %d\r" % 1)  

        #print self.readline()
    def setAddress(self,adr):
        self.write(":ADR%d;" % adr)
        print(self.readline())
        
    def setRemote(self,flag):
        if (flag):
            self.write(":RMT1;")
        else:
            self.write(":RMT0;")
        print(self.readline())
        
    def remoteMode(self):
        self.write(":RMT?;")
        rep=self.readline()
        print(rep)
        return int(rep[4:4])==1
    
    def model(self):
        self.write(":MDL?;")
        rep=self.readline()
        print(rep)
        return rep
    
    def version(self):
        self.write(":REV?;")
        rep=self.readline()
        print(rep)
        return rep
    
    def setVoltage(self,v):
        if not "vset" in p:
            print("no vset value in ",p)
            return

        self.write(":VOL%.3f;" % p["vset"] )
        rep=self.readline()
        print(rep)
        
    def vSet(self):
        self.write(":VOL!;")
        rep=self.readline()
        print(rep)
        return float(rep[3:len(rep)-1])
    
    def vOut(self):
        self.write(":VOL?;")
        rep=self.readline()
        print(rep)
        return float(rep[3:len(rep)-1])
    
    def setCurrent(self,p):
        if not "iset" in p:
            print("no iset value in ",p)
            return
        self.write(":CUR%.3f;" % p["iset"] )
        rep=self.readline()
        print(rep)
        
    def iSet(self):
        self.write(":CUR!;")
        rep=self.readline()
        print(rep)
        return float(rep[3:len(rep)-1])
    
    def iOut(self):
        self.write(":CUR?;")
        rep=self.readline()
        print(rep)
        return float(rep[3:len(rep)-1])
    
    def setOff(self):
        self.write(":OUT0;")
        print(self.readline())
        
    def setOn(self):
        self.write(":OUT1;")
        print(self.readline())

    def isLvOn(self):
        self.write(":OUT?;")
        rep=self.readline()
        print(rep)
        return int(rep[3:3])==1
    
    def status(self):
        rep={}
        self.write(":MDL?;")
        rep["model"]=self.readline()
        self.write(":REV?;")
        rep["version"]=self.readline()
        self.write(":VOL!;")
        rl=self.readline()
        rep["vset"]=float(rl[3:len(rl)-1])
        self.write(":VOL?;")
        rl=self.readline()
        rep["vout"]=float(rl[3:len(rl)-1])
        self.write(":CUR!;")
        rl=self.readline()
        rep["iset"]=float(rl[3:len(rl)-1])
        self.write(":CUR?;")
        rl=self.readline()
        rep["iout"]=float(rl[3:len(rl)-1])
        self.write(":OUT?;")
        rep["status"]=int(rep[3:3])
        return rep
