#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
import bson
from bson.objectid import ObjectId
import time
import csv

class MongoMqtt:
    """!
    Main class to access the Mongo DB 
    """

    def __init__(self, host, port, dbname, username, pwd,auth=None):
        """!
        connect Mongodb database 

        @param host: Hostanme of the PC running the mongo DB

        @param port: Port to access the base

        @param dbname: Data base name

        @param username: Remote access user

        @param pwd: Remote access password

        """

        if (pymongo.version_tuple[0] < 3):
            self.connection = MongoClient(host, port)
            self.db = self.connection[dbname]
            self.db.authenticate(username, pwd)
        else:
            if (auth==None):
                self.connection = MongoClient(
                    host, port, username=username, password=pwd, authSource=dbname)
            else:
                self.connection = MongoClient(
                    host, port, username=username, password=pwd, authSource=auth)
                
            self.db = self.connection[dbname]

    def reset(self):
        """!
        Reset connection to download another configuration
        """
        self.bson_id = []

    def store(self, topic, tims, cti, message):
        """!
        Store a status message in MQTT_ITEMS

        @param topic the message topic
        @param tims Time Stamp
        @param cti The ctime since epoch
        @param message The JSON message
        """
        mondoc = {}
        mondoc["topic"] = topic
        mondoc["timestamp"] = tims
        mondoc["ctime"] = cti
        mondoc["message"] = message
        #print("storing",mondoc)
        res = self.db.MQTT_ITEMS.insert_one(mondoc)
        #print(res)

    def store_info(self, topic, r):
        """!
        Store an info message in MQTT_INFOS

        @param topic the message topic
        @param r The python object containing tags: timestamp,ctime,message and type
        """
        mondoc = {}
        mondoc["topic"] = topic
        mondoc["timestamp"] = r["timestamp"]
        mondoc["ctime"] = r["ctime"]
        mondoc["message"] =r["content"]
        mondoc["type"]=r["type"]
        #print("storing",mondoc)
        res = self.db.MQTT_INFOS.insert_one(mondoc)
        #print(res)

    def check_infos(self,session,i_type="INFOS",subsystem=None,device=None,from_time=0):
        """!
        Query entries in MQTT_INFO collections

        @param session the top MQTT name (Leval 0)
        @param subsystem Level 1 name
        @param device Level 2 name
        @param from_time depth since now in seconds

        @return array of results
        """
        topic=session
        if (subsystem!=None):
            topic=topic+"/"+subsystem
        if (device!=None):
            topic=topic+"/"+device
        topic=topic
        #print(topic)
        depth=1000000
        
        mintime = 0
        if (from_time > 0):
            mintime = time.time()-from_time
        res = self.db.MQTT_INFOS.find(
          {"$and": [
              {"topic": {'$regex': topic}},
              {"ctime": {'$gt': mintime}},
              {"type": {'$regex':i_type}}
          ]
           },
            {"_id": 0}).limit(depth).sort("ctime", pymongo.DESCENDING)
        #for x in res:
        #    print(x)
        return res

        #return None
    def infos(self, topic, depth=50000, from_time=0):
        """!
        List and printout all the MQTT_INFOS informations stored
        
        @param depth Maximum number of entries scanned
        @param from_time depth since now in seconds

        """
       
        mintime = 0
        if (from_time > 0):
            mintime = time.time()-from_time

        res = self.db.MQTT_INFOS.find({"topic": {'$regex': topic}, "ctime": {'$gt': mintime}}, {
                                      "_id": 0}).limit(depth).sort("ctime", pymongo.DESCENDING)
        for x in res:
            print(x)
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(x["ctime"]))
            #m = x["message"]
    def items(self, topic, depth=50000, from_time=0):
        """!   
        List and printout all the MQTT_ITEMS status stored

        @param topic the message topic
        @param depth Maximum number of entries scanned
        @param from_time depth since now in seconds

        """
        
        print("Dans Items",topic)
        path_elem=topic.split("/")
        if (len(path_elem)<3):
            print("Too short topic ",topic)
        device = path_elem[2].split("_")[0]
        print(path_elem,device)
        mintime = 0
        if (from_time > 0):
            mintime = time.time()-from_time

        #res = self.db.MQTT_ITEMS.find({"topic": {'$regex': topic}, "ctime": {'$gt': mintime}}, {"_id": 0}).limit(depth).sort("ctime", pymongo.DESCENDING)
        res = self.db.MQTT_ITEMS.find({"topic": {'$regex': topic}, "ctime": {'$gt': mintime}}, {"_id": 0}).limit(depth)
        for x in res:
            #print(device,x,x["message"].keys())
            #str=f' '
            #for k in x["message"].keys():
            #     str=str+f'{x["message"][k]},'
            #str=str+f'{x["ctime"]}'
            #print(str)
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(x["ctime"]))
            m = x["message"]
            #print(m)
            ltop=x["topic"].split("/")
            if (len(ltop)==4 and ltop[3]=="INFOS"):
                continue
            #print(sti, m)
            if (device=="wiener") :
                print(sti)
                for y in x["message"]["channels"]:
                    if (not isinstance(y["status"],str)):
                        print(y)
                        continue
                    sstat=y["status"].split("=")[1]

                    #print("ch%.3d %12.2f %12.2f %12.2f %s" %(y["id"],y["vset"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
                    print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
            if (device=="sy1527"):
                print(sti)
                #print(m)
                for y in m["channels"]:
                    print("%12s %8.2f %8.2f %8.2f %8.2f %8.2f %d" %(y["chname"],y["vset"],y["iset"],y["rampup"],y["vout"],y["iout"],y["status"]))                
            if (device == "rp2040"):
                if  ( "T" in m.keys() ): 
                    print("%s Pico Board T=%.2f C " % (sti,  m["T"]))
                else:
                    print(sti,x["topic"],m)
            if (device == "cpwplus"):
                if  ( "net" in m.keys() ): 
                    print("%s  Masse=%.2f Kg " % (sti,  m["net"]))
                else:
                    print(sti,x["topic"],m)
            if (device == "brooks"):
                gas=ltop[3]
                if ("setpoint_selected" in m.keys() and "primary_variable" in m.keys()):
                    print(m)
                    print("%s %s Flow Set %.5f l/h Read %.5f l/h" % (sti,gas,m["setpoint_selected"],m["primary_variable"]))
                else:
                    print(sti,x["topic"],m)
            if (device == "bme"):
                if  ( "P" in m.keys() and "T" in m.keys() and "H" in m.keys()): 
                    print("%s P=%.2f mbar T=%.2f K %.2f C Humidity %.2f %% " %
                          (sti, m["P"], m["T"]+273.15, m["T"], m["H"]))
                else:
                    print(sti,x["topic"],m)
            if (device == "hih"):
                if  ( "T" in m.keys() and "H" in m.keys()): 
                    print("%s H0=%.2f %% T0=%.2f C " % (sti, m["H"], m["T"]))
                else:
                    print(sti,x["topic"],m)
            if (device == "genesys"):
                tags=["vset","vout","iout","iset","status"]
                ok=True
                for t in tags:
                    ok =ok and t in m.keys()
                if (ok):
                    print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %d " %
                          (sti, m["vset"], m["vout"], m["iout"], m["status"]))
                else:
                    print(sti,x["topic"],m)
            if (device == "zup"):
                tags=["vset","vout","iout","iset","status"]
                ok=True
                for t in tags:
                    ok =ok and t in m.keys()
                if (ok):
                    print(" VSET=%.2f V VOut=%.2f V IOut=%.2f A Imax=%.2f A  Status %d   " % (
                        m["vset"], m["vout"], m["iout"], m["iset"], m["status"]))
                else:
                    print(sti,x["topic"],m)

    def last(self, topics, lim=20):
        """
        List all the run informations stored
        """
        #query = { "topic": { "$regex": topics } }
        #docs = self.db.MQTT_ITEMS.count_documents( query )
        #print(docs)
        res = self.db.MQTT_ITEMS.find({"topic": {'$regex': topics}}, {"_id": 0}).limit(lim).sort("ctime", pymongo.DESCENDING)
        for x in res:
            print(x)

    def dumpcsv(self,topic,depth=5000,from_time=0):
        """!   
        List and write in a csv file all the MQTT_ITEMS status stored

        @param topic the message topic
        @param depth Maximum number of entries scanned
        @param from_time depth since now in seconds

        @deprecated Use cvs instead
        """
        writer=None
        path_elem=topic.split("/")
        last_ctime=0
        last_err=0
        if (len(path_elem)<3):
            print("Too short topic ",topic)
            return
        fname="_".join(path_elem)+".csv"
        fout=open(fname, 'w')
        device = path_elem[2].split("_")[0]
        print(path_elem,device)

        writer = csv.writer(fout, delimiter="|")
        mintime = 0
        if (from_time > 0):
            mintime = time.time()-from_time

        res = self.db.MQTT_ITEMS.find({"topic": {'$regex': topic}, "ctime": {'$gt': mintime}}, {
                                      "_id": 0}).limit(depth).sort("ctime", pymongo.DESCENDING)
        headers=['Date']
        itype='flat'
        for y in res:
            #print(y)
            x=y["message"]
                    #print(device,x,x["message"].keys())
            if (len(headers)==1):
                for k in x.keys():
                    if isinstance(x[k],list):
                        if isinstance(x[k][0],dict):
                            for l in x[k][0].keys():
                                headers.append(l)
                        itype='array'
                    else:
                        headers.append(k)
                headers.append('ctime')
                writer.writerow(headers)

            values=[]
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(y["ctime"]))
            if (itype=='flat'):
                values.append(sti)
            for k in x.keys():
                if isinstance(x[k],list):
                    for j in range(len(x[k])):
                        val=[]
                        val.append(sti)
                        for l in x[k][j].keys():
                            if (isinstance(x[k][j][l],float)):
                                val.append(round(x[k][j][l],3))
                            else:
                                val.append(x[k][j][l])
                        val.append(round(y['ctime'],2))
                        writer.writerow(val)
                else:
                    values.append(x[k])
            if (itype=='flat'):
                values.append(round(y['ctime'],1))
                writer.writerow(values)
        fout.close()

    def csv(self,topic,depth=50000,from_time=0):
        """!   
        List and write in a csv file all the MQTT_ITEMS status stored

        @param topic the message topic
        @param depth Maximum number of entries scanned
        @param from_time depth since now in seconds

        @warning Only bme,hih,rp2040 and genesys ae supported yet
        """
       
   
        writer=None
        path_elem=topic.split("/")
        last_ctime=0
        last_err=0
        if (len(path_elem)<3):
            print("Too short topic ",topic)
            return
        fname="_".join(path_elem)+".csv"
        fout=open(fname, 'w')
        device = path_elem[2].split("_")[0]
        print(path_elem,device)

        if (device=="bme" ):
            fieldnames = ['Date','P', 'T','H']
            writer = csv.writer(fout, delimiter="|")
            writer.writerow(fieldnames)
        if (device=="hih" ):
            fieldnames = ['Date','T','H']
            writer = csv.writer(fout, delimiter="|")
            writer.writerow(fieldnames)
        if (device=="rp2040" ):
            fieldnames = ['Date','rtc','T']
            writer = csv.writer(fout, delimiter="|")
            writer.writerow(fieldnames)
        if (device=="genesys" or device =="zup"):
            fieldnames = ['Date','vout', 'iout','vset','iset','status']
            writer = csv.writer(fout, delimiter="|")
            writer.writerow(fieldnames)

        mintime = 0
        if (from_time > 0):
            mintime = time.time()-from_time

        res = self.db.MQTT_ITEMS.find({"topic": {'$regex': topic}, "ctime": {'$gt': mintime}}, {
                                      "_id": 0}).limit(depth).sort("ctime", pymongo.ASCENDING)

        for y in res:

            x=y["message"]


            ltop=y["topic"].split("/")
            if (len(ltop)==4 and ltop[3]=="INFOS"):
                continue
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(y["ctime"]))

            if (y["ctime"]-last_ctime>120):
                print(int(y["ctime"]-last_err),int(y["ctime"]-last_ctime),sti,y)
                last_err=y["ctime"]
            last_ctime=y["ctime"]
            if ( device=="bme"):
                writer.writerow([sti,x["P"],x["T"],x["H"]])
            if ( device=="hih"):
                writer.writerow([sti,x["T"],x["H"]])
            if ( device=="rp2040"):
                if ("rtc" in x):
                    writer.writerow([sti,x["rtc"],x["T"]])
                else:
                    writer.writerow([sti,int(y["ctime"]),x["T"]])
            if (device =="genesys" or device=="zup") :
                writer.writerow([sti,x["vout"],x["iout"],x["vset"],x["iset"],x["status"]])
        fout.close()

def instance():
    """!
    Create a MongoJob Object

    The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set

    @return: The MongoJob Object
    """
    # create the default access
    login = os.getenv("MGDBMON", "NONE")
    if (login == "NONE"):
        print("The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set")
        exit(0)
    list_info=login.split("@")
    userinfo = list_info[0]
    hostinfo = list_info[1]
    dbname = list_info[2]
    auth=None
    if (len(list_info)==4):
        auth=list_info[3]
    user = userinfo.split("/")[0]
    pwd = userinfo.split("/")[1]
    host = hostinfo.split(":")[0]
    port = int(hostinfo.split(":")[1])
    # print(host,port,dbname,user,pwd)
    _wdd = MongoMqtt(host, port, dbname, user, pwd,auth)
    # print("apres")
    return _wdd
