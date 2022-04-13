#!/usr/bin/env python3
import os
import pymongo
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

        if (pymongo.version_tuple[0]<4):
            self.connection=MongoClient(host,port)
            self.db=self.connection[dbname]
            self.db.authenticate(username,pwd)
        else:
            self.connection=MongoClient(host,port,username=username,password=pwd,authSource=dbname)
            self.db=self.connection[dbname]




        
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

    def updateStateInfo(self,statename,vers,tag,vtag):
        filter = { 'name': statename,'version':vers}
 
        # Values to be updated.
        newvalues = { "$set": { tag: vtag } }
 
        # Using update_one() method for single
        # updation.
        print(filter,newvalues)
        self.db.states.update_one(filter, newvalues)
    
    
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
        
    def changeParam(self,pname,pval,idif=0, iasic=0):
        """
        Change all the ENable signals of PETIROC asic

        :param idif: DIF_ID (IP>>16), if 0 all FEBs are changed
        :param iasic: asic number, if 0 all Asics are changed
        :param pname: parameter name
        :param pval: paramter value
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"][pname]=pval
                a["_id"]=None

            except Exception as e:
                print(e.getMessage())
    
    
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
