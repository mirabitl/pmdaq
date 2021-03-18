#!/usr/bin/env python3
import os
from pymongo import MongoClient
import json
from bson.objectid import ObjectId
import time
import prettyjson as pj




def IP2Int(ip):
    """
    convert IP adress string to int

    :param IP: the IP address
    :return: the encoded integer 
    """
    o = list(map(int, ip.split('.')))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res


class MongoRoc:
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
        self.connection=MongoClient(host,port)
        self.db=self.connection[dbname]
        self.db.authenticate(username,pwd)
        self.state = {}
        self.asiclist = []
        self.bson_id=[]

    def reset(self):
        """
        Reset connection to download another state
        """
        self.state = {}
        self.asiclist = []
        self.bson_id=[] 
    def createNewState(self,name):
        """
        Create a new state , version is set to 1

        :param name: Name of the state

        """
        self.state["name"]=name
        self.state["version"]=1
        self.state["asics"]=[]
    def addFEB(self,ipname,nasic,asictype="PR2"):
        """
        Add a FEBV1 with asics

        :param  ipname: IP address of the FEBV1

        :param nasic: Number of PETIROC asics connected

        :param asictype: "PR2" for PETIROC2A , "PR2B" for PETIROC2B
        """
        febid=IP2Int(ipname)
        for i in range(nasic):
            asic={}
            asic["address"]=ipname
            asic["dif"]=(febid>>16)&0xFFFF
            asic["num"]=i+1
            asic["slc"]=self.initPR2(i+1,asictype)
            asic["_id"]=None
            print(asic["dif"],asic["num"],asic["_id"]," is added")
            self.asiclist.append(asic)
            
    def addDIF(self,difid,nasic,address="USB"):
        """
        Add a DIF with asics
        
        :param  address: IP address of the GRIC or USB for SDHCAL ones
 
        :param nasic: Number of HARDROC2 asics connected
        
        :param difid: unused for GRIC, DIF id for SDHCAL
        
        """

        if (address != "USB"):
            id=(IP2Int(address)>>16)
            ipa=address
        else:
            id=difid
            ipa="0.0.0.%d" % difid
        for i in range(nasic):
            asic={}
            asic["address"]=ipa
            asic["dif"]=id
            asic["num"]=i+1
            asic["slc"]=self.initHR2(i+1,128)
            asic["_id"]=None
            print(asic["address"],asic["dif"],asic["num"],asic["_id"]," is added")
            self.asiclist.append(asic)

    
    def uploadFromFile(self,fname):
        """
        Upload a state in DB from a JSON file

        :param fname: File name
        """
        f=open(fname)
        sf=json.loads(f.read())
        f.close()
        self.state["name"]=sf["state"]
        self.state["version"]=sf["version"]
        for x in sf["asics"]:
            result=self.db.asics.insert_one(x)
            x["_id"]=result.inserted_id
        self.bson_id=[]
        for  i in range(len(sf["asics"])):
            self.bson_id.append(sf["asics"][i]["_id"])
        self.state["asics"]=self.bson_id
        self.state["comment"]="Upload from %s" % fname
        resstate=self.db.states.insert_one(self.state)
        print(resstate)
        
    def uploadNewState(self,comment="NEW"):
        """
        Create a new state in the DB with data stored in object memory

        :param comment: A comment on the state

        """
        # First append modified ASICS
        for i in range(len(self.asiclist)):
            if (self.asiclist[i]["_id"]!=None):
                continue
            del self.asiclist[i]["_id"]
            result=self.db.asics.insert_one(self.asiclist[i])
            self.asiclist[i]["_id"]=result.inserted_id
        for  i in range(len(self.asiclist)):
            self.bson_id.append(self.asiclist[i]["_id"])
        self.state["asics"]=self.bson_id
        self.state["comment"]=comment
        resstate=self.db.states.insert_one(self.state)
        print(resstate)
    def uploadFromOracle(self,asiclist,statename,version,comment="NEW"):
        """
        Migration method to update an ASIC list created with OracleAccess class to the DB

        :param asiclist: List of asics created with OracleAccess

        :param statename: Name of the state

        :param version: version of the state

        :param comment: A comment on the state

        """
        self.state["name"]=statename
        self.state["version"]=version
        self.state["asics"]=[]
        for i in range(len(asiclist)):
            self.asiclist.append(asiclist[i])
        # First append modified ASICS
        for i in range(len(self.asiclist)):
            if (self.asiclist[i]["_id"]!=None):
                continue
            del self.asiclist[i]["_id"]
            result=self.db.asics.insert_one(self.asiclist[i])
            self.asiclist[i]["_id"]=result.inserted_id
        for  i in range(len(self.asiclist)):
            self.bson_id.append(self.asiclist[i]["_id"])
        self.state["asics"]=self.bson_id
        self.state["comment"]=comment
        resstate=self.db.states.insert_one(self.state)
        print(resstate)
    def uploadConfig(self,name,fname,comment,version=1):
        """
        jobcontrol configuration upload

        :obsolete: Use MongoJob instead
        """
        s={}
        s["content"]=json.loads(open(fname).read())
        s["name"]=name
        s["time"]=time.time()
        s["comment"]=comment
        s["version"]=version
        resconf=self.db.configurations.insert_one(s)
        print(resconf)
    def states(self):
        """
        List all states in the DB
        """
        cl=[]
        res=self.db.states.find({})
        for x in res:
            if (not ("name" in x)):
                continue
            if ("comment" in x):
                print(x["name"],x["version"],x["comment"])
                cl.append((x["name"],x['version'],x['comment']))
            else:
                print(x["name"],x["version"] )
                cl.append((x["name"],x['version'],"None"))
        return cl
    def configurations(self):
        """
        List all jobcontrol configurations in the db 

        :obsolete: Use MongoJob instead
        """
        res=self.db.configurations.find({})
        for x in res:
            if ("comment" in x):
                print(time.ctime(x["time"]),x["version"],x["name"],x["comment"])

    def downloadConfig(self,cname,version):
        """
        Download a jobcontrol configuration to /dev/shm/mgjob/ directory
        
        :param cname: Configuration name
        :param version: Configuration version
         
        :obsolete: Use MongoJob instead
        """
        print("Downloading %s version %d" % (cname,version))
        res=self.db.configurations.find({'name':cname,'version':version})
        for x in res:
            print(x["name"],x["version"],x["comment"])
            #var=raw_input()
            slc=x["content"]
            os.system("mkdir -p /dev/shm/mgroc")
            fname="/dev/shm/mgjob/%s_%s.json" % (cname,version)
            f=open(fname,"w+")
            f.write(json.dumps(slc, indent=2, sort_keys=True))
            f.close()
            return slc
        
    def download(self,statename,version,toFileOnly=False):
        """
        Download a state configuration to /dev/shm/mgroc/ directory and load it in the MongoRoc object
        
        :param statename: State name
        :param version: State version
        :param toFileOnly: if True and /dev/shm/mgroc/statename_version.json already exists, it exits
        """        
        os.system("mkdir -p /dev/shm/mgroc")
        fname="/dev/shm/mgroc/%s_%s.json" % (statename,version)
        if os.path.isfile(fname) and toFileOnly:
            print('%s already download, Exiting' % fname)
            return None
        res=self.db.states.find({'name':statename,'version':version})
        for x in res:
            print(x["name"],x["version"],len(x["asics"])," asics")
            self.state["name"]=x["name"]
            self.state["version"]=x["version"]
            #var=raw_input()
            slc={}
            slc["state"]=statename
            slc["version"]=version
            slc["asics"]=[]
            self.asiclist=[]
            #for y in x["asics"]:
            #    resa=self.db.asics.find_one({'_id':y})
            #print(x["asics"])
            resl=self.db.asics.find({'_id': {'$in': x["asics"]}})
            
            for resa in resl:
                self.asiclist.append(resa)
                #print(resa)
                s={}
                s["slc"]=resa["slc"]
                s["num"]=resa["num"]
                s["dif"]=resa["dif"]
                if ( "address" in resa):
                    s["address"]=resa["address"]
                #print(res["dif"])
                slc["asics"].append(s)

            #os.system("mkdir -p /dev/shm/mgroc")
            #fname="/dev/shm/mgroc/%s_%s.json" % (statename,version)
            #if os.path.isfile(fname):
            #    print('%s already download' % fname)
            #else:
            f=open(fname,"w+")
            #f.write(json.dumps(slc,indent=2, sort_keys=True))
            f.write(json.dumps(slc,sort_keys=True))
            #f.write(pj.prettyjson(slc, maxlinelength=255))
            f.close()
            return slc
    def initPR2(self, num,version="PR2"):
        """
        PETIROC 2  initialisation, it creates a default dictionary representation of a PETIROC2

        :param num: Asic number
        :param version: Asic type (PR2 or PR2B)
        :return: the dictionary
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


    def uploadChanges(self,statename,comment):
        """
        Upload a new version of the state
        it finds the last version of the state and upload a new one with incremented version number

        :param statename: Name of the state
        :param comment: A comment on the changes
        """
        # Find last version
        res=self.db.states.find({'name':statename})
        last=0
        for x in res:
            if (last<x["version"]):
                last=x["version"]
        if (last==0):
            print(" No state ",statename,"found")
            return
        # First append modified ASICS
        for i in range(len(self.asiclist)):
            if (self.asiclist[i]["_id"]!=None):
                continue
            del self.asiclist[i]["_id"]
            result=self.db.asics.insert_one(self.asiclist[i])
            self.asiclist[i]["_id"]=result.inserted_id
        self.bson_id=[]
        for  a in self.asiclist:
            print(a)
            print(a["_id"])
            self.bson_id.append(a["_id"])
        self.state["asics"]=self.bson_id
        self.state["version"]=last+1
        self.state["comment"]=comment
        resstate=self.db.states.insert_one(self.state)
        print(resstate,self.state["version"],self.state["name"])
        

    def addAsic(self, dif_num, header,version="PR2"):
        """
        Add a new PETIROC2 to the asic list

        :param dif_num: DIF ID (ipaddr in integer >>16)
        :param header: ASIC number
        :param version: PR2 for 2A , PR2B for 2B
        """
        print("force ASIC")

        thePR2 = self.initPR2(dif_num, header,version)
        self.asiclist.append(thePR2)
        self.asicConf.add(thePR2)

 
    def PR2_ChangeLatch(self, Latch, idif=0, iasic=0):
        """
        Change the Latch mode of specified  asics, modified asics are tagged for upload
        
        :param Latch: Latch value (1/0)
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
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

    def PR2_ChangeVthTime(self, VthTime, idif=0, iasic=0):
        """
        Change the VTHTIME threshold of specified  asics, modified asics are tagged for upload
        
        :param VthTime: Threshold of time discriminators
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
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


    def PR2_ChangeDacDelay(self, delay, idif=0, iasic=0):
        """
        Change the DAC delay of specified  asics, modified asics are tagged for upload
        
        :param delay: Dac delay
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
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

    def PR2_ChangeAllEnabled(self, idif=0, iasic=0):
        """
        Change all the ENable signals of PETIROC asic

        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
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

    def PR2_ChangeInputDac(self, idif, iasic, ich, dac):
        """
        Change the InputDAC valu of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ich: The channel number
        :param dac: The DAC value
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
    def PR2_Change6BDac(self, idif, iasic, ich, dac):
        """
        Change the 6BDAC valu of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ich: The channel number
        :param dac: The DAC value
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
    def PR2_Correct6BDac(self, idif, iasic, cor):
        """
        Correct the 6BDAC value of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param cor:  A 32 channels array of corrections to be applied on the 6BDAC values of all channels
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


    def PR2_ChangeMask(self, idif, iasic, ich, mask):
        """
        Change PETIROC2 MASKDISCRITIME parameter for one channel

        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ich: The channel number
        :param mask: the channel mask
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
                
    def PR2_SetCCOMP(self, v0,v1,v2,v3, idif=0, iasic=0):
        """
        Change the CCOMP value of specified  asics, modified asics are tagged for upload
        
        :param v0: CCOMP 0 value
        :param v1: CCOMP 1 value
        :param v2: CCOMP 2 value
        :param v3: CCOMP 3 value
        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
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


                
# HR2 access
    def initHR2(self,num,gain=128):
        """
   
        HARDROC 2  initialisation, it creates a default dictionary representation of a HARDROC

        :param num: Asic number
        :param gain: Channel gain
        :return: the dictionary
        """

	#print("***** init HR2")
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
        _jasic["B2"]=250
        _jasic["B1"]=250
        _jasic["B0"]=250
        mask=[]
        for i in range(64):
            mask.append(1)
        _jasic["MASK0"]=mask
        _jasic["MASK1"]=mask
        _jasic["MASK2"]=mask
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
        
        dv=[]
        ct=[]
        for x in range(64):
            dv.append(gain)
            ct.append(0)
        _jasic["PAGAIN"]=dv
        _jasic["CTEST"]=ct

        return _jasic


    def HR2_unsetPowerPulsing(self):
        """
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



    def HR2_setPowerPulsing(self):
        """
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

 

    def HR2_ChangeThreshold(self,B0,B1,B2,idif=0,iasic=0):
        """
        Set the 3 thresholds of specified  asics, modified asics are tagged for upload

        :param B0: First Threshold
        :param B1: Second Threshold
        :param B2: Third Threshold        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["B0"]=B0;a["_id"]=None;
            a["slc"]["B1"]=B1;a["_id"]=None;
            a["slc"]["B2"]=B2;a["_id"]=None;

    def HR2_ChangeGain(self,idif,iasic,ipad,scale):
        """
        Scale the gain of one pad  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param ipad: Channel number
        :param scale: ratio Gain_new/Gain

        """
      

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue

            a["slc"]["PAGAIN"][ipad]=scale*a["slc"]["PAGAIN"][ipad]
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["PAGAIN"][ipad])

    def HR2_SetGain(self,idif,iasic,ipad,vnew):
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

            a["slc"]["PAGAIN"][ipad]=vnew
            a["_id"]=None
            print(idif,iasic,ipad,a["slc"]["PAGAIN"][ipad])
   

    def HR2_SetAsicGain(self,idif,iasic,vnew):
        """
        Set the gain of all pads  of specified  asics, modified asics are tagged for upload
        
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param vnew: new gain

        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in range(0,64):
                a["slc"]["PAGAIN"][ipad]=vnew
                a["_id"]=None
                print(idif,iasic,ipad,a["slc"]["PAGAIN"][ipad])



                
    def HR2_RescaleGain(self,gain0,gain1,idif=0,iasic=0):
        """
         Rescale the gain of all pads with scale=gain1/gain0  of specified  asics, modified asics are tagged for upload
        
        :param gain0: initial gain
        :param gain1: new gain
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed

        """

        scale=gain1*1./gain0
        print(" Rescale factor",scale)

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            for ipad in range(0,64):
                a["slc"]["PAGAIN"][ipad]=int(scale*a["slc"]["PAGAIN"][ipad])
            a["_id"]=None

    def HR2_slowShaper(self):
        """
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
            
    def HR2_verySlowShaper(self):
        """
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

    def HR2_ChangeCTest(self,channel,ctest,idif=0,iasic=0):
        """
         Change the CTEST value of one channel  of specified  asics, modified asics are tagged for upload
        
        :param channel: Pad tested
        :param ctest: CTEST value
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed

        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            a["slc"]["CTEST"][channel]=ctest
            a["_id"]=None


            
    def HR2_SetMask(self,list,idif=0,iasic=0):
        """
        Mask the channels specified in the list  for specified  asics, modified asics are tagged for upload
        
        :param list: List of channels to be masked
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed

        """
        m=0xFFFFFFFFFFFFFFFF
        for i in list:
            m=m &~(1<<i);
        sm="0x%lx" % m
        self.HR2_ChangeMask(sm,sm,sm,idif,iasic)
        
    def HR2_SetNewMask(self,list,idif=0,iasic=0):
        """
        Mask the channels specified in the list  for specified  asics, modified asics are tagged for upload
        
        :param list: List of channels to be masked
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed

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

    def HR2_ChangeMask(self,M0,M1,M2,idif=0,iasic=0):
        """
        Set the 3 masks  for specified  asics, modified asics are tagged for upload
        
        :param M0: Hexadecimal string of threshold 0 mask
        :param M1: Hexadecimal string of threshold 1 mask
        :param M2: Hexadecimal string of threshold 2 mask
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed

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
            
    def HR2_setEnable(self,enable,idif=0,iasic=0):
        """
        Set the ENABLE tag for specified  asics, modified asics are tagged for upload
        
        
        :param enable: Enable value (1/0)
        :param idif: DIF_ID, if 0 all DIFs are changed
        :param iasic: asic number, if 0 all Asics are changed

        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            a["slc"]["ENABLED"]=enable
            a["_id"]=None


      
def instance():
    """
    Create a MongoRoc Object

    :return: The MongoRoc Object
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
        _wdd=MongoRoc(host,port,dbname,user,pwd)
        return _wdd
    else:
        if os.path.isfile("/etc/.mongoroc.json"):
            f=open("/etc/.mongoroc.json")
            s=json.loads(f.read())
            _wdd=MongoRoc(s["host"],s["port"],s["db"],s["user"],s["pwd"])
            f.close()
            return _wdd
        else:
            return None
