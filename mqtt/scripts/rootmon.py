import time
import json
import paho.mqtt.client as paho
import os
import MongoSlow as ms
import logging
import ROOT
class rootmon:
    def __init__(self,host,port,session):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.port=port
        self.host=host
        self.session=session
        self.client=None
        self.l_json=None
        self.topicinfos=[]
        self.subtop=[]
        self.hws=[]
        self.rcv_msg={}
        self.msi=None
        self.hmap={}
        logging.basicConfig(level=logging.INFO)

   
        
    def on_topics(self,client, userdata, message):
        #time.sleep(1)

        print("received topic =",str(message.topic))
        #print("received message =",str(message.payload.decode("utf-8")))
        self.l_json=json.loads(str(message.payload.decode("utf-8")))
    def on_message(self,client, userdata, message):
        #time.sleep(1)
        #print(message.timestamp)

        print("received topic =",str(message.topic))
        #print("received message =",str(message.payload.decode("utf-8")))
        if str(message.topic) in self.hmap:
            del  self.hmap[str(message.topic)]
        self.hmap[str(message.topic)]=ROOT.TBufferJSON.ConvertFromJSON(str(message.payload.decode("utf-8")))
       
    def Connect(self):
        self.cname="monitor-%d" % os.getpid()
        self.client= paho.Client(self.cname) 
        ######Bind function to callback
       

        print("connecting to broker ",self.host,":",self.port)
        self.client.connect(self.host,self.port)#connect
        print("connected");
    def sendCommand(self,command,params={}):
        topic=self.session+"/CMD"
        msg={}
        msg["name"]=command
        msg["params"]=params
        self.client.publish(topic, payload=json.dumps(msg), qos=0, retain=False)
    def ListTopics(self):
        self.client.on_message=self.on_topics
        self.client.loop_start() #start loop to process received messages
        print("subscribing all ")
        self.client.subscribe(self.session+"/HISTOLIST")
        self.sendCommand("HISTOLIST")
        #subscribe
        time.sleep(6)
        self.client.unsubscribe("#")
        print(self.l_json)
        self.client.loop_stop()
        self.client.on_message=self.on_message
        self.client.loop_start()
        for x in self.l_json["TH1"]:
            print("subscribing %s \n" % (self.session+"/sdhcalmon"+x))
            self.client.subscribe(self.session+"/sdhcalmon"+x)
        for x in self.l_json["TH2"]:
            print("subscribing %s \n" % (self.session+"/sdhcalmon"+x))
            self.client.subscribe(self.session+"/sdhcalmon"+x)
        
    def loop(self):
        self.client.on_message=self.on_message
        self.client.loop_start() #start loop to process received messages

        for x in self.subtop:
            self.client.subscribe(x)#subscribe
            print("subscribing ",x)


            

#s=monitor("lyocms09.in2p3.fr",1883,"dome_sdhcal")
#s.Connect()
#time.sleep(4)
#s.ListTopics()
#s.loop()
#while (True):
#    time.sleep(1)
