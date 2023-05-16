import json
import pprint
import csv
import os
import tempfile
import pymongo
from pymongo import MongoClient
import json
import time
from bson.objectid import ObjectId
class feb_fpga_registers:
    def __init__(self):
        self.lines=[]
        self.headers= ['name', 'LEFT','MIDDLE','RIGHT','pseudo register']
        #self.defaults()
        self._id=None
    def load_defaults(self,fn="default_fpga.csv"):
        print("Loading defaults for FPGA from %s" % fn)
        self.load_from_csv_file(fn)
        return

    def store_in_db(self,dbclient):
        self.make_csv_string()
        x={}
        x["csv"]=self.csv
        result=dbclient.fpga_csv.insert_one(x)
        self._id=result.inserted_id
        return self._id
    def load_from_db(self,dbclient,bsid):
        resl=dbclient.fpga_csv.find({'_id': {'$in': [bsid]}})
                    
        for resa in resl:
            #print(resa)
            self.set_csv(resa["csv"])
            self._id=bsid
            break
        
    def set_csv(self,s_csv):
        self.csv=s_csv
        self.load_from_csv_string(s_csv)
    def load_from_csv_file(self,fname):
        f=open(fname)
        #f=open("/home/acqcmsmu/feb-backend-emulator/Python_project/FEB_config/QC_config_petiroc.csv")
        csv_reader = csv.DictReader(f, delimiter=';')
        self.lines=[]
        for r in csv_reader:
            if (len(r["name"])==0):
                continue
            #print(r["name"],r["LEFT_TOP"])
            s={}
            s["name"]=r["name"]
            for p in self.headers[1:4]:
                if len(r[p]):
                    s[p]=int(r[p],0)
                else:
                    s[p]=None
            s["pseudo register"]=(r["pseudo register"].lower()=="true")       
            self.lines.append(s)

            #pprint.pprint("self.lines="+str(self.lines))
        self.make_csv_string()
        self._id=None
    def make_csv_string(self):
        # Open the file for writing.

        tmp = tempfile.NamedTemporaryFile()

        # Open the file for writing.
        with open(tmp.name, 'w') as f:
            self.to_csv(f)
        
        self.csv=open(tmp.name).read()
        #print(tmp.name)
    def load_from_csv_string(self,s_csv):
        if (s_csv==None):
            return
        tmp = tempfile.NamedTemporaryFile()
        with open(tmp.name, 'w') as f:
             f.write(self.csv)
        #print(tmp.name)
        #v=input()
        self.load_from_csv_file(tmp.name)
    def to_csv(self,fout):
        
        hexl=["DATA_PATH_CTRL.FPGA_MIDDLE_DELAY.STEP_120MHz",
              "DATA_PATH_CTRL.MAX_QUEUE_SIZE.NB_FRAMES",
              "DATA_PATH_CTRL.MAX_READOUT_TIME_DRIFT.STEP_120MHz"]
        for i in range(16):
            hexl.append("DATA_PATH_CTRL.PAIR_TS_DIFF_MAX_STRIP%d" %i)
    
        binl=["TDC_CTRL.INJECTION_MODE","PETIROC_BOT_CTRL.TYPE_SEL"]

        with fout as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=self.headers,delimiter=";")

            writer.writeheader()
            for l in self.lines:

                cl=l.copy()
                if l["name"] in hexl:
                    for cn in self.headers[1:4]:
                        if (cl[cn]):
                            cl[cn]='{0:#x}'.format(cl[cn])
                if l["name"] in binl:
                    for cn in self.headers[1:]:
                        if (cl[cn]):
                            cl[cn]='{0:#b}'.format(cl[cn])
                writer.writerow(cl)
    
    def write_csv_file(self,fn,direc="/dev/shm/feb_csv"):
        os.system("mkdir -p %s" % direc)
        fname="%s/%s_config_fpga.csv" % (direc,fn)
        self.last_file=fname
        fout=open(fname, 'w', newline='')
        self.to_csv(open(fname, 'w', newline=''))
        fout.close()
        
    def set_modified(self):
        self._id=None

    def set_parameter(self,cn,value,fpga=None):
        for l in self.lines:
            if (l["name"]==cn):
                if fpga!=None:
                    l[fpga]=value
                else:
                    for a in self.headers[1:4]:
                        l[a]=value
        self.set_modified()
    def set_ts_offset(self,channel,value,fpga=None):
        cn="TDC_TS_CORRECTION.CHAN%d_TS_OFFSET" % channel
        for l in self.lines:
            if (l["name"]==cn):
                if fpga!=None:
                    l[fpga]=value
                else:
                    for a in self.headers[1:4]:
                        l[a]=value
        self.set_modified()
        
    def set_cmd_meas_en(self,channel,value,fpga=None):
        cn="TDC_CTRL.CMD_MEAS_EN_%d.CHAN%d" % (channel/16,channel)
        for l in self.lines:
            if (l["name"]==cn):
                if fpga!=None:
                    l[fpga]=value
                else:
                    for a in self.headers[1:4]:
                        l[a]=value
        self.set_modified()
    def set_pair_filtering_en(self,strip,value,fpga=None):
        cn="DATA_PATH_CTRL.PAIR_FILTERING_EN.STRIP%d" % strip
        for l in self.lines:
            if (l["name"]==cn):
                if fpga!=None:
                    l[fpga]=value
                else:
                    for a in self.headers[1:4]:
                        l[a]=value
        self.set_modified()
    def set_pair_ts_diff_min(self,strip,value,fpga=None):
        cn="DATA_PATH_CTRL.PAIR_TS_DIFF_MIN_STRIP%d" % strip
        for l in self.lines:
            if (l["name"]==cn):
                if fpga!=None:
                    l[fpga]=value
                else:
                    for a in self.headers[1:4]:
                        l[a]=value
        self.set_modified()
        
    def set_pair_ts_diff_max(self,strip,value,fpga=None):
        cn="DATA_PATH_CTRL.PAIR_TS_DIFF_MAX_STRIP%d" % strip
        for l in self.lines:
            if (l["name"]==cn):
                if fpga!=None:
                    l[fpga]=value
                else:
                    for a in self.headers[1:4]:
                        l[a]=value
        self.set_modified()

class feb_petiroc_registers:
    def __init__(self):
        self.lines=[]
        self.headers=["name","LEFT_TOP","LEFT_BOT","MIDDLE_TOP","MIDDLE_BOT","RIGHT_TOP","RIGHT_BOT"]
        #self.defaults()
        self._id=None
    def load_defaults(self,fn="default_petiroc.csv"):
        print("Loading defaults for PETIROC from %s" % fn)
        self.load_from_csv_file(fn)
        return
    def store_in_db(self,dbclient):
        self.make_csv_string()
        x={}
        x["csv"]=self.csv
        result=dbclient.petiroc_csv.insert_one(x)
        self._id=result.inserted_id
        return self._id
    def load_from_db(self,dbclient,bsid):
        resl=dbclient.petiroc_csv.find({'_id': {'$in': [bsid]}})
                    
        for resa in resl:
            #print(resa)
            self.set_csv(resa["csv"])
            self._id=bsid
            break
    
    def set_csv(self,s_csv):
        self.csv=s_csv
        self.load_from_csv_string(s_csv)

    def load_from_csv_file(self,fname):
        f=open(fname)
        #f=open("/home/acqcmsmu/feb-backend-emulator/Python_project/FEB_config/QC_config_petiroc.csv")
        csv_reader = csv.DictReader(f, delimiter=';')
        self.lines=[]
        for r in csv_reader:
            if (len(r["name"])==0):
                continue
            #print(r["name"],r["LEFT_TOP"])
            s={}
            s["name"]=r["name"]
            for p in self.headers[1:]:
                if len(r[p]):
                    s[p]=int(r[p],0)
                else:
                    s[p]=None
                    
            self.lines.append(s)

            #pprint.pprint("self.lines="+str(self.lines))
        self.make_csv_string()
        self._id=None
    def make_csv_string(self):
        # Open the file for writing.

        tmp = tempfile.NamedTemporaryFile()

        # Open the file for writing.
        with open(tmp.name, 'w') as f:
            self.to_csv(f)
        
        self.csv=open(tmp.name).read()
        #print(tmp.name)
    def load_from_csv_string(self,s_csv):
        if (s_csv==None):
            return
        tmp = tempfile.NamedTemporaryFile()
        with open(tmp.name, 'w') as f:
             f.write(self.csv)
        #print(tmp.name)
        #v=input()
        self.load_from_csv_file(tmp.name)
    def to_csv(self,fout):
        hexl=["input_dac_ch_dummy"]
        for i in range(32):
            hexl.append("input_dac_ch%d" %i)
            hexl.append("6b_dac_ch%d" %i)
        binl=["pa_ccomp"]
        with fout as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=self.headers,delimiter=";")

            writer.writeheader()
            for l in self.lines:

                cl=l.copy()
                if l["name"] in hexl:
                    for cn in self.headers[1:]:
                        if (cl[cn]):
                            cl[cn]='{0:#x}'.format(cl[cn])
                if l["name"] in binl:
                    for cn in self.headers[1:]:
                        if (cl[cn]):
                            cl[cn]='{0:#b}'.format(cl[cn])
                writer.writerow(cl)
    
    def write_csv_file(self,fn,direc="/dev/shm/feb_csv"):
        os.system("mkdir -p %s" % direc)
        fname="%s/%s_config_petiroc.csv" % (direc,fn)
        self.last_file=fname
        fout=open(fname, 'w', newline='')
        self.to_csv(open(fname, 'w', newline=''))
        fout.close()

    def set_modified(self):
        self._id=None
    def set_mask_discri_time(self,channel,value,asic=None):
        cn="mask_discri_time_ch%d" % channel
        for l in self.lines:
            if (l["name"]==cn):
                if asic!=None:
                    l[asic]=value
                else:
                    for a in self.headers[1:]:
                        l[a]=value
        self.set_modified()
    def set_input_dac(self,channel,value,asic=None):
        cn="input_dac_ch%d" % channel
        for l in self.lines:
            if (l["name"]==cn):
                if asic!=None:
                    l[asic]=value
                else:
                    for a in self.headers[1:]:
                        l[a]=value
        self.set_modified()
    def set_cmd_input_dac(self,channel,value,asic=None):
        cn="cmd_input_dac_ch%d" % channel
        for l in self.lines:
            if (l["name"]==cn):
                if asic!=None:
                    l[asic]=value
                else:
                    for a in self.headers[1:]:
                        l[a]=value
        self.set_modified()
    def set_6b_dac(self,channel,value,asic=None):
        cn="6b_dac_ch%d" % channel
        for l in self.lines:
            if (l["name"]==cn):
                if asic!=None:
                    l[asic]=value
                else:
                    for a in self.headers[1:]:
                        l[a]=value
        self.set_modified()
    def shift_10b_dac(self,dth,asic=None):
        cn="10b_dac_vth_discri_time"
        for l in self.lines:
            if (l["name"]==cn):
                if asic!=None:
                    l[asic]=l[asic]+dth
                else:
                    for a in self.headers[1:]:
                        print(a,l[a],l[a]+dth)
                        l[a]=l[a]+dth

        self.set_modified()

    def correct_6b_dac(self,channel,cor,asic):
        cn="6b_dac_ch%d" % channel
        for l in self.lines:
            if (l["name"]==cn):
                #print(l)
                l[asic]=l[asic]+cor
        self.set_modified()
    def set_parameter(self,cn,value,asic=None):
        for l in self.lines:
            if (l["name"]==cn):
                if asic!=None:
                    l[asic]=value
                else:
                    for a in self.headers[1:]:
                        l[a]=value
        self.set_modified()
    def get_parameter(self,cn,asic):
        for l in self.lines:
            if (l["name"]==cn):
                return l[asic.upper()]
    def get_6b_dac(self,asic):
        v=[]
        for channel in range(32):
            cn="6b_dac_ch%d" % channel
            for l in self.lines:
                if (l["name"]==cn):
                    v.append(l[asic.upper()])
        return v
class febv2_registers:
    def __init__(self,f_id=0,v_fpga="3.13",v_asic="2C"):
        self.feb_id=f_id
        self.fpga=feb_fpga_registers()
        self.petiroc=feb_petiroc_registers()
        self.fpga_version=v_fpga
        self.petiroc_version=v_asic
        self._id=None
    def load_defaults(self,fna="default_fpga.csv",fnp="default_petiroc.csv"):
        self.fpga.load_defaults(fna)
        self.petiroc.load_defaults(fnp)
        return
    def store_in_db(self,dbclient):
        x={}
        x["feb_id"]=self.feb_id
        x["fpga_version"]=self.fpga_version
        x["petiroc_version"]=self.petiroc_version
        if (self.fpga._id==None):
            x["fpga_bsid"]=self.fpga.store_in_db(dbclient)
            self._id=None
        else:
            x["fpga_bsid"]=self.fpga._id
        if (self.petiroc._id==None):
            x["petiroc_bsid"]=self.petiroc.store_in_db(dbclient)
            self._id=None
        else:
            x["petiroc_bsid"]=self.petiroc._id

        if (self._id==None):
            result=dbclient.febv2.insert_one(x)
            self._id=result.inserted_id
        return self._id
    def has_changes(self):
        return ((self._id==None) or (self.fpga._id==None) or (self.petiroc._id==None))
    def load_from_db(self,dbclient,bsid):
        self._id=bsid
        resl=dbclient.febv2.find({'_id': {'$in': [bsid]}})
                    
        for resa in resl:
            #print(resa)
            self.fpga_version=resa["fpga_version"]
            self.petiroc_version=resa["petiroc_version"]
            self.fpga.load_from_db(dbclient,resa["fpga_bsid"])
            self.petiroc.load_from_db(dbclient,resa["petiroc_bsid"])
            self.feb_id=resa["feb_id"]
            break
    def set_feb_id(self,f_id):
        self.feb_id=f_id
        self._id=None
    def set_fpga_version(self,v):
        self.fpga_version_id=v
        self._id=None
    def set_petiroc_version(self,v):
        self.petiroc_version=v
        self._id=None

    def to_csv_files(self,name,version):
        fn="%s_%d_f_%d" % (name,version,self.feb_id)
        self.fpga.write_csv_file(fn)
        self.petiroc.write_csv_file(fn)
        
class febv2_setup:
    def __init__(self,name=None,version=0):
        self.name=name
        self.version=version
        self.last_version=version
        self.febs=[]
        self._id=None
    def add_febv2(self,feb):
        self.febs.append(feb)
    def has_changes(self):
        changes=False
        for f in self.febs:
            changes=changes or f.has_changes()
        return changes
        
    def store_in_db(self,dbclient,comment):
        #
        if not self.has_changes():
            print("No changes in setup %s %d , exiting " % (self.name,self.version))
            return
        self._id=None
        # Find last version
        res=dbclient.febv2_setup.find({'name':self.name})
        last=0
        for x in res:
            if (last<x["version"]):
                last=x["version"]
        if (last==0):
            print(" No state ",self.name," found, it will be created")
        else:
            print("new version created %s %d " % (self.name,last+1))

        x={}
        x["name"]=self.name
        x["comment"]=comment
        x["last_version"]=self.version
        x["version"]=last+1
        x["febs_id"]=[]
        for f in self.febs:
            x["febs_id"].append(f.store_in_db(dbclient))
        
        
        if (self._id==None):
            result=dbclient.febv2_setup.insert_one(x)
            self._id=result.inserted_id
            self.version=x["version"]
        return self._id
    def load_from_db(self,dbclient,name,version):
        res=dbclient.febv2_setup.find({'name':name,'version':version})
        for resa in res:
            self._id=resa["_id"]
            #print(resa)
            self.name=name
            self.version=version
            self.febs=[]
            for fid in resa["febs_id"]:
                f=febv2_registers()
                f.load_from_db(dbclient,fid)
                self.febs.append(f)
    def to_csv_files(self):
        for f in self.febs:
            f.to_csv_files(self.name,self.version)


class mgdb_feb:
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

        if (pymongo.version_tuple[0]<4):
            self.connection=MongoClient(host,port)
            self.db=self.connection[dbname]
            self.db.authenticate(username,pwd)
        else:
            self.connection=MongoClient(host,port,username=username,password=pwd,authSource=dbname)
            self.db=self.connection[dbname]




        
        self.setup=None
        self.asiclist = []
        self.bson_id=[]
    def create_setup(self,name):
        self.setup=febv2_setup(name)
        
    def download_setup(self,name,version):
        self.setup=febv2_setup(name,version)
        self.setup.load_from_db(self.db,name,version)
    def to_csv_files(self):
        self.setup.to_csv_files()
    def upload_changes(self,comment):
        if (self.setup!=None):
            self.setup.store_in_db(self.db,comment)
        else:
            print("No setup defined")
    def list_setups(self):
        res=self.db.febv2_setup.find({})
        for resa in res:
            print(resa["name"],"/",resa["version"],"|",resa["comment"],"| Previous ",resa["last_version"])
    def list_tests(self):
        res=self.db.febv2_tests.find({})
        for resa in res:
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(resa["ctime"]))
            if "asic" in resa:
                print(f'{sti}|{resa["state"]}/{resa["version"]}/{resa["analysis"]}|{resa["feb"]}|{resa["asic"]}|{resa["comment"]}')
            if "fpga" in resa:
                print(f'{sti}|{resa["state"]}/{resa["version"]}/{resa["analysis"]}|{resa["feb"]}|{resa["fpga"]}|{resa["comment"]}')
            #print(f'{sti}|{resa["state"]}/{resa["version"]}/{resa["analysis"]}|{resa["feb"]}|{resa["comment"]}')
                
    def upload_results(self,state,version,feb,analysis,res,comment=None):
        analysisType=["SCURVE_1","SCURVE_A","TIME_PEDESTAL","NOISE"]
        if not (analysis in analysisType):
            print(f"Analysis type is {analysis} not in {analysisType}")
            return
        res["state"]=state
        res["version"]=version
        res["feb"]=feb
        res["analysis"]=analysis
        if (comment!=None):
            
            res["comment"]=comment
            res["ctime"]=time.time()

            result=self.db.febv2_tests.insert_one(res)

        else:
            print(res)
    def get_scurve(self,state,version,feb,analysis,asic,channel=None):
        res=self.db.febv2_tests.find({'state':state,'version':version,"feb":feb,"analysis":analysis,"asic":asic})

        results=[]
        for resa in res:
            results.append(resa)
        if (channel==None):
            return results[len(results)-1]
        else:
            for x in results[len(results)-1]["channels"]:
                if (x["prc"]==channel):
                    return x
    def get_time_pedestal(self,state,version,feb,analysis,fpga,channel=None):
        print(state,version,feb,analysis,fpga,channel)
        res=self.db.febv2_tests.find({'state':state,'version':version,"feb":feb,"analysis":analysis,"fpga":fpga})

        results=[]
        for resa in res:
            results.append(resa)
        if (channel==None):
            return results[len(results)-1]
        else:
            return results[len(results)-1][channel]
    def getRun(self,location,comment="Not set"):
        """
        Get a new run number for a given setup

        :param location: Setup Name
        :param comment: Comment on the run
        :return: a dictionnary corresponding to the base insertion {run,location,time,comment}
        """
        res=self.db.runs.find({'location':location})
        runod={}
        for x in res:
            #print(x["location"],x["run"],x["comment"])
            #var=raw_input()
            runod=x
        runnb=1000
        if ("location" in runod):
            runnb=runod["run"]+1
        runid={}
        runid["run"]=runnb
        runid["location"]=location
        runid["time"]=time.time()
        runid["comment"]=comment
        os.system("mkdir -p /dev/shm/mgjob")
        fname="/dev/shm/mgjob/lastrun.json"
        f=open(fname,"w+")
        f.write(json.dumps(runid, indent=2, sort_keys=True))
        f.close()
        resconf=self.db.runs.insert_one(runid)
        print(resconf)
        return runid
    def updateRun(self,run,loc,tag,vtag):
        filter = { 'run': run,'location':loc }
 
        # Values to be updated.
        newvalues = { "$set": { tag: vtag } }
 
        # Using update_one() method for single
        # updation.
        print(filter,newvalues)
        self.db.runs.update_one(filter, newvalues)
    def runs(self):
        """
        List all the run informations stored
        """
        res=self.db.runs.find({})
        for x in res:
            #print(x)
            if ("run" in x):
                if ("comment" in x and "time" in x and "P" in x):
                    print(time.ctime(x["time"]),x["location"],x["run"],x["P"],x["comment"])
                    continue
                if ("comment" in x and "time" in x):
                    print(time.ctime(x["time"]),x["location"],x["run"],x["comment"])
                else:
                    if ("run" in x):
                        print(x["location"],x["run"],x["comment"])
                #print(x["time"],x["location"],x["run"],x["comment"])
    def configurations(self):
        """
        List all the configurations stored
        """
        cl=[]
        res=self.db.configurations.find({})
        for x in res:
            if ("comment" in x):
                print(time.ctime(x["time"]),x["version"],x["name"],x["comment"])
                cl.append((x["name"],x['version'],x['comment']))
        return cl
    def upload_configuration(self,fname,comment):
        """
        jobcontrol configuration upload

        :param fname: File name to upload
        :param comment: A comment on the configuration
        """
        s={}
        s["content"]=json.loads(open(fname).read())
        s["name"]=s["content"]["name"]
        s["time"]=time.time()
        s["comment"]=comment
        s["version"]=s["content"]["version"]
        resconf=self.db.configurations.insert_one(s)
        print(resconf)
    def download_configuration(self,cname,version,toFileOnly=False):
        """
        Download a jobcontrol configuration to /dev/shm/config/ directory
        
        :param cname: Configuration name
        :param version: Configuration version
        :param toFileOnly:if True and /dev/shm/config/cname_version.json exists, then it exits
        """
        os.system("mkdir -p /dev/shm/config")
        fname="/dev/shm/config/%s_%s.json" % (cname,version)
        if os.path.isfile(fname) and toFileOnly:
            #print('%s already download, Exiting' % fname)
            return
        print("Looking for ",cname,version)
        res=self.db.configurations.find({'name':cname,'version':version})
        for x in res:
            print(x["name"],x["version"],x["comment"])
            #var=raw_input()
            slc=x["content"]
            os.system("mkdir -p /dev/shm/config")
            fname="/dev/shm/config/%s_%s.json" % (cname,version)
            f=open(fname,"w+")
            f.write(json.dumps(slc, indent=2, sort_keys=True))
            f.close()
            return slc
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
        _wdd=mgdb_feb(host,port,dbname,user,pwd)
        return _wdd
    else:
        if os.path.isfile("/etc/.mongoroc.json"):
            f=open("/etc/.mongoroc.json")
            s=json.loads(f.read())
            _wdd=mgdb_feb(s["host"],s["port"],s["db"],s["user"],s["pwd"])
            f.close()
            return _wdd
        else:
            print("missing access to DB")
            return None

if __name__ == "__main__":

    p=feb_petiroc_registers()
    p.load_defaults()
    p.set_mask_discri_time(1,1)
    p.set_mask_discri_time(2,1,"MIDDLE_TOP")
    p.set_input_dac(2,0x4)
    p.set_6b_dac(3,23)
    p.set_parameter("10b_dac_vth_discri_time",320,"LEFT_BOT")
    p.set_parameter("pa_ccomp",0b0111,"LEFT_TOP")
    p.write_csv_file("essai_3")
    
    pf=feb_fpga_registers()
    pf.load_defaults()
    pf.set_pair_ts_diff_max(3,12000,"RIGHT")
    pf.write_csv_file("essai_fpga_2")
