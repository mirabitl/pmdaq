#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
from bson.objectid import ObjectId
import time
import prettyjson as pj
import MongoAsic as mga





class MongoPR2(mga.MongoRoc):
    """
    Main class to access PETIROC asics in MongoDB
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

        @remark
        The constructor is called via a singleton creation with the function instance()
        """

        super().__init__(host,port,dbname,username,pwd)    
        self.state["type"]="PETIROC"  

    def addBoard(self,ipname,nasic,asictype="PR2"):
        """!
        Add a FEBV1 with asics

        @param  ipname: IP address of the FEBV1

        @param nasic: Number of PETIROC asics connected

        @param asictype: "PR2" for PETIROC2A , "PR2B" for PETIROC2B
        """
        febid=mga.IP2Int(ipname)
        for i in range(nasic):
            asic={}
            asic["address"]=ipname
            asic["dif"]=(febid>>16)&0xFFFF
            asic["num"]=i+1
            asic["slc"]=self.initPR2(i+1,asictype)
            asic["_id"]=None
            print(asic["dif"],asic["num"],asic["_id"]," is added")
            self.asiclist.append(asic)
            
 
  
   
  
    def initPR2(self, num,version="PR2"):
        """!
        PETIROC 2  initialisation, it creates a default dictionary representation of a PETIROC2

        @param num: Asic number
        @param version: Asic type (PR2 or PR2B)
        @return: the dictionary
        """
	#print("***** init HR2")
        _jasic={}
        _jasic["header"]=num
        _jasic["EN10bDac"] = 1
        _jasic["PP10bDac"] = 0
        _jasic["EN_adc"] =0
        _jasic["PP_adc"] = 0
        _jasic["sel_starb_ramp_adc_ext"] = 0
        _jasic["usebcompensation"] = 0
        _jasic["EN_bias_dac_delay"] = 0
        _jasic["PP_bias_dac_delay"] = 0
        _jasic["EN_bias_ramp_delay"] = 0
        _jasic["PP_bias_ramp_delay"] = 0
        _jasic["EN_discri_delay"] =0
        _jasic["PP_discri_delay"] = 0
        _jasic["EN_temp_sensor"] = 0
        _jasic["PP_temp_sensor"] = 0
        _jasic["EN_bias_pa"] = 1
        _jasic["PP_bias_pa"] = 0
        _jasic["EN_bias_discri"] = 1
        _jasic["PP_bias_discri"] = 0
        _jasic["cmd_polarity"] = 0
        _jasic["latch"] = 1
        _jasic["EN_bias_6bit_dac"] =1
        _jasic["PP_bias_6bit_dac"] = 0
        _jasic["EN_bias_tdc"] = 0
        _jasic["PP_bias_tdc"] = 0
        _jasic["ON_OFF_input_dac"] = 1
        _jasic["EN_bias_charge"] = 0
        _jasic["PP_bias_charge"] = 0
        _jasic["Cf3_100fF"] = 0
        _jasic["Cf2_200fF"] = 0
        _jasic["Cf1_2p5pF"] = 0
        _jasic["Cf0_1p25pF"] = 0
        _jasic["EN_bias_sca"] = 0
        _jasic["PP_bias_sca"] = 0
        _jasic["EN_bias_discri_charge"] = 0
        _jasic["PP_bias_discri_charge"] = 0
        _jasic["EN_bias_discri_adc_time"] = 0
        _jasic["PP_bias_discri_adc_time"] = 0
        _jasic["EN_bias_discri_adc_charge"] = 0
        _jasic["PP_bias_discri_adc_charge"] = 0
        _jasic["DIS_razchn_int"] = 1
        _jasic["DIS_razchn_ext"] = 0
        _jasic["SEL_80M"] = 0
        _jasic["EN_80M"] = 0
        _jasic["EN_slow_lvds_rec"] = 1
        _jasic["PP_slow_lvds_rec"] = 0
        _jasic["EN_fast_lvds_rec"] = 1
        _jasic["PP_fast_lvds_rec"] = 0
        _jasic["EN_transmitter"] = 0
        _jasic["PP_transmitter"] = 0
        _jasic["ON_OFF_1mA"] =1
        _jasic["ON_OFF_2mA"] = 1
        _jasic["ON_OFF_otaQ"] = 0
        _jasic["ON_OFF_ota_mux"] = 0
        _jasic["ON_OFF_ota_probe"] = 0
        _jasic["DIS_trig_mux"] = 1
        _jasic["EN_NOR32_time"] = 1
        _jasic["EN_NOR32_charge"] = 0
        _jasic["DIS_triggers"] = 0
        _jasic["EN_dout_oc"] = 0
        _jasic["EN_transmit"] = 1
        if (version == "PR2B"):
            _jasic["PA_ccomp_0"] =0
            _jasic["PA_ccomp_1"] =0
            _jasic["PA_ccomp_2"] = 0
            _jasic["PA_ccomp_3"] =0
            _jasic["Choice_Trigger_Out"] =0
        _jasic["DacDelay"] = 0
        idac=[]
        bdac=[]
        mdc=[]
        mdt=[]
        idc=[]
        for ch in range(32):
            idac.append(125);
            bdac.append(31);
            mdc.append(1);
            mdt.append(0)
            idc.append(1)
    
        _jasic["InputDac"] = idac;
        _jasic["6bDac"] = bdac;
        _jasic["MaskDiscriCharge"] = mdc;
        _jasic["MaskDiscriTime"] = mdt;
        _jasic["InputDacCommand"] = idc;

        _jasic["VthDiscriCharge"] = 863
        _jasic["VthTime"] = 610

        return _jasic


        

    def addAsic(self, dif_num, header,version="PR2"):
        """!
        Add a new PETIROC2 to the asic list

        @param dif_num: DIF ID (ipaddr in integer >>16)
        @param header: ASIC number
        @param version: PR2 for 2A , PR2B for 2B
        """
        print("force ASIC")

        thePR2 = self.initPR2(dif_num, header,version)
        thePR2["_id"]=None
        self.asiclist.append(thePR2)

 
    def changeLatch(self, Latch, idif=0, iasic=0):
        """!
        Change the Latch mode of specified  asics, modified asics are tagged for upload
        
        @param Latch: Latch value (1/0)
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["latch"]=Latch
                a["_id"]=None
            except Exception as e:
                print(e.getMessage())

    def changeVthTime(self, VthTime, idif=0, iasic=0):
        """!
        Change the VTHTIME threshold of specified  asics, modified asics are tagged for upload
        
        @param VthTime: Threshold of time discriminators
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["VthTime"]=VthTime
                a["_id"]=None
            except Exception as e:
                print(e.getMessage())


    def changeDacDelay(self, delay, idif=0, iasic=0):
        """!
        Change the DAC delay of specified  asics, modified asics are tagged for upload
        
        @param delay: Dac delay
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """        
       
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["DacDelay"]=delay
                a["_id"]=None
            except Exception as e:
                print(e.getMessage())

    def changeAllEnabled(self, idif=0, iasic=0):
        """!
        Change all the ENable signals of PETIROC asic

        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["EN_bias_discri"]=1
                a["slc"]["EN_bias_pa"]=1
                a["slc"]["EN_bias_discri_charge"]=1
                a["slc"]["EN_dout_oc"]=1
                a["slc"]["EN_bias_dac_delay"]=1
                a["slc"]["EN10bdac"]=1
                a["slc"]["EN_bias_discri_adc_charge"]=1
                a["slc"]["EN_bias_sca"]=1
                a["slc"]["EN_bias_6bit_dac"]=1
                a["slc"]["EN_transmit"]=1
                a["slc"]["EN_bias_ramp_delay"]=1
                a["slc"]["EN_bias_charge"]=1
                a["slc"]["EN_fast_lvds_rec"]=1
                a["slc"]["EN_transmitter"]=1
                a["slc"]["EN_adc"]=1
                a["slc"]["EN_NOR32_charge"]=1
                a["slc"]["EN_80M"]=1
                a["slc"]["EN_discri_delay"]=1
                a["slc"]["EN_bias_discri_adc_time"]=1
                a["slc"]["EN_NOR32_time"]=1
                a["slc"]["EN_temp_sensor"]=1

                a["_id"]=None

            except Exception as e:
                print(e.getMessage())

    def changeInputDac(self, idif, iasic, ich, dac):
        """!
        Change the InputDAC valu of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param ich: The channel number
        @param dac: The DAC value
        """
        

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["InputDac"][ich]=dac
                a["_id"]=None
            except Exception as e:
                print(e)
    def change6BDac(self, idif, iasic, ich, dac):
        """!
        Change the 6BDAC valu of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param ich: The channel number
        @param dac: The DAC value
        """
        

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["6bDac"][ich]=dac
                a["_id"]=None
            except Exception as e:
                print(e)
    def correct6BDac(self, idif, iasic, cor):
        """!
        Correct the 6BDAC value of specified  asics, modified asics are tagged for upload
        
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param cor:  A 32 channels array of additive corrections to be applied on the 6BDAC values of all channels
        """
       
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                print(a["slc"]["6bDac"])
                for ich in range(32):
                    print(" Dac changed", idif, iasic, ich, cor[ich])
                    ng= a["slc"]["6bDac"][ich]+cor[ich]
                    if (ng<0):
                        ng=1
                    if (ng>63):
                        ng=63
                    a["slc"]["6bDac"][ich] =ng
                print(a["slc"]["6bDac"])
                a["_id"]=None
            except Exception as e:
                print(e)


    def changeMask(self, idif, iasic, ich, mask):
        """!
        Change PETIROC2 MASKDISCRITIME parameter for one channel

        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param ich: The channel number
        @param mask: the channel mask
        :warning: 1 = inactive, 0=active
        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["MaskDiscriTime"][ich] = mask
                a["_id"]=None
            except Exception as e:
                print(e)
    def changeInputDacCommand(self, idif, iasic, ich, active):
        """!
        Change PETIROC2 MASKDISCRITIME parameter for one channel

        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        @param ich: The channel number
        @param active: the channel mask
        :warning: 1 = active, 0=inactive
        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["InputDacCommand"][ich] = active
                a["_id"]=None
            except Exception as e:
                print(e)
                
    def setCCOMP(self, v0,v1,v2,v3, idif=0, iasic=0):
        """!
        Change the CCOMP value of specified  asics, modified asics are tagged for upload
        
        @param v0: CCOMP 0 value
        @param v1: CCOMP 1 value
        @param v2: CCOMP 2 value
        @param v3: CCOMP 3 value
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """        
       
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["PA_ccomp_0"]=v0
                a["slc"]["PA_ccomp_1"]=v1
                a["slc"]["PA_ccomp_2"]=v2
                a["slc"]["PA_ccomp_3"]=v3
                a["_id"]=None
            except Exception as e:
                print(e.getMessage())


    def setCfValue(self, v0,v1,v2,v3, idif=0, iasic=0):
        """!
        Change the Cf values of specified  asics, modified asics are tagged for upload
        
        @param v0: Cf0_1p25pF  value
        @param v1: Cf1_2p5pF  value
        @param v2: Cf2_200fF  value
        @param v3: Cf3_100fF  value
        @param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        @param iasic: asic number, if 0 all Asics are changed
        """  
               
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["Cf0_1p25pF"]=v0
                a["slc"]["Cf1_2p5pF"]=v1
                a["slc"]["Cf2_200fF"]=v2
                a["slc"]["Cf3_100fF"]=v3
                a["_id"]=None
            except Exception as e:
                print(e.getMessage())
                


def PR2Instance():
    """!
    Create a MongoRoc Object

    @return: The MongoRoc Object
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
        _wdd=MongoPR2(host,port,dbname,user,pwd)
        return _wdd
    else:
        if os.path.isfile("/etc/.mongoroc.json"):
            f=open("/etc/.mongoroc.json")
            s=json.loads(f.read())
            _wdd=MongoPR2(s["host"],s["port"],s["db"],s["user"],s["pwd"])
            f.close()
            return _wdd
        else:
            return None
