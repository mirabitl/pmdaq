#!/usr/bin/python3
import time
import json
import paho.mqtt.client as paho
import os
import MongoMqtt as ms
import logging
import graphyte
import random

class pico_monitor:
    def __init__(self,host,port,config_name="/etc/pico_mon.json"):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.fout=open("/var/log/pico.log","a+")
        self.port=port
        self.host=host
        self.session=None
        self.client=None
        self.flag_connected=0

        self.settings=json.load(open(config_name))
        self.topics=[]
        self.topicm={}
        #self.topicinfos=[]
        #self.subtop=[]
        #self.hws=[]
        self.rcv_msg={}
        self.msi=None
        login = os.getenv("MGDBMON", "NONE")
        if ("mongo" in self.settings):
            os.environ["MGDBMON"]=self.settings["mongo"]
            self.connectMongo()
            self.check_topics()
        ghost = os.getenv("GRAPHITEHOST", "NONE")
        self.gsender=None
        if ("graphyte" in self.settings):
            self.gsender=graphyte.Sender(self.settings["graphyte"])
        logging.basicConfig(level=logging.INFO)

    def connectMongo(self):
        
        self.msi=ms.instance()

    def check_topics(self):
        for sess in self.settings["sessions"]:
            r=self.msi.check_infos(sess,i_type="INFO")
            for x in r:
                lt=x["topic"].split("/")
                topic=lt[0]+"/"+lt[1]+"/"+lt[2]
                if (not topic in self.topics):
                    self.topics.append(topic)
                    self.topicm[topic]=x["message"]

    def on_topics(self,client, userdata, message):
        self.fout.flush()
        #time.sleep(1)

        #print("received topic =",str(message.topic),file=self.fout)
        #print("received message =",str(message.payload.decode("utf-8")),file=self.fout)
        lt=str(message.topic).split("/")
        # At least session/system/device_name/INFOS
        if (len(lt)<4):
            return
        if (not lt[0] in self.settings["sessions"]):
            return
        #l_device=lt[2].split("_")
        if (lt[3]!="INFOS" and lt[3]!="GAS"):
            return
        # Add topics to listen
        # for INFOS registered devices
        if (lt[3]=="INFOS"):
            topic=lt[0]+"/"+lt[1]+"/"+lt[2]
            if (not topic in self.topics):
                self.topics.append(topic)
                self.topicm[str(message.topic)]=str(message.payload.decode("utf-8"))
        # Store in MQTT_INFOS, registered devices and gas informations
        r_m={}
        r_m["ctime"]=time.time()
        r_m["timestamp"]=message.timestamp
        r_m["content"]=json.loads(str(message.payload.decode("utf-8")));
        r_m["type"]=lt[3]
        # Store in Mongo DB if connected
        if (self.msi!=None):
            #self.msi.store(p[0],p[1], r_m["ctime"], r_m["content"])
            
            self.msi.store_info(message.topic,r_m)
            #r_m["timestamp"],r_m["ctime"],r_m["content"])

            
    def on_connect(self,client, userdata, flags, rc):
        self.flag_connected = 1

    def on_disconnect(self,client, userdata, rc):
        self.flag_connected = 0

    def on_message(self,client, userdata, message):
        self.fout.flush()
        tfirst=time.time()
        #time.sleep(1)
        #print(message.timestamp,file=self.fout)

        print("received topic =",str(message.topic),file=self.fout)
        #print("received message =",str(message.payload.decode("utf-8")),file=self.fout)

        p=message.topic.split("/")
        if (not p[0] in self.settings["sessions"]):
            return
        if (p[len(p)-1]=="INFOS"):
            return
        if (len(p)>3 and p[3]=="GAS"):
            return
        if ( not message.topic in self.rcv_msg):
            self.rcv_msg[message.topic]=[]
        r_m={}
        r_m["ctime"]=time.time()
        r_m["timestamp"]=message.timestamp
        r_m["content"]=json.loads(str(message.payload.decode("utf-8")));
        self.rcv_msg[message.topic].append(r_m)
        if (len( self.rcv_msg[message.topic])>1000):
            self.rcv_msg[message.topic].pop(0)

        # Store in Mongo DB if connected
        if (self.msi!=None):
            #self.msi.store(p[0],p[1], r_m["ctime"], r_m["content"])
            sm=p[0]+p[1]+json.dumps(r_m)
            self.msi.store(message.topic,r_m["timestamp"],r_m["ctime"],r_m["content"])
            logging.debug(sm)
        else:
            if (False):
                print("No db storage",file=self.fout)
            #print(r_m,file=self.fout)
        ## BMP data
        if (self.gsender!=None):
            
            for x in r_m["content"].keys():
                #print(len(p),x,file=self.fout)
                if (len(p)>=4 and p[3]=="INFOS"):
                    continue
                if (len(p)>=4 and p[3]=="GAS"):
                    continue
                metric=""
                for i in range(len(p)):
                    metric=metric+p[i]+"."
                metric=metric+x
                #print(metric,x,file=self.fout)
                try:
                    self.gsender.send(metric,r_m["content"][x])
                except Exception as error:
                    print("Error sending ",x,r_m["content"],file=self.fout)
                    break
        #print("%s Processing time : %.3f" % (message.topic,(time.time()-tfirst)),file=self.fout)
    def Connect(self):
        if (self.client!=None):
            del self.client
        id=random.randrange(1, 1000)
        self.cname="monitor-%d" % id
        self.client= paho.Client(self.cname)
        self.client.on_connect=self.on_connect
        self.client.on_disconnect=self.on_disconnect
        ######Bind function to callback
       

        print("connecting to broker ",self.host,":",self.port,file=self.fout)
        self.client.connect(self.host,self.port,keepalive=600)#connect
        print("connected",file=self.fout);
    def ListTopics(self):
        self.client.on_message=self.on_topics
        self.client.loop_start() #start loop to process received messages
        print("subscribing all ",file=self.fout)
        self.client.subscribe("#")#subscribe
        time.sleep(3)
        self.client.unsubscribe("#")
        #print(self.topics,file=self.fout)
        self.client.loop_stop()
        #idt=0
        for s in self.topics:
            print("Registered topic %s \n",s,file=self.fout)
    def loop(self):
        self.client.on_message=self.on_message
        self.client.loop_start() #start loop to process received messages

        for x in self.topics:
            t=x+"/#"
            self.client.subscribe(t)#subscribe
            print("subscribing ",t,file=self.fout)
        
    def stop(self):
        self.client.loop_stop() #start loop to process received messages

        for x in self.topics:
            t=x+"/#"
            self.client.unsubscribe(t)#subscribe
            print("subscribing ",t,file=self.fout)
        


            

#s=monitor("lyocms09.in2p3.fr",1883,"dome_sdhcal")
#s.Connect()
#time.sleep(4)
#s.ListTopics()
#s.loop()
#while (True):
#    time.sleep(1)
if __name__ == "__main__":
    s=pico_monitor("lyoilc07",1883,"/etc/pico_mon.json")
    s.Connect()
    s.ListTopics()
    s.loop()
    while 1:
        if (s.flag_connected==0):
            s.stop()
            s.Connect()
            s.loop()
        time.sleep(1)
