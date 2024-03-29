#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
import bson
from bson.objectid import ObjectId
import time
import csv




def IP2Int(ip):
    """
    convert IP adress string to int

    :param IP: the IP address
    :return: the encoded integer 
    """
    o = list(map(int, ip.split('.')))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res


class MongoSlow:
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


    def reset(self):
        """
        Reset connection to download another configuration
        """
        self.bson_id=[]
        
    def store(self,location,hardware,cti,status):
        mondoc={}
        mondoc["path"]=","+location+","+hardware+","
        mondoc["ctime"]=cti
        mondoc["status"]=status
        #print("storing",mondoc)
        res=self.db.MQTT_ITEMS.insert_one(mondoc)
        #print(res)
    def store_mqtt(self,topic,tims,cti,message):
        mondoc={}
        mondoc["topic"]=topic
        mondoc["timestamp"]=tims
        mondoc["ctime"]=cti
        mondoc["message"]=message
        #print("storing",mondoc)
        res=self.db.MQTT_ITEMS.insert_one(mondoc)
        #print(res)
    def dump_path(self,depth=50000,from_time=0):
        mintime=0
        if (from_time>0):
            mintime=time.time()-from_time
        res=self.db.MQTT_ITEMS.find({"ctime":{'$gt':mintime}},{"_id":0}).limit(depth).sort("ctime",pymongo.DESCENDING)
        for x in res:
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            print(sti,x["topic"])
    def items(self,path,depth=50000,from_time=0):
        """
        List all the run informations stored
        """
        device=path.split("/")[2]
        mintime=0
        if (from_time>0):
            mintime=time.time()-from_time
            
        res=self.db.MQTT_ITEMS.find({"topic":{'$regex':path},"ctime":{'$gt':mintime}},{"_id":0}).limit(depth).sort("ctime",pymongo.DESCENDING)
        for x in res:
            #print(x)
           
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            if (device == "cpwplus"):
                print(f"{sti} {x['message']['net']} kg")
            if (device=="bme"):
                print("%s P=%.2f mbar T=%.2f K %.2f %% " % (sti,x["message"]["P"]/100.,x["message"]["T"]+273.15,x["message"]["H"]))
            if (device=="hih" ):
                print("%s H=%.2f %% T=%.2f C " % (sti,x["message"]["H"],x["message"]["T"]))
            if (device=="wiener") :
                print(sti)
                for y in x["message"]["channels"]:
                    if (not isinstance(y["status"],str)):
                        print(y)
                        continue
                    sstat=y["status"].split("=")[1]
                    
                    #print("ch%.3d %12.2f %12.2f %12.2f %s" %(y["id"],y["vset"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
                    print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
            if (device=="genesys"):
                print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Message %s " % (sti,x["message"]["vset"],x["message"]["vout"],x["message"]["iout"],x["message"]["status"]))
            if (device=="SYX27" and x["status"]["name"]==device):
                print(sti)
                for y in x["status"]["channels"]:
                    print("%12s %8.2f %8.2f %8.2f %8.2f %8.2f %d" %(y["name"],y["vset"],y["iset"],y["rampup"],y["vout"],y["iout"],y["status"]))                
            if ((device=="ZUP" and x["status"]["name"]==device)):
                #print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (sti,x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["status"]))
                print(" VSET=%.2f V VOut=%.2f V IOut=%.2f A Imax=%.2f A  Status %s  Bit Status %s " % (x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["iset"],x["status"]["status"],bin(x["status"]["pwrstatus"])))
            if (device =="ZupPaho"):
                #print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (sti,x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["status"]))
                print(" VSET=%.2f V VOut=%.2f V IOut=%.2f A Imax=%.2f A  Status %s   " % (x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["iset"],x["status"]["status"]))

                
    def csv(self,path,depth=50000,from_time=0,file_name="/tmp/mgslow.csv"):
        """
        List all the run informations stored
        """
        fout=open(file_name, 'w')
        writer=None
        device=path.split(",")[2]
        if (device=="BMP" or device=="BmpPaho" ):
            fieldnames = ['Date','Pressure', 'Temperature']
            writer = csv.writer(fout, delimiter="|")
            writer.writerow(fieldnames)
        if (device=="GENESYS" or device =="ZUP" or device=="ZupPaho"  or device=="GenesysPaho"  ):
            fieldnames = ['Date','VOUT', 'IOUT']
            writer = csv.writer(fout, delimiter="|")
            writer.writerow(fieldnames)
        mintime=0
        if (from_time>0):
            mintime=time.time()-from_time
            
        res=self.db.MQTT_ITEMS.find({"path":{'$regex':path},"ctime":{'$gt':mintime}},{"_id":0}).limit(depth).sort("ctime",pymongo.DESCENDING)
        for x in res:
            #print(x)
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            if ( (device=="BMP" and x["status"]["name"]==device) or device=="BmpPaho" ):
                print("%s P=%.2f mbar T=%.2f K " % (sti,x["status"]["pressure"],x["status"]["temperature"]+273.15))
                writer.writerow([sti,x["status"]["pressure"],x["status"]["temperature"]+273.15])
            if (device=="HIH" and x["status"]["name"]==device):
                print("%s H0=%.2f %% T0=%.2f K H1=%.2f %% T1=%.2f K " % (sti,x["status"]["humidity0"],x["status"]["temperature0"],x["status"]["humidity1"],x["status"]["temperature1"]))
            if ( (device=="ISEG" and x["status"]["name"]==device) or device=="WienerPaho" ):
                print(sti)
                if (writer==None):
                    fieldnames=["date"]
                    for y in x["status"]["channels"]:
                        fieldnames.append("ch%.2d_vout" % y["id"])
                        fieldnames.append("ch%.2d_iout" % y["id"])
                    writer = csv.writer(fout, delimiter="|")
                    writer.writerow(fieldnames)
                measure=[]
                for y in x["status"]["channels"]:
                    sstat=y["status"].split("=")[1]
                    if (len(measure)==0):
                        measure.append(sti)
                    measure.append(y["vout"])
                    measure.append(y["iout"])
                writer.writerow(measure)
                    #print("ch%.3d %12.2f %12.2f %12.2f %s" %(y["id"],y["vset"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
                    #print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
            if ((device=="GENESYS" and x["status"]["name"]==device) or device=="GenesysPaho") :
                print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (sti,x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["status"]))
                writer.writerow([sti,x["status"]["vout"],x["status"]["iout"]])

            if ((device=="SYX27" and x["status"]["name"]==device) or device=="Sy1527Paho") :
                print(sti)
                for y in x["status"]["channels"]:
                    print("%12s %8.2f %8.2f %8.2f %8.2f %8.2f %d" %(y["name"],y["vset"],y["iset"],y["rampup"],y["vout"],y["iout"],y["status"]))
                if (writer==None):
                    fieldnames=["date"]
                    for y in x["status"]["channels"]:
                        fieldnames.append("%s_vout" % y["name"])
                        fieldnames.append("%s_iout" % y["name"])
                        fieldnames.append("%s_vset" % y["name"])
                        fieldnames.append("%s_iset" % y["name"])
                    writer = csv.writer(fout, delimiter="|")
                    writer.writerow(fieldnames)
                measure=[]
                for y in x["status"]["channels"]:
                    if (len(measure)==0):
                        measure.append(sti)
                    measure.append(y["vout"])
                    measure.append(y["iout"])
                    measure.append(y["vset"])
                    measure.append(y["iset"])
                writer.writerow(measure)

            if ((device=="ZUP" and x["status"]["name"]==device) or device=="ZupPaho" ):
                #print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (sti,x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["status"]))
                print(" VSET=%.2f V VOut=%.2f V IOut=%.2f A Imax=%.2f A  Status %s  Bit Status %s " % (x["status"]["vset"],x["status"]["vout"],x["status"]["iout"],x["status"]["iset"],x["status"]["status"],bin(x["status"]["pwrstatus"])))
                writer.writerow([sti,x["status"]["vout"],x["status"]["iout"]])
    def last(self,path):
        """
        List all the run informations stored
        """
        res=self.db.MQTT_ITEMS.find({"path":{'$regex':path}},{"_id":0}).limit(1).sort("ctime",pymongo.DESCENDING)
        for x in res:
            return x


                
def instance():
    """
    Create a MongoJob Object
    
    The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set

    :return: The MongoJob Object
    """
    # create the default access
    login=os.getenv("MGDBMON","NONE")
    if (login == "NONE"):
        print("The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set")
        exit(0)
    userinfo=login.split("@")[0]
    hostinfo=login.split("@")[1]
    dbname=login.split("@")[2]
    user=userinfo.split("/")[0]
    pwd=userinfo.split("/")[1]
    host=hostinfo.split(":")[0]
    port=int(hostinfo.split(":")[1])
    #print(host,port,dbname,user,pwd)
    _wdd=MongoSlow(host,port,dbname,user,pwd)
    #print("apres")
    return _wdd
