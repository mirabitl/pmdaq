#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
from bson.objectid import ObjectId
import time
import prettyjson as pj
import MongoAsic as mga




class MongoMROC(mga.MongoRoc):
    """!
    Main class to access the Asic MicroRoc in Mongo DB

    It inherits from MongoAsic.MongoRoc
    """

    def __init__(self, host,port,dbname,username,pwd):
        """!
        connect Mongodb database 

        @param host: Hostanme of the PC running the mongo DB

        @param port: Port to access the base

        @param dbname: Data base name

        @param username: Remote access user

        @param pwd: Remote access password

        @remark The constructor is called via a singleton creation with the function instance()
        
        """
        super().__init__(host,port,dbname,username,pwd)  
        self.state["type"]="MICROROC"  
    def reset(self):
        """!
        Reset connection to download another state
        """
        self.state = {}
        self.asiclist = []
        self.bson_id=[] 

    def addBoard(self,difid,nasic,address="USB"):
        """!
        Add a DIF with asics
        
        @param  address: IP address of the GRIC or USB for SDHCAL ones
 
        @param nasic: Number of MICROROC asics connected
        
        @param difid: unused for GRIC, DIF id for SDHCAL
        
        """

        if (address != "USB"):
            id=(mga.IP2Int(address)>>16)
            ipa=address
        else:
            id=difid
            ipa="0.0.0.%d" % difid
        for i in range(nasic):
            asic={}
            asic["address"]=ipa
            asic["dif"]=id
            asic["num"]=i+1
            asic["slc"]=self.initMROC(i+1,128)
            asic["_id"]=None
            print(asic["address"],asic["dif"],asic["num"],asic["_id"]," is added")
            self.asiclist.append(asic)

 
 
 
# MROC access
    def initMROC(self,num):
        """!
        MICROROC  initialisation, it creates a default dictionary representation of a HARDROC

        @param num: Asic number
        @return: the dictionary
        """

	#print("***** init MROC")
        _jasic={}
        _jasic["ENABLED"]=1
        _jasic["HEADER"]=num
        _jasic["QSCSROUTSC"]=1
        _jasic["ENOCDOUT1B"]=1
        _jasic["ENOCDOUT2B"]=0
        _jasic["ENOCTRANSMITON1B"]=1
        _jasic["ENOCTRANSMITON2B"]=0
        _jasic["ENOCCHIPSATB"]=1
        _jasic["SELENDREADOUT"]=1
        _jasic["SELSTARTREADOUT"]=1
        _jasic["CLKMUX"]=1
        _jasic["SCON"]=0
        _jasic["RAZCHNEXTVAL"]=0
        _jasic["RAZCHNINTVAL"]=1
        _jasic["TRIGEXTVAL"]=0
        _jasic["DISCROROR"]=1
        _jasic["ENTRIGOUT"]=1
        _jasic["TRIG0B"]=1
        _jasic["TRIG1B"]=0
        _jasic["TRIG2B"]=0
        _jasic["OTABGSW"]=0
        _jasic["DACSW"]=0
        _jasic["SMALLDAC"]=0

        _jasic["RS_OR_DISCRI"]=1
        _jasic["DISCRI1"]=0
        _jasic["DISCRI2"]=0
        _jasic["DISCRI0"]=0
        _jasic["OTAQ_PWRADC"]=0
        _jasic["EN_OTAQ"]=1
        _jasic["SW50F0"]=1
        _jasic["SW100F0"]=1
        _jasic["SW100K0"]=1
        _jasic["SW50K0"]=1
        _jasic["PWRONFSB1"]=0
        _jasic["PWRONFSB2"]=0
        _jasic["PWRONFSB0"]=0
        _jasic["SEL1"]=0
        _jasic["SEL0"]=1
        _jasic["SW50F1"]=1
        _jasic["SW100F1"]=1
        _jasic["SW100K1"]=1
        _jasic["SW50K1"]=1
        _jasic["CMDB0FSB1"]=1
        _jasic["CMDB1FSB1"]=1
        _jasic["CMDB2FSB1"]=0
        _jasic["CMDB3FSB1"]=1
        _jasic["SW50F2"]=1
        _jasic["SW100F2"]=1
        _jasic["SW100K2"]=1
        _jasic["SW50K2"]=1
        _jasic["CMDB0FSB2"]=1
        _jasic["CMDB1FSB2"]=1
        _jasic["CMDB2FSB2"]=0
        _jasic["CMDB3FSB2"]=1
        _jasic["PWRONW"]=0
        _jasic["PWRONSS"]=0
        _jasic["PWRONBUFF"]=0
        _jasic["SWSSC"]=7
        _jasic["CMDB0SS"]=0
        _jasic["CMDB1SS"]=0
        _jasic["CMDB2SS"]=0
        _jasic["CMDB3SS"]=0
        _jasic["PWRONPA"]=0
        # Unset power pulsing
        _jasic["CLKMUX"]=1
        _jasic["SCON"]=1
        _jasic["OTAQ_PWRADC"]=1
        _jasic["PWRONW"]=1
        _jasic["PWRONSS"]=0
        _jasic["PWRONBUFF"]=1
        _jasic["PWRONPA"]=1
        _jasic["DISCRI0"]=1
        _jasic["DISCRI1"]=1
        _jasic["DISCRI2"]=1
        _jasic["OTABGSW"]=1
        _jasic["DACSW"]=1
        _jasic["PWRONFSB0"]=1
        _jasic["PWRONFSB1"]=1
        _jasic["PWRONFSB2"]=1

        # Non-bool values
        dv=[]
        ct=[]
        for x in range(64):
            dv.append(8)
            ct.append(0)
        _jasic["DAC4BITS"]=dv
        _jasic["CTEST"]=ct
        mask=[]
        for i in range(64):
            mask.append(1)
        _jasic["MASK0"]=mask
        _jasic["MASK1"]=mask
        _jasic["MASK2"]=mask
        _jasic["BB2"]=250
        _jasic["BB1"]=250
        _jasic["BB0"]=250
        _jasic["SW_LG"]=1
        _jasic["SW_HG"]=1
        return _jasic


    def unsetPowerPulsing(self):
        """!
        Unset Power pulsing on all ASICs
        """
        for a in self.asiclist: 

            a["slc"]["CLKMUX"]=1;a["_id"]=None
            a["slc"]["SCON"]=1;a["_id"]=None
            a["slc"]["OTAQ_PWRADC"]=1;a["_id"]=None
            a["slc"]["PWRONW"]=1;a["_id"]=None
            a["slc"]["PWRONSS"]=1;a["_id"]=None
            a["slc"]["PWRONBUFF"]=1;a["_id"]=None
            a["slc"]["PWRONPA"]=1;a["_id"]=None
            a["slc"]["DISCRI0"]=1;a["_id"]=None
            a["slc"]["DISCRI1"]=1;a["_id"]=None
            a["slc"]["DISCRI2"]=1;a["_id"]=None
            a["slc"]["OTABGSW"]=1;a["_id"]=None
            a["slc"]["DACSW"]=1;a["_id"]=None
            a["slc"]["PWRONFSB0"]=1;a["_id"]=None
            a["slc"]["PWRONFSB1"]=1;a["_id"]=None
            a["slc"]["PWRONFSB2"]=1;a["_id"]=None



    def setPowerPulsing(self):
        """!
        set Power pulsing on all ASICs
        """
        for a in self.asiclist: 

            a["slc"]["CLKMUX"]=0;a["_id"]=None
            a["slc"]["SCON"]=0;a["_id"]=None
            a["slc"]["OTAQ_PWRADC"]=0;a["_id"]=None
            a["slc"]["PWRONW"]=0;a["_id"]=None
            a["slc"]["PWRONSS"]=0;a["_id"]=None
            a["slc"]["PWRONBUFF"]=0;a["_id"]=None
            a["slc"]["PWRONPA"]=0;a["_id"]=None
            a["slc"]["DISCRI0"]=0;a["_id"]=None
            a["slc"]["DISCRI1"]=1;a["_id"]=None
            a["slc"]["DISCRI2"]=1;a["_id"]=None
            a["slc"]["OTABGSW"]=0;a["_id"]=None
            a["slc"]["DACSW"]=0;a["_id"]=None
            a["slc"]["PWRONFSB0"]=0;a["_id"]=None
            a["slc"]["PWRONFSB1"]=1;a["_id"]=None
            a["slc"]["PWRONFSB2"]=1;a["_id"]=None

 

    def changeThreshold(self,BB0,BB1,BB2,idif=0,iasic=0):
        """!
        Set the 3 thresholds of specified  asics, modified asics are tagged for upload

        @param BB0: First Threshold
        @param BB1: Second Threshold
        @param BB2: Third Threshold        
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["BB0"]=B0;a["_id"]=None;
            a["slc"]["BB1"]=B1;a["_id"]=None;
            a["slc"]["BB2"]=B2;a["_id"]=None;

    def changeDac4bits(self,idif,iasic,ipad,scale):
        """!
        Scale the dac4bits of one pad  of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param ipad: Channel number
        @param scale: ratio Dac4bits_new/Dac4bits

        """
      

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["DAC4BITS"][ipad]=scale*a["slc"]["DAC4BITS"][ipad]
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["DAC4BITS"][ipad])

    def setPadDac4bits(self,idif,iasic,ipad,gnew):
        """!
        Set the dac4bits of one pad  of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param ipad: Channel number
        @param gnew: new dac4bits

        """
  
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["DAC4BITS"][ipad]=gnew
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["DAC4BITS"][ipad])
   

    def setAsicDac4bits(self,idif,iasic,gnew):
        """!
        Set the dac4bits of all pads  of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param gnew: new dac4bits

        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in range(0,64):
                a["slc"]["DAC4BITS"][ipad]=gnew
                a["_id"]=None
                print(idif,iasic,ipad,a["slc"]["DAC4BITS"][ipad])



                
    def setAsicAllDac4bits(self,idif,iasic,vnew):
        """!
        Set the dac4bits of all pads  of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param vnew: vector of new dac4bitss

        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in range(0,64):
                a["slc"]["DAC4BITS"][ipad]=vnew[ipad]
                a["_id"]=None
            print(idif,iasic,a["slc"]["DAC4BITS"])



                
    def slowShaper(self):
        """!
        Slow down the shaper , Set SW100 F and K to 1
        """
        for a in self.asiclist:
            
            a["slc"]["SW100F0"]=1
            a["slc"]["SW100K0"]=1
            a["slc"]["SW50F0"]=0
            a["slc"]["SW50K0"]=0

            a["slc"]["SW100F1"]=1
            a["slc"]["SW100K1"]=1
            a["slc"]["SW50F1"]=0
            a["slc"]["SW50K1"]=0

            a["slc"]["SW100F2"]=1
            a["slc"]["SW100K2"]=1
            a["slc"]["SW50F2"]=0
            a["slc"]["SW50K2"]=0
            a["_id"]=None
            
    def verySlowShaper(self):
        """!
        Slow down the shaper , Set SW100 F and K to 1
        """
        for a in self.asiclist:
            
            a["slc"]["SW100F0"]=1
            a["slc"]["SW100K0"]=1
            a["slc"]["SW50F0"]=1
            a["slc"]["SW50K0"]=1

            a["slc"]["SW100F1"]=1
            a["slc"]["SW100K1"]=1
            a["slc"]["SW50F1"]=1
            a["slc"]["SW50K1"]=1

            a["slc"]["SW100F2"]=1
            a["slc"]["SW100K2"]=1
            a["slc"]["SW50F2"]=1
            a["slc"]["SW50K2"]=1
            a["_id"]=None
            
    def changeCTest(self,channel,ctest,idif=0,iasic=0):
        """!
         Change the CTEST value of one channel  of specified  asics, modified asics are tagged for upload
        
        @param channel: Pad tested
        @param ctest: CTEST value
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed

        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            a["slc"]["CTEST"][channel]=ctest
            a["_id"]=None


            
    def setMask(self,list,idif=0,iasic=0):
        """!
        Mask the channels specified in the list  for specified  asics, modified asics are tagged for upload
        
        @param list: List of channels to be masked
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed

        """
        m=0xFFFFFFFFFFFFFFFF
        for i in list:
            m=m &~(1<<i);
        sm="0x%lx" % m
        self.changeMask(sm,sm,sm,idif,iasic)
        
    def setNewMask(self,list,idif=0,iasic=0):
        """!
        Clear the Mask of the channels specified in the list  for specified  asics, modified asics are tagged for upload
        
        @param list: List of channels to be masked
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed

        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in list:
                a["slc"]["MASK0"][ipad]=0
                a["slc"]["MASK1"][ipad]=0
                a["slc"]["MASK2"][ipad]=0
            a["_id"]=None

    def changeMask(self,M0,M1,M2,idif=0,iasic=0):
        """!
        Set the 3 masks  for specified  asics, modified asics are tagged for upload
        
        @param M0: Hexadecimal string of threshold 0 mask
        @param M1: Hexadecimal string of threshold 1 mask
        @param M2: Hexadecimal string of threshold 2 mask
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed

        """
        print(M0,M1,M2,idif,iasic)
        im0n=int(M0,16)
        im1n=int(M1,16)
        im2n=int(M2,16)
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in range(0,64):
                a["slc"]["MASK0"][ipad]=a["slc"]["MASK0"][ipad]& (im0n>>ipad)
                a["slc"]["MASK1"][ipad]=a["slc"]["MASK1"][ipad]& (im1n>>ipad)
                a["slc"]["MASK2"][ipad]=a["slc"]["MASK2"][ipad]& (im2n>>ipad)
            a["_id"]=None

    def setChannelMask(self,idif,iasic,ipad,ival):
        """!
        Set the 3 masks to thesame value for a pad and for specified  asics, modified asics are tagged for upload
        

        @param idif: DIF_ID
        @param iasic: asic number
        @param ipad: pad from 0
        @param ival: 0 Off 1 On

        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["MASK0"][ipad]=ival
            a["slc"]["MASK1"][ipad]=ival
            a["slc"]["MASK2"][ipad]=ival
            a["_id"]=None

    def setEnable(self,enable,idif=0,iasic=0):
        """!
        Set the ENABLE tag for specified  asics, modified asics are tagged for upload
        
        
        @param enable: Enable value (1/0)
        @param idif: DIF_ID, if 0 all DIFs are changed
        @param iasic: asic number, if 0 all Asics are changed

        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            a["slc"]["ENABLED"]=enable
            a["_id"]=None


      
def MROCInstance():
    """!
    Create a MongoMROC Object

    @return: The MongoMROC Object
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
        _wdd=MongoMROC(host,port,dbname,user,pwd)
        return _wdd
    else:
        if os.path.isfile("/etc/.mongoroc.json"):
            f=open("/etc/.mongoroc.json")
            s=json.loads(f.read())
            _wdd=MongoMROC(s["host"],s["port"],s["db"],s["user"],s["pwd"])
            f.close()
            return _wdd
        else:
            return None
