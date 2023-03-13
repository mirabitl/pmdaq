#!/usr/bin/python3
import time
import os,sys

class abstractZup:
    def __init__(self,number):
        self.board=number
    
        # configure the serial connections (the parameters differs on the device you are connecting to)


        self.setAddress({"address":self.board})

        self.setRemote({"remote":1})
        
        #self.write("RMT %d\r" % 1)  

        #print self.readline()
    def setAddress(self,p):
        if not "address" in p:
            print("no address value in ",p)
            return
        self.write(":ADR%.2d;" % p"[address"])
        rep=self.readline()
        #print(rep)
        return rep
        

    def setRemote(self,p):
        if not "remote" in p:
            print("no remote value in ",p)
            return
        flag=p["remote"]==1

        if (flag):
            self.write(":RMT1;")
        else:
            self.write(":RMT0;")
        print(self.readline())
        
    def remoteMode(self):
        self.write(":RMT?;")
        rep=self.readline()
        #print(rep)
        return int(rep[4:4])==1
    
    def model(self):
        self.write(":MDL?;")
        rep=self.readline()
        #print(rep)
        return rep
    
    def version(self):
        self.write(":REV?;")
        rep=self.readline()
        #print(rep)
        return rep
    
    def setVoltage(self,p):
        if not "vset" in p:
            print("no vset value in ",p)
            return

        self.write(":VOL%.3f;" % p["vset"] )
        rep=self.readline()
        #print(rep)
        return rep
    def vSet(self):
        self.write(":VOL!;")
        rep=self.readline()
        #print(rep)
        return float(rep[3:len(rep)-1])
    
    def vOut(self):
        self.write(":VOL?;")
        rep=self.readline()
        print(rep)
        return float(rep[2:len(rep)-1])
    
    def setCurrent(self,p):
        if not "iset" in p:
            print("no iset value in ",p)
            return
        self.write(":CUR%.3f;" % p["iset"] )
        rep=self.readline()
        #print(rep)
        return rep
    def iSet(self):
        self.write(":CUR!;")
        rep=self.readline()
        #print(rep)
        return float(rep[2:len(rep)-1])
    
    def iOut(self):
        self.write(":CUR?;")
        rep=self.readline()
        #print(rep)
        return float(rep[2:len(rep)-1])
    
    def setOff(self):
        self.write(":OUT0;")
        rep=self.readline()
        return rep
        
    def setOn(self):
        self.write(":OUT1;")
        rep=self.readline()
        return rep

    def isLvOn(self):
        self.write(":OUT?;")
        rep=self.readline()
        #print(rep)
        return int(rep[2:3])==1
    
    def status(self):
        rep={}
        self.write(":STT?;")
        self.value_read=self.readline()
        tlen=len(self.value_read)
        ipv=self.value_read.find("SV")+2
        rep["vset"]=float(self.value_read[ipv:ipv+5])
        imv=self.value_read.find("AV")+2
        rep["vout"]=float(self.value_read[imv:imv+5])
        ipc=self.value_read.find("SA")+2
        rep["iset"]=float(self.value_read[ipc:ipc+5])
        imc=self.value_read.find("AA")+2
        rep["iout"]=float(self.value_read[imc:imc+5])
        ios=self.value_read.find("OS")+2
        rep["status"]=int(self.value_read[ios:ios+8],2)
        return rep
    def info(self):
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
        print(rep)
        self.write(":OUT?;")
        rl=self.readline()
        
        rep["status"]=rl[3:len(rl)-1]
        return rep
