#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
from bson.objectid import ObjectId
import time





def IP2Int(ip):
    """!
    convert IP adress string to int

    @param IP: the IP address
    @return: the encoded integer 
    """
    o = list(map(int, ip.split('.')))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res


class MongoJob:
    """!
    Main class to access the Mongo DB 
    """

    def __init__(self, host,port,dbname,username,pwd):
        """!
        connect Mongodb database 

        @param host: Hostanme of the PC running the mongo DB
        @param port: Port to access the base
        @param dbname: Data base name
        @param username: Remote access user
        @param pwd: Remote access password

        """
        if (pymongo.version_tuple[0]<4):
            self.connection=MongoClient(host,port)
            self.db=self.connection[dbname]
            self.db.authenticate(username,pwd)
        else:
            self.connection=MongoClient(host,port,username=username,password=pwd,authSource=dbname)
            self.db=self.connection[dbname]


    def reset(self):
        """!
        Reset connection to download another configuration
        """
        self.bson_id=[] 
    def uploadConfig(self,name,fname,comment,version=1):
        """!
        jobcontrol configuration upload

        @param name: Name of the configuration
        @param fname: File name to upload
        @param comment: A comment on the configuration
        @param version: The version of the configuration
        """
        s={}
        s["content"]=json.loads(open(fname).read())
        s["name"]=name
        s["time"]=time.time()
        s["comment"]=comment
        s["version"]=version
        resconf=self.db.configurations.insert_one(s)
        print(resconf)

    def uploadCalibration(self,setup,run,ctype,results,comment="not set"):
        """!
        Calibration results upload

        @param setup: Name of the experimental setup ($DAQSETUP)
        @param run: Run analyzed
        @param ctype: SCURVE or GAIN ...
        @param results: dictionnary
        @param comment: un commentaire
        """
        s={}
        name="%s_%d" % (setup,run)
        res=self.db.calibrations.find({'name':name})
        last=0
        for x in res:
            if (last<x["version"]):
                last=x["version"]
        if (last==0):
            s["version"]=1
        else:
            s["version"]=last+1


        s["comment"]=comment
        s["content"]=results
        s["type"]=ctype
        s["name"]=name
        s["time"]=time.time()
        s["setup"]=setup
        s["run"]=run
        resconf=self.db.calibrations.insert_one(s)
        print(resconf)

    def calibrations(self):
        """!
        List all the calibrations stored
        """
        cl=[]
        res=self.db.calibrations.find({})
        for x in res:
            if ("comment" in x):
                print(time.ctime(x["time"]),x["name"],x["version"],x["type"],x["setup"],x["run"],x["comment"])
                cl.append((x["name"],x["version"],x['run'],x['comment']))
        return cl


    def configurations(self):
        """!
        List all the configurations stored
        """
        cl=[]
        res=self.db.configurations.find({})
        for x in res:
            #print(x)
            if ("comment" in x):
                if ("pns" in x):
                    print(time.ctime(x["time"]),x["version"],x["name"],x["comment"],x["pns"])
                else:
                    print(time.ctime(x["time"]),x["version"],x["name"],x["comment"])

                cl.append((x["name"],x['version'],x['comment']))
        return cl
    def updateRun(self,run,loc,tag,vtag):
        """!
        Update the run entries with an additional or modified tag

        @param run the run number
        @param loc the experiment name
        @param tag Tag name 
        @param vtag Tag value 
        """
        filter = { 'run': run,'location':loc }
 
        # Values to be updated.
        newvalues = { "$set": { tag: vtag } }
 
        # Using update_one() method for single
        # updation.
        print(filter,newvalues)
        self.db.runs.update_one(filter, newvalues)

    def updateConfigurationInfo(self,cname,version,tag,vtag):
        """!
        Update the configuration entries with an additional or modified tag

        @param cname the configuration name
        @param version the version number
        @param tag Tag name 
        @param vtag Tag value 
        """
        filter = {'name':cname,'version':version}
 
        # Values to be updated.
        newvalues = { "$set": { tag: vtag } }
 
        # Using update_one() method for single
        # updation.
        print(filter,newvalues)
        self.db.configurations.update_one(filter, newvalues)
        
    def runs(self):
        """!
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
    def getRunInfo(self,run):
        """!
        Get the info on a given run number

        @param run the run number
        @return a list of db object with this run number
        """
        rlist=[]
        res=self.db.runs.find({"run":run})
        for x in res:
            #    print(x["time"],x["location"],x["run"],x["comment"])
            rlist.append(x)
        return rlist

    def runInfo(self,run,loc,printout=True):
        """!
        Get the info on a given run

        @param run the run number
        @param loc the experiment name
        @param printout a bool to trigger the printing
        @return the db object stored
        """
        res=self.db.runs.find({"run":run,"location":loc})
        for x in res:
            if (printout):
                for y in x.keys():
                    print(y,":",x[y])
            #if ("comment" in x):
            #    print(x["time"],x["location"],x["run"],x["comment"])
            return x
        return None

    def setFsmInfo(self,name,version,location,job=None,daq=None):
        """!
        fsm info insertion

        @param name: Configuration name
        @param version: Configuration version
        @param location: Setup name
        @param job: job state name or none
        @param daq: daq state name or none
        @deprecated
        """
        s=self.fsmInfo(name,version,location)
        if (s == None):
            s={}
            s["name"]=name
            s["location"]=location
            s["version"]=version
            s["job"]="NOTSET"
            s["daq"]="NOTSET"
        else:
            del s["_id"]
        s["time"]=time.time()
        if (job!=None):
            s["job"]=job
        if (daq!=None):
            s["daq"]=daq
        resconf=self.db.fsm.insert_one(s)
        print(resconf)

    def fsmInfo(self,name,version,location):
        """!
        Get FSM's information for a given configuration and setup
        @param name: Configuration name
        @param version: Configuration version
        @param location: Setup name
        @return the db object
        @deprecated
        """
        res=self.db.fsm.find({"name":name,"version":version,"location":location})
        last={}
        last["time"]=0
        for x in res:
            #print(x["name"],x["version"],x["location"],x["time"],x["job"],x["daq"])
            if (x["time"]>last["time"]):
                last=x
        if (last["time"]!=0):
            return last
        else:
            return None

    def fsms(self):
        """!
        Get FSM's informations dump
        """
        res=self.db.fsm.find({})
        for x in res:
            print(x["name"],x["version"],x["location"],time.ctime(x["time"]),x["job"],x["daq"])

    def downloadConfig(self,cname,version,toFileOnly=False):
        """!
        Download a jobcontrol configuration to /dev/shm/mgjob/ directory
        
        @param cname: Configuration name
        @param version: Configuration version
        @param toFileOnly:if True and /dev/shm/mgjob/cname_version.json exists, then it exits
        """
        os.system("mkdir -p /dev/shm/mgjob")
        fname="/dev/shm/mgjob/%s_%s.json" % (cname,version)
        if os.path.isfile(fname) and toFileOnly:
            #print('%s already download, Exiting' % fname)
            return
        res=self.db.configurations.find({'name':cname,'version':version})
        for x in res:
            print(x)
            print(x["name"],x["version"],x["comment"])
            #var=raw_input()
            slc=x["content"]
            if ("pns" in x):
                slc["pns"]=x["pns"]
            os.system("mkdir -p /dev/shm/mgjob")
            fname="/dev/shm/mgjob/%s_%s.json" % (cname,version)
            f=open(fname,"w+")
            f.write(json.dumps(slc, indent=2, sort_keys=True))
            f.close()
            return slc
    def downloadCalibration(self,cname,version,toFileOnly=False):
        """!
        Download a calibration to /dev/shm/mgjob/ directory
        
        @param cname: calibration name
        @param version: calibration version
        @param toFileOnly:if True and /dev/shm/mgjob/cname_version.json exists, then it exits
        """
        os.system("mkdir -p /dev/shm/mgjob")
        fname="/dev/shm/mgjob/calib_%s_%d.json" % (cname,version)
        if os.path.isfile(fname) and toFileOnly:
            print('%s already download, Exiting' % fname)
            return
        res=self.db.calibrations.find({'name':cname,'version':version})
        for x in res:
            print(x["name"],x["version"],x["type"],x["setup"],x["run"],x["comment"])
            #var=raw_input()
            slc=x["content"]
            os.system("mkdir -p /dev/shm/mgjob")
            fname="/dev/shm/mgjob/calib_%s_%d.json" % (cname,version)
            f=open(fname,"w+")
            f.write(json.dumps(slc))
            f.close()
            return slc
       
    def getRun(self,location,comment="Not set"):
        """!
        Get a new run number for a given setup

        @param location: Setup Name
        @param comment: Comment on the run
        @return: a dictionnary corresponding to the base insertion {run,location,time,comment}
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
 
     
def instance():
    """!
    Create a MongoJob Object
    
    The ENV varaible MGDBLOGIN=user/pwd@host:port@dbname mut be set

    @return: The MongoJob Object
    """
    # create the default access
    login=os.getenv("MGDBLOGIN","NONE")
    if (login == "NONE"):
        print("The ENV varaible MGDBLOGIN=user/pwd@host:port@dbname mut be set")
        exit(0)
    userinfo=login.split("@")[0]
    hostinfo=login.split("@")[1]
    dbname=login.split("@")[2]
    user=userinfo.split("/")[0]
    pwd=userinfo.split("/")[1]
    host=hostinfo.split(":")[0]
    port=int(hostinfo.split(":")[1])
    #print(host,port,dbname,user,pwd)
    _wdd=MongoJob(host,port,dbname,user,pwd)
    #print("apres")
    return _wdd
