#!/usr/bin/env python3
import os
import pymongo
from pymongo import MongoClient
import json
import bson
from bson.objectid import ObjectId
import time


class MongoMqtt:
    """
    Main class to access the Mongo DB 
    """

    def __init__(self, host, port, dbname, username, pwd):
        """
        connect Mongodb database 

        :param host: Hostanme of the PC running the mongo DB

        :param port: Port to access the base

        :param dbname: Data base name

        :param username: Remote access user

        :param pwd: Remote access password

        """

        if (pymongo.version_tuple[0] < 4):
            self.connection = MongoClient(host, port)
            self.db = self.connection[dbname]
            self.db.authenticate(username, pwd)
        else:
            self.connection = MongoClient(
                host, port, username=username, password=pwd, authSource=dbname)
            self.db = self.connection[dbname]

    def reset(self):
        """
        Reset connection to download another configuration
        """
        self.bson_id = []

    def store(self, topic, tims, cti, message):
        mondoc = {}
        mondoc["topic"] = topic
        mondoc["timestamp"] = tims
        mondoc["ctime"] = cti
        mondoc["message"] = message
        #print("storing",mondoc)
        res = self.db.MQTT_ITEMS.insert_one(mondoc)
        #print(res)

    def items(self, topic, depth=50000, from_time=0):
        """
        List all the run informations stored
        """
        path_elem=topic.split("/")
        if (len(path_elem)<3):
            print("Too short topic ",topic)
        device = path_elem[2].split("_")[0]
        print(path_elem,device)
        mintime = 0
        if (from_time > 0):
            mintime = time.time()-from_time

        res = self.db.MQTT_ITEMS.find({"topic": {'$regex': topic}, "ctime": {'$gt': mintime}}, {
                                      "_id": 0}).limit(depth).sort("ctime", pymongo.DESCENDING)
        for x in res:
            #print(x)
            sti = time.strftime('%Y-%m-%d %H:%M:%S',
                                time.localtime(x["ctime"]))
            m = x["message"]
            #print(sti, m)
            if (device == "bme"):
                print("%s P=%.2f mbar T=%.2f K %.2f C Humidity %.2f %% " %
                      (sti, m["P"], m["T"]+273.15, m["T"], m["H"]))
            if (device == "hih"):
                print("%s H0=%.2f %% T0=%.2f C " % (sti, m["H"], m["T"]))
            if (device == "genesys"):
                print("%s VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " %
                      (sti, m["vset"], m["vout"], m["iout"], m["status"]))
            if (device == "zup"):
                print(" VSET=%.2f V VOut=%.2f V IOut=%.2f A Imax=%.2f A  Status %s   " % (
                    m["vset"], m["vout"], m["iout"], m["iset"], m["status"]))

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


def instance():
    """
    Create a MongoJob Object

    The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set

    :return: The MongoJob Object
    """
    # create the default access
    login = os.getenv("MGDBMON", "NONE")
    if (login == "NONE"):
        print("The ENV varaible MGDBMON=user/pwd@host:port@dbname mut be set")
        exit(0)
    userinfo = login.split("@")[0]
    hostinfo = login.split("@")[1]
    dbname = login.split("@")[2]
    user = userinfo.split("/")[0]
    pwd = userinfo.split("/")[1]
    host = hostinfo.split(":")[0]
    port = int(hostinfo.split(":")[1])
    # print(host,port,dbname,user,pwd)
    _wdd = MongoMqtt(host, port, dbname, user, pwd)
    # print("apres")
    return _wdd
