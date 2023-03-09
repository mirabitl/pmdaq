#!/usr/bin/python3
import time
import os,sys

class abstractGenesys:
    def __init__(self,number):
        self.board=number
    
        self.write("ADR%d" % self.board)

        #self.write("RMT %d\r" % 1)  

        #print self.readline()
    def setAddress(self,adr):
        self.write("ADR%d;" % adr)
        rep=self.readline()
        print(rep)
        return rep

    def setRemote(self,flag):
        if (flag):
            self.write("RMT1")
        else:
            self.write("RMT0")
        rep=self.readline()
        print(rep)
        return rep
        
    def remoteMode(self):
        self.write("RMT?")
        rep=self.readline()
        print(rep)
        return int(rep[4:4])==1
    
    def model(self):
        self.write("IDN?")
        rep=self.readline()
        print(rep)
        return rep
    
    def version(self):
        self.write("REV?")
        rep=self.readline()
        print(rep)
        return rep
    
    def serialNumber(self):
        self.write("SN?")
        rep=self.readline()
        print(rep)
        return rep
    
    def calibrationDate(self):
        self.write("DATE?")
        rep=self.readline()
        print(rep)
        return rep
    
    def clear(self):
        self.write("CLS")
        rep=self.readline()
        print(rep)
        return rep
    
    def reset(self):
        self.write("RST")
        rep=self.readline()
        print(rep)
        return rep
        
    def setVoltage(self,p):
        if not "vset" in p:
            print("no vset value in ",p)
            return
        self.write("PV %.3f" % p["vset"] )
        rep=self.readline()
        print(rep)
        return rep
    
    def vSet(self):
        self.write("PV?")
        rep=self.readline()
        print(rep)
        return float(rep)
    
    def vOut(self):
        self.write("MV?")
        rep=self.readline()
        print(rep)
        return float(rep[3:len(rep)-1])
    
    def setCurrent(self,p):
        if not "iset" in p:
            print("no iset value in ",p)
            return
        self.write("PC %.3f;" % p["iset"] )
        rep=self.readline()
        print(rep)
        return rep
    
    def iSet(self):
        self.write("PC?")
        rep=self.readline()
        print(rep)
        return float(rep)
    
    def iOut(self):
        self.write("MC?")
        rep=self.readline()
        print(rep)
        return float(rep)
    
    def setOff(self):
        self.write("OUT 0")
        rep=self.readline()
        print(rep)
        return rep

        
    def setOn(self):
        self.write("OUT 1")
        rep=self.readline()
        print(rep)
        return rep


    def isLvOn(self):
        self.write("OUT?")
        rep=self.readline()
        print(rep)
        return int(rep)==1
    
    def status(self):
        rep={}
        self.write("IDN?")
        rep["model"]=self.readline()
        self.write("REV?")
        rep["version"]=self.readline()
        self.write("DATE?")
        rep["date"]=self.readline()
        self.write("SN?")
        rep["serial"]=self.readline()
        self.write("PV?")
        rl=self.readline()
        rep["vset"]=float(rl)
        self.write("MV?")
        rl=self.readline()
        rep["vout"]=float(rl)
        self.write("PC?")
        rl=self.readline()
        rep["iset"]=float(rl)
        self.write("MC?")
        rl=self.readline()
        rep["iout"]=float(rl)
        self.write("OUT?")
        rep["status"]=int(rep)
        return rep
