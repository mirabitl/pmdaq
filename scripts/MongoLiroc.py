#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
from bson.objectid import ObjectId
import time
import prettyjson as pj
import MongoAsic as mga





class MongoLiroc(mga.MongoRoc):
    """
    Main class to access the Mongo DB
    """

    def __init__(self, host,port,dbname,username,pwd):
        """
        connect Mongodb database 

        :param host: Hostanme of the PC running the mongo DB

        :param port: Port to access the base

        :param dbname: Data base name

        :param username: Remote access user

        :param pwd: Remote access password

        """

        super().__init__(host,port,dbname,username,pwd)    
        self.state["type"]="LIROC"  


    def addLBoard(self,difid,nasic):
        """
        Add a LIROC board with asics
        
        :param nasic: Number of LIROC asics connected
        
        :param difid: LIROC board USB id
        
        """


        ipa="0.0.0.%d" % difid
        for i in range(nasic):
            asic={}
            asic["address"]=ipa
            asic["dif"]=difid
            asic["num"]=i+1
            asic["slc"]=self.initLIROC(0)
            asic["_id"]=None
            print(asic["address"],asic["dif"],asic["num"],asic["_id"]," is added")
            self.asiclist.append(asic)

    

        
 
  
# LIROC access
    def initLIROC(self,gain=0):
        """
   
        LIROC initialisation, it creates a default dictionary representation of a LIROC

        :param gain: Channel gain
        :return: the dictionary
        """

	#print("***** init HR2")
        _kasic={}
        _kasic["EN_pa"]=1
        _kasic["PP_pa"]=0
        _kasic["PA_gain"]=10
        _kasic["EN_7b"]=1
        _kasic["PP_7b"]=0
        _kasic["EN_disc"]=1
        _kasic["PP_disc"]=0
        _kasic["Polarity"]=0
        _kasic["Hysteresys"]=0
        _kasic["EN_bg"]=1
        _kasic["PP_bg"]=0
        _kasic["EN_10bDAC"]=1
        _kasic["PP_10bDAC"]=0
        _kasic["DAC_threshold"]=472
        
        _kasic["CLPS_bsize"]=4
        _kasic["EN_pre_emphasis"]=0
        _kasic["Pre_emphasis_delay"]=0
        _kasic["EN_differential"]=0
        _kasic["PP_differential"]=0
        _kasic["Forced_ValEvt"]=1
        _kasic["EN_probe"]=1
        _kasic["PP_probe"]=0
        _kasic["MillerComp"]=4
        _kasic["Ibi_probe"]=2
        _kasic["Ibo_probe"]=32
        # Channel dependant values
        dc_pa=[]
        ct=[]
        mk=[]
        dac=[]
        for x in range(64):
            dc_pa.append(gain)
            ct.append(0)
            mk.append(0)
            dac.append(64)
        _kasic["DC_pa"]=dc_pa
        _kasic["Ctest"]=ct
        _kasic["DAC_local"]=dac
        _kasic["Mask"]=mk

        return _kasic

    def setForced_ValEvt(self,forced,idif=0,iasic=0):
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["Forced_ValEvt"]=forced;a["_id"]=None;
            if (forced==1):
                a["slc"]["EN_differential"]=0;a["_id"]=None;
            else:
                a["slc"]["EN_differential"]=1;a["_id"]=None;
                
    def setDAC_threshold(self,thr):
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["DAC_threshold"]=thr;a["_id"]=None;
            
    def setPA_gain(self,g):
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["PA_gain"]=g;a["_id"]=None;

    def setDC_pa(self,idif,iasic,ipad,vnew):
        """
        Set the gain of one pad  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ipad: Channel number
        :param vnew: new gain

        """
  
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["DC_pa"][ipad]=vnew
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["DC_pa"][ipad])
            
    def setDAC_local(self,idif,iasic,ipad,vnew):
        """
        Set the gain of one pad  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ipad: Channel number
        :param vnew: new gain

        """
  
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["DAC_local"][ipad]=vnew
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["DAC_local"][ipad])

    def shiftDAC_local(self,idif,iasic,delta):
        """
        Set the gain of one pad  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ipad: Channel number
        :param vnew: new gain

        """
  
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in range(64):
                a["slc"]["DAC_local"][ipad]= a["slc"]["DAC_local"][ipad]+delta[ipad]
            a["_id"]=None
            print(idif,iasic,a["slc"]["DAC_local"])
            

    def setCtest(self,idif,iasic,ipad,vnew):
        """
        Set the gain of one pad  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ipad: Channel number
        :param vnew: new gain

        """
  
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["Ctest"][ipad]=vnew
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["Ctest"][ipad])
            
    def setMask(self,idif,iasic,ipad,vnew):
        """
        Set the gain of one pad  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ipad: Channel number
        :param vnew: new gain

        """
  
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["Mask"][ipad]=vnew
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["Mask"][ipad])
            
   
      
def LirocInstance():
    """
    Create a MongoRLiroc Object

    :return: The MongoLiRoc Object
    """
    # create the default access
    login=os.getenv("MGDBLOGIN","NONE")
    if (login != "NONE"):
        
        userinfo=login.split("@")[0]
        hostinfo=login.split("@")[1]
        dbname=login.split("@")[2]
        user=userinfo.split("/")[0]
        pwd=userinfo.split("/")[1]
        host=hostinfo.split(":")[0]
        port=int(hostinfo.split(":")[1])
        #print("MGROC::INSTANCE() ",host,port,dbname,user,pwd)
        _wdd=MongoLiroc(host,port,dbname,user,pwd)
        return _wdd
    else:
        if os.path.isfile("/etc/.mongoroc.json"):
            f=open("/etc/.mongoroc.json")
            s=json.loads(f.read())
            _wdd=MongoLiroc(s["host"],s["port"],s["db"],s["user"],s["pwd"])
            f.close()
            return _wdd
        else:
            return None
