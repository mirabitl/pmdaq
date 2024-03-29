#!/usr/bin/python3
import time
import os,sys

class abstractGenesys:
    def __init__(self,number):
        self.board=number
    
        
        self.setAddress({"address":self.board})
        #    time.sleep(1)
        #self.setRemote({"remote":1})
        #self.write("RMT %d\r" % 1)  

        #print self.readline()
    def setAddress(self,p):
        if not "address" in p:
            print("no address value in ",p)
            return
        
        self.write("ADR %d" % p["address"])
        rep=self.readline()
        #print(rep)
        return rep

    def setRemote(self,p):
        if not "remote" in p:
            print("no remote value in ",p)
            return
        flag=p["remote"]==1
        if (flag):
            self.write("RMT 1")
        else:
            self.write("RMT 0")
        rep=self.readline()
        #print(rep)
        return rep
        
    def remoteMode(self):
        self.write("RMT?")
        rep=self.readline()
        #print(rep)
        return int(rep[4:4])==1
    
    def model(self):
        self.write("IDN?")
        rep=self.readline()
        #print(rep)
        return rep
    
    def version(self):
        self.write("REV?")
        rep=self.readline()
        #print(rep)
        return rep
    
    def serialNumber(self):
        self.write("SN?")
        rep=self.readline()
        #print(rep)
        return rep
    
    def calibrationDate(self):
        self.write("DATE?")
        rep=self.readline()
        #print(rep)
        return rep
    
    def clear(self):
        self.write("CLS")
        rep=self.readline()
        #print(rep)
        return rep
    
    def reset(self):
        self.write("RST")
        rep=self.readline()
        #print(rep)
        return rep
        
    def setVoltage(self,p):
        if not "vset" in p:
            print("no vset value in ",p)
            return
        self.write("PV %.3f" % p["vset"] )
        rep=self.readline()
        #print(rep)
        return rep
    
    def vSet(self):
        self.write("PV?")
        rep=self.readline()
        #print(rep)
        return float(rep)
    
    def vOut(self):
        self.write("MV?")
        rep=self.readline()
        #print(rep)
        return float(rep)
    
    def setCurrent(self,p):
        if not "iset" in p:
            print("no iset value in ",p)
            return
        self.write("PC %.3f" % p["iset"] )
        rep=self.readline()
        #print(rep)
        return rep
    
    def iSet(self):
        self.write("PC?")
        rep=self.readline()
        #print(rep)
        return float(rep)
    
    def iOut(self):
        self.write("MC?")
        rep=self.readline()
        #print(rep)
        return float(rep)
    
    def setOff(self):
        self.write("OUT 0")
        rep=self.readline()
        #print(rep)
        return rep

        
    def setOn(self):
        self.write("OUT 1")
        rep=self.readline()
        #print(rep)
        return rep


    def isLvOn(self):
        self.write("OUT?")
        rep=self.readline()
        if (rep==None):
            return 0
        
        #print(rep,rep[0:2])
        if rep[0:2]=="ON":
            return 1
        else:
            return 0

    def status(self):
        rep={}
        self.write("STT?")
        self.value_read=self.readline()
        if (self.value_read==None):
            return rep
        tlen=len(self.value_read)
        ipv=self.value_read.find("PV(")+3
        lpv=self.value_read[ipv:ipv+8].find(")")

        rep["vset"]=float(self.value_read[ipv:ipv+lpv])
        imv=self.value_read.find("MV(")+3
        lmv=self.value_read[imv:imv+8].find(")")
        rep["vout"]=float(self.value_read[imv:imv+lmv])
        ipc=self.value_read.find("PC(")+3
        lpc=self.value_read[ipc:ipc+8].find(")")
        rep["iset"]=float(self.value_read[ipc:ipc+lpc])
        imc=self.value_read.find("MC(")+3
        lmc=self.value_read[imc:imc+8].find(")")
        rep["iout"]=float(self.value_read[imc:imc+lmc])
        rep["status"]=self.isLvOn()
        return rep
    
    def info(self):
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
        rep["status"]=self.isLvOn()
        return rep
