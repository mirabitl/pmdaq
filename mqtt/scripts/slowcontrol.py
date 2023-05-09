#!/usr/bin/env python3
import time
import json
import paho.mqtt.client as paho
import os
import MongoSlow as ms
import logging
import graphyte
class slowcontrol:
    def __init__(self,fsettings):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        # Parse the configuration file
        logging.basicConfig(level=logging.INFO)
        self.settings=json.load(open(fsettings))
        if (not "broker" in self.settings.keys()):
            logging.fatal("Invalid settings file =",fsettings)
            exit(0)
        self.port=self.settings["broker"]["port"]
        self.host=self.settings["broker"]["host"]
        self.msi=None
        if ("mongo" in self.settings.keys()):
            os.environ["MGDBMON"]=self.settings["mongo"]
            self.connectMongo()
        self.gsender=graphyte.Sender('lyocms09.in2p3.fr')
        self.subtop={}

        for x in self.settings["broker"]["sessions"]:
            session=x["name"]
            for y in x["services"]:
                instance=0
                if ("instance" in y.keys()):
                    instance=y["instance"]
                hw=y["name"]
                proc=session+"/"+hw+"/%d" % instance
                self.subtop[proc]={}
                self.subtop[proc]["period"]=y["period"]
                self.subtop[proc]["valid"]=0
        self.client=None
        self.rcv_msg={}
        self.topics=[]
        self.topicinfos=[]



    def connectMongo(self):
        self.msi=ms.instance()
    def Connect(self):
        self.cname="monitor-%d" % os.getpid()
        self.client= paho.Client(self.cname) 
        ######Bind function to callback
       

        print("connecting to broker ",self.host,":",self.port)
        self.client.connect(self.host,self.port)#connect
        print("connected");
    def Disconnect(self):
        self.client.disconnect()
        self.client=None
        
    def on_topics(self,client, userdata, message):
        #time.sleep(1)
        print(message.topic)
        logging.info("received topic ="+str(message.topic))
        logging.info("received message ="+str(message.payload.decode("utf-8")))
        if (not message.topic in self.topics):
            self.topics.append(str(message.topic))
            self.topicinfos.append(message.payload.decode("utf-8"))
    def ListTopics(self):
        self.client.on_message=self.on_topics
        self.client.loop_start() #start loop to process received messages
        logging.info("subscribing all on "+self.settings["broker"]["host"]+" port %d " % self.settings["broker"]["port"])
        self.client.subscribe("#")#subscribe
        time.sleep(6)
        self.client.unsubscribe("#")
        print(self.topics)
        self.client.loop_stop()
        idt=0
        for s in self.topics:

            p=s.split("/")
            if (len(p)<4):
                continue
            idt=idt+1
            print("Process ",s,p);
            # Find PahoInterface processes
            if (p[3]=="INFOS"):
                logging.info("Command "+self.topicinfos[idt-1])
                ss=p[0]+"/"+p[1]+"/"+p[2]
                if (ss in self.subtop.keys()):
                    self.subtop[ss]["valid"]=1


    def on_message(self,client, userdata, message):
        #time.sleep(1)
        #print(message.timestamp)

        #print("received topic =",str(message.topic))
        #print("received message =",str(message.payload.decode("utf-8")))
        p=message.topic.split("/")

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
            self.msi.store(p[0],p[1], r_m["ctime"], r_m["content"])
            self.msi.store_mqtt(message.topic,message.timestamp, r_m["ctime"], r_m["content"])
            sm=p[0]+p[1]+json.dumps(r_m)
            logging.debug(sm)
        else:
            if (False):
                print("No db storage")
            #print(r_m)
        if (p[1]=='BmpPaho'):
            metric=p[0]+"."+p[1]+".P"
            self.gsender.send(metric,r_m["content"]["pressure"])
            metric=p[0]+"."+p[1]+".T"
            self.gsender.send(metric,r_m["content"]["temperature"])
        if (p[1]=='HihPaho'):
            metric=p[0]+"."+p[1]+".h0"
            self.gsender.send(metric,r_m["content"]["h0"])
            metric=p[0]+"."+p[1]+".t0"
            self.gsender.send(metric,r_m["content"]["t0"]-273.15)
            metric=p[0]+"."+p[1]+".h1"
            self.gsender.send(metric,r_m["content"]["h1"])
            metric=p[0]+"."+p[1]+".t1"
            self.gsender.send(metric,r_m["content"]["t1"]-273.15)
        if (p[1]=='GenesysPaho'):
            metric=p[0]+"."+p[1]+".vout"
            self.gsender.send(metric,r_m["content"]["vout"])
            metric=p[0]+"."+p[1]+".iout"
            self.gsender.send(metric,r_m["content"]["iout"])
        if (p[1]=='ZupPaho'):
            metric=p[0]+"."+p[1]+".vout"
            self.gsender.send(metric,r_m["content"]["vout"])
            metric=p[0]+"."+p[1]+".iout"
            self.gsender.send(metric,r_m["content"]["iout"])
    def UpdateInfos(self):
        for x in self.subtop.keys():
            if (self.subtop[x]["valid"]==0):
                continue
            
            logging.info("sending Status to %s \n",x)
            self.sendCommand(x,"STATUS",{})

    def loop(self):
        self.client.on_message=self.on_message
        self.client.loop_start() #start loop to process received messages

        for x in self.subtop.keys():
            if (self.subtop[x]["valid"]==1):
                s_status=x+"/STATUS"
                self.client.subscribe(s_status)#subscribe
                logging.info("subscribing "+s_status)
                
    def sendCommand(self,topic,command,params):
        topicmd=topic+"/CMD"
        msg={}
        msg["name"]=command
        msg["params"]=params
        self.client.publish(topic, payload=json.dumps(msg), qos=0, retain=False)

    def printBmp(self,npmax=1):
        for x in self.settings["broker"]["sessions"]:
            session=x["name"]
            for y in x["services"]:                
                topic=session+"/BmpPaho/0/STATUS"
                if (not topic in self.rcv_msg.keys()):
                    continue
                nm=len(self.rcv_msg[topic])
                nr=0
                for i in range(nm-1,-1,-1):
                    if (nr>=npmax):
                        break
                    nr=nr+1
                    x=self.rcv_msg[topic][i]
                    sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
                    #if (device=="BMP" and x["status"]["name"]==device):
                    print("%s P=%.2f mbar T=%.2f K %.2f C " % (sti,x["content"]["pressure"],x["content"]["temperature"]+273.15,x["content"]["temperature"]))
    def bmpInfo(self,npmax=1):
        res=[]
        for x in self.settings["broker"]["sessions"]:
            session=x["name"]
            for y in x["services"]:                
                topic=session+"/BmpPaho/0/STATUS"
                if (not topic in self.rcv_msg.keys()):
                    continue
                nm=len(self.rcv_msg[topic])
                nr=0

                for i in range(nm-1,-1,-1):
                    if (nr>=npmax):
                        break
                    nr=nr+1
                    x=self.rcv_msg[topic][i]
                    res.append(x)
                    sti=time.strftime('%s %Y-%m-%d %H:%M:%S',time.localtime(x["ctime"]))
            #if (device=="BMP" and x["status"]["name"]==device):
        return res

            
if __name__=='__main__':
    s=slowcontrol("/etc/slowcontrol.json")
    s.Connect()
    time.sleep(4)
    s.ListTopics()
    s.loop()
    while (True):
        time.sleep(1)
