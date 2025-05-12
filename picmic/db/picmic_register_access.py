"""
 Defines the PICMIC description classes and tools to access them.

@file csv_register_access.py

@brief Defines the MongoDb classes and tools to store and access PICMIC parameters.

@section database objects 
Defines the classes to handle PICMIC collections data.
 - picmic_registers
 - picmic_setup
@section Access and tools
 - mg_picboard
"""
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
import re
from collections import defaultdict

class picmic_parameters:
    def __init__(self):
        self.data = defaultdict(dict)
        self.liroc = defaultdict(dict)
        self.liroc["LIROC"]="LIROC"
        self.pico = defaultdict(dict)
        self.pico["PTDC"]="PTDC"
        self.exceptions = {'EN_NOR64', 'channel_split2','channel_split4','shift_clk1G28','header_fields0','header_fields1','header_fields2','header_fields3'}
        self.digit={}
        self._id=None
    def parse_csv_file(self,file_name,a_type=None):
        if (a_type == None):
            dico=self.data
        if (a_type == "LIROC"):
            dico=self.liroc
        if (a_type == "PICO"):
            dico=self.pico
            
        with open(file_name, newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                key = row['Parameter']
                value = row['Value']
                if (key == "LIROC" or key == "PTDC"):
                    dico[key]=value
                # Skip non-numeric and non-convertible values
                try:
                    if isinstance(value, str):
                        lvalue=value.lower()
                        #print(value,lvalue)
                        #input()
                        if lvalue.startswith("0b"):
                            num_value = int(value, 2)
                            self.digit[key]="b"
                        elif lvalue.startswith("0x"):
                            num_value = int(value, 16)
                            self.digit[key]="x"
                        elif lvalue.startswith("0o"):
                            num_value = int(value, 8)
                            self.digit[key]="o"
                        else:
                            num_value = int(value)
                            self.digit[key]="d"
                    else:
                        num_value = int(value)
                        self.digit[key]="d"
                except ValueError:
                    continue

                # Match ending digit (e.g. ch0, ch1), with exceptions
                if key not in self.exceptions:
                    match = re.match(r'^(.*?)(?:_ch)?(\d+)$', key)
                    if match:
                        base_name, index = match.groups()
                        index = int(index)
                        if base_name not in dico:
                            dico[base_name] = []
                            self.digit[base_name]=self.digit[key]
                        # Ensure list is large enough
                        while len(dico[base_name]) <= index:
                            dico[base_name].append(None)
                        dico[base_name][index] = num_value
                        continue

                # Default case: treat as single value
                dico[key] = num_value
        if (a_type !=None):
            self.data={**self.liroc,**self.pico}
    def get(self, name, index=None):
        if index is not None:
            return self.data.get(name, [])[index]
        return self.data.get(name)

    def set(self, name, value, index=None):
        if index is not None:
            if name not in self.data or not isinstance(self.data[name], list):
                self.data[name] = []
            while len(self.data[name]) <= index:
                self.data[name].append(None)
            self.data[name][index] = value
        else:
            self.data[name] = value
        self.set_modified()

    def to_json(self, output_path,asic=None):
        if asic == None :
            dico=self.data
        if asic == "LIROC" :
            dico=self.liroc
        if asic == "PICO" :
            dico=self.pico
        f=open(output_path,"w+")
        f.write(json.dumps(dico, indent=2, sort_keys=True))
        f.close()
    def load_liroc(self,json_liroc):
        f=open(json_liroc)
        s=json.loads(f.read())
        self.liroc={**self.liroc,**s}
        f.close()
        self.data={**self.liroc,**self.pico}
    def load_pico(self,json_pico):
        f=open(json_pico)
        s=json.loads(f.read())
        self.pico={**self.pico,**s}
        f.close()
        self.data={**self.liroc,**self.pico}
    def to_csv(self, output_path,asic=None):
        if asic == None :
            dico=self.data
        if asic == "LIROC" :
            dico=self.liroc
        if asic == "PICO" :
            dico=self.pico
            
            
        with open(output_path, mode='w', newline='') as csvfile:
            writer = csv.writer(csvfile,quoting=csv.QUOTE_NONNUMERIC)
            writer.writerow(['Parameter', 'Value'])
            for key, val in dico.items():
                if not key in self.digit.keys() and not isinstance(val, list):
                    writer.writerow([key, val])
                    continue
                if isinstance(val, list):
                    for i, v in enumerate(val):
                        if v is not None:
                            if not key in self.digit.keys():
                                writer.writerow([f"{key}_ch{i}", v])
                                continue
                            if (self.digit[key]=="d"):
                                writer.writerow([f"{key}_ch{i}", v])
                            if (self.digit[key]=="b"):
                                writer.writerow([f"{key}_ch{i}", '{0:#b}'.format(v)])
                            if (self.digit[key]=="x"):
                                writer.writerow([f"{key}_ch{i}", '{0:#x}'.format(v)])
                            if (self.digit[key]=="o"):
                                writer.writerow([f"{key}_ch{i}", '{0:#o}'.format(v)])
                else:
                    if (self.digit[key]=="d"):
                        writer.writerow([key, val])
                    if (self.digit[key]=="b"):
                        writer.writerow([key, '{0:#b}'.format(val)])
                    if (self.digit[key]=="x"):
                        writer.writerow([key, '{0:#x}'.format(val)])
                    if (self.digit[key]=="o"):
                        writer.writerow([key, '{0:#o}'.format(val)])
                    
    def store_in_db(self,dbclient):
        """ Store parameter in picotdc_csv collection, given a MongoClient access
        Args:
            dbclient: The mongodb client access
        Returns:
            the bson id of the insertion
        """
        self.make_csv_string()
        x={}
        x["csv"]=self.csv_str
        result=dbclient.picmic_csv.insert_one(x)
        self._id=result.inserted_id
        return self._id
    
    def load_from_db(self,dbclient,bsid):
        """ Load parameter from picmic_csv collection, given a MongoClient access and an id
        Args:
            dbclient: The mongodb client access
            bsid: The bson id in mongo db collection
        """       
        resl=dbclient.picmic_csv.find({'_id': {'$in': [bsid]}})
                    
        for resa in resl:
            #print(resa)
            self.set_csv(resa["csv"])
            self._id=bsid
            break
        
    def set_csv(self,s_csv):
        """Initialise object from the csv string
        Args:
            s_csv(str): The csv string 
        """
        self.csv_str=s_csv
        self.load_from_csv_string(s_csv)

    def make_csv_string(self):
        """ Build the csv string from the lines object
        """
        # Open the file for writing.

        tmp = tempfile.NamedTemporaryFile()

        # Open the file for writing.
        #with open(tmp.name, 'w') as f:
        self.to_csv(tmp.name)
        
        self.csv_str=open(tmp.name).read()
        #print(tmp.name)

    def load_from_csv_string(self,s_csv):
        """ Load the csv lines from the csv string
        Args:
            s_csv(str): A string containing csv data
        """
        if (s_csv==None):
            return
        tmp = tempfile.NamedTemporaryFile()
        with open(tmp.name, 'w') as f:
             f.write(self.csv_str)
        #print(tmp.name)
        #v=input()
        self.load_from_csv_file(tmp.name)

    def load_from_csv_file(self,fname):
        self.parse_csv_file(fname)
        self.make_csv_string()
    def write_csv_file(self,fn,direc="/dev/shm/board_csv"):
        """ Write lines in a file
        Args:
            fn(str): File name
            direc(str): directory  (default /dev/shm/board_csv)
        """
        os.system("mkdir -p %s" % direc)
        fname="%s/%s_config_picmic.csv" % (direc,fn)
        self.last_file=fname
        #fout=open(fname, 'w', newline='')
        #self.to_csv(open(fname, 'w', newline=''))
        self.to_csv(fname)
        #fout.close()
    def set_modified(self):
        """ Tag the object as modified, ie, one line was at least changed
        """
        self._id=None




class picmic_registers:
    """ Handlers of thecsv objects per board
    """
    def __init__(self,f_id=0,v_picmic="1.0"):
        """
        Initialise a picmic_registers object
        Args:
            f_id (int): The board number
            v_picmic(str): Version of the picmic firmware (4.0 default)
        """
        self.board_id=f_id
        self.picmic=picmic_parameters()
        self.picmic_version=v_picmic
        self._id=None
    def load_defaults(self,fnp="default_pico.csv",fnl="default_liroc.csv"):
        """ Load values of PICOTDC and LIROC csv
        Args:
            fnp(str): File name of the picotdc csv to be used as default 
            fnl(str): File name of the LIROC csv to be used as default
        """
        self.picmic.parse_csv_file(fnl,"LIROC")
        self.picmic.parse_csv_file(fnp,"PICO")
        return
    def store_in_db(self,dbclient):
        """ Store the picmic_register in the picmic collection
        Args:
            dbclient: MongoDB client access
        Returns:
            bson id of the document
        """
        x={}
        print(self.picmic_version)
        x["board_id"]=self.board_id
        x["picmic_version"]=self.picmic_version
        if (self.picmic._id==None):
            x["picmic_bsid"]=self.picmic.store_in_db(dbclient)
            self._id=None
        else:
            x["picmic_bsid"]=self.picmic._id
       
        if (self._id==None):
            result=dbclient.picmic.insert_one(x)
            self._id=result.inserted_id
        return self._id
    def has_changes(self):
        """ Check if one value has changed
        Returns:
            True if one of the csv object or firmware version has change
        """
        return ((self._id==None) or (self.picmic._id==None) )
    def load_from_db(self,dbclient,bsid):
        """ Load one board from the DB collection picmic
        Args:
            dbclient: MongoDB client access
            bsid : Bson id of the docuement in DB collection
        """
        self._id=bsid
        resl=dbclient.picmic.find({'_id': {'$in': [bsid]}})
                    
        for resa in resl:
            #print(resa)
            self.picmic_version=resa["picmic_version"]
            self.picmic.load_from_db(dbclient,resa["picmic_bsid"])
            self.board_id=resa["board_id"]
            break
    def set_board_id(self,f_id):
        """ Change board id
        Args:
            f_id (int): new board id
        """
        self.board_id=f_id
        self._id=None
    def set_picmic_version(self,v):
        """ Change picmic version
        Args:
            v(str): picmic version
        """
        self.picmic_version=v
        self._id=None

    def to_csv_files(self,name,version):
        """ Writes the 2 csv files to disK (name_version_picmic/petiroc.csv)
        Args:
            name(str) : State name
            version(int): State version
        """
        fn="%s_%d_f_%d" % (name,version,self.board_id)
        self.picmic.write_csv_file(fn)
        
class picmic_setup:
    """
    It handles all picmic_registers object of a given setup and have 
    interfaces method to store/load them
    """
    def __init__(self,name=None,version=0):
        """ Initialisation
        Args:
            name(str): Name of the state
            version(int): version number (0 by default)
        Returns:
            An instance of picmic_setup    
        """
        self.name=name
        self.version=version
        self.last_version=version
        self.boards=[]
        self._id=None
    def add_picmic(self,board):
        """ Add a picmic_register object to the list of board
        Args:
            board: picmic_register object
        """
        self.boards.append(board)
    def has_changes(self):
        """ Check if there is any changes ins setup
        Returns: 
            true if one of the picmic_registers object has changes
        """
        changes=False
        for f in self.boards:
            changes=changes or f.has_changes()
        return changes
        
    def store_in_db(self,dbclient,comment):
        """ Store the setup in db
        Args:
            dbclient: MongoDb Client access
            comment(str): A comment for the update
        Returns:
            The bson id of the insertion    
        """
        #
        if not self.has_changes():
            print("No changes in setup %s %d , exiting " % (self.name,self.version))
            return
        self._id=None
        # Find last version
        res=dbclient.picmic_setup.find({'name':self.name})
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
        x["boards_id"]=[]
        for f in self.boards:
            x["boards_id"].append(f.store_in_db(dbclient))
        
        
        if (self._id==None):
            result=dbclient.picmic_setup.insert_one(x)
            self._id=result.inserted_id
            self.version=x["version"]
        return self._id
    def load_from_db(self,dbclient,name,version):
        """
        Fill the boards list with boards in a given setup
        Args:
            dbclient: databas MongoClient access
            name: Setup name
            version (int): Version number
        """
        res=dbclient.picmic_setup.find({'name':name,'version':version})
        for resa in res:
            self._id=resa["_id"]
            #print(resa)
            self.name=name
            self.version=version
            self.boards=[]
            for fid in resa["boards_id"]:
                f=picmic_registers()
                f.load_from_db(dbclient,fid)
                self.boards.append(f)
    def to_csv_files(self):
        """
        Store the boards csv data to files in /dev/shm/board_csv with names
        setup_version_picmic/petiroc.csv
        """
        for f in self.boards:
            f.to_csv_files(self.name,self.version)


class mg_picboard:
    """
    Main class to access the Mongo DB 
    It gives access to all collections (picmic_setup,picmic_test,configurations,runs)
    """

    def __init__(self, host,port,dbname,username,pwd,auth=None):
        """
        connect Mongodb database 

        Args:

            host: Hostanme of the PC running the mongo DB
            port (int): Port to access the base
            dbname: Data base name
            username: Remote access user
            pwd: Remote access password

        """

        if (pymongo.version_tuple[0]<3):
            self.connection=MongoClient(host,port)
            self.db=self.connection[dbname]
            self.db.authenticate(username,pwd)
        else:
            if (auth==None):
                self.connection=MongoClient(host,port,username=username,password=pwd,authSource=dbname)
            else:
                self.connection=MongoClient(host,port,username=username,password=pwd,authSource=auth)

            self.db=self.connection[dbname]




        
        self.setup=None
    def create_setup(self,name):
        """
        Create a new empty setup 

        Args:
            name: Setup name
        """
        self.setup=picmic_setup(name)
        
    def download_setup(self,name,version):
        """
        Load from the picmic_setup collection the state with given name and version

        Args:
            name (str): Setup name
            version (int): Setup version
        """
        self.setup=picmic_setup(name,version)
        self.setup.load_from_db(self.db,name,version)
    def to_csv_files(self):
        """
        Save the current setup values in /dev/shm/board_csv directory with name
        state_version_picmic.csv and state_version_petiroc.csv
        """
        self.setup.to_csv_files()
    def upload_changes(self,comment):
        """
        Upload changes in the last download setup and generates the next version number
        Args:
            comment (str): The comment for the new version
        """
        if (self.setup!=None):
            self.setup.store_in_db(self.db,comment)
        else:
            print("No setup defined")
    def list_setups(self):
        """
        Print the list of states in the picmic_setup collection
        """
        res=self.db.picmic_setup.find({})
        for resa in res:
            print(resa["name"],"/",resa["version"],"|",resa["comment"],"| Previous ",resa["last_version"])
    def list_tests(self):
        """
        Print the list of test in the picmic_test collection
        """
        res=self.db.picmic_tests.find({})
        for resa in res:
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(resa["ctime"]))
            print(f'{sti}|{resa["state"]}/{resa["version"]}/{resa["analysis"]}|{resa["board"]}|{resa["comment"]}')
            #print(f'{sti}|{resa["state"]}/{resa["version"]}/{resa["analysis"]}|{resa["board"]}|{resa["comment"]}')
                
    def upload_results(self,state,version,board,analysis,res,comment=None):
        """
        Store test results for a given analysis in picmic_test collection

        Args:
            state (str): DB state used for the test
            version (int): DB version used for the test
            board (int): PICMIC id
            analysis (str): the analysis type (SCURVE_1,SCURVE_A or TIME_PEDESTAL)
            res: An dictionnary conatining test results (histograms)       
        """
        analysisType=["SCURVE_1","SCURVE_A","TIME_PEDESTAL","NOISE"]
        if not (analysis in analysisType):
            print(f"Analysis type is {analysis} not in {analysisType}")
            return
        res["state"]=state
        res["version"]=version
        res["board"]=board
        res["analysis"]=analysis
        if (comment!=None):
            
            res["comment"]=comment
            res["ctime"]=time.time()

            result=self.db.picmic_tests.insert_one(res)

        else:
            print(res)
    def get_scurve(self,state,version,board,analysis,channel=None):
        """
        Get the last stored test results of a SCURVE_1 or SCURVE_A analysis

        Args:
            state (str): DB state used for the test
            version (int): DB version used for the test
            board (int): PICMIC id
            analysis (str): the analysis type (SCURVE_1 or SCURVE_A)
            channel (int): Optionnal , if set only this channel is return
        Returns:
            The last scurves histograms inserted 
        """
        res=self.db.picmic_tests.find({'state':state,'version':version,"board":board,"analysis":analysis})

        results=[]
        for resa in res:
            results.append(resa)
        if (channel==None):
            return results[len(results)-1]
        else:
            for x in results[len(results)-1]["channels"]:
                if (x["prc"]==channel):
                    return x
    
    def getRun(self,location,comment="Not set"):
        """
        Get a new run number for a given setup

        Args:
            location: Setup Name
            comment: Comment on the run
        Returns:
            a dictionnary corresponding to the base insertion {run,location,time,comment}
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
        """
        Update run information

        Args:
            run (int): The run number
            loc (str): The setup name
            tag (str): The tag in the collection runs to be updated
            vtag (str): The value of the tag
        """
        filter = { 'run': run,'location':loc }
 
        # Values to be updated.
        newvalues = { "$set": { tag: vtag } }
 
        # Using update_one() method for single
        # updation.
        print(filter,newvalues)
        self.db.runs.update_one(filter, newvalues)
    def runs(self):
        """
        List all the run informations stored in the runs collection
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
        List all the process (pmdaq) configurations stored in the configurations collection

        Returns:
            A List of tuplet with (name,version,comment)
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
        Upload a process configuration for pmdaq
        The version number is set in the file 

        Args:
            fname: File name to  be uploaded
            comment: A comment on the configuration
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
        
        Args:
            cname (str): Configuration name
            version (int): Configuration version
            toFileOnly (bool):if True and /dev/shm/config/cname_version.json exists, then it exits

        Returns:
            slc, the slow control JSON content written in the file
        
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
    Create an instance of the mg_picboard Object

    Returns:
        The mg_picboard Object
    """
    # create the default access
    login=os.getenv("MGDBLOGIN","NONE")
    if (login != "NONE"):
        list_info=login.split("@")
        userinfo = list_info[0]
        hostinfo = list_info[1]
        dbname = list_info[2]
        auth=None
        if (len(list_info)==4):
            auth=list_info[3]
        user=userinfo.split("/")[0]
        pwd=userinfo.split("/")[1]
        host=hostinfo.split(":")[0]
        port=int(hostinfo.split(":")[1])
        #print("MGROC::INSTANCE() ",host,port,dbname,user,pwd)
        _wdd=mg_picboard(host,port,dbname,user,pwd,auth)
        return _wdd
    else:
        if os.path.isfile("/etc/.mongoroc.json"):
            f=open("/etc/.mongoroc.json")
            s=json.loads(f.read())
            _wdd=mg_picboard(s["host"],s["port"],s["db"],s["user"],s["pwd"])
            f.close()
            return _wdd
        else:
            print("missing access to DB")
            return None

if __name__ == "__main__":

    p=liroc_registers()
    p.load_defaults()
    p.set_mask_discri_time(1,1)
    p.set_mask_discri_time(2,1,"MIDDLE_TOP")
    p.set_input_dac(2,0x4)
    p.set_6b_dac(3,23)
    p.set_parameter("10b_dac_vth_discri_time",320,"LEFT_BOT")
    p.set_parameter("pa_ccomp",0b0111,"LEFT_TOP")
    p.write_csv_file("essai_3")
    
    pf=picmic_registers()
    pf.load_defaults()
    pf.set_pair_ts_diff_max(3,12000,"RIGHT")
    pf.write_csv_file("essai_picmic_2")
