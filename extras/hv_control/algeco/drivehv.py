#!/usr/bin/python3
import time
import json
import paho.mqtt.client as paho
import os
import MongoMqtt as ms
import logging
import graphyte
import random
import csv
def calV(V,P,T,unit="K"):
    #P0=965
    P0=990
    T0=293.15
    if (unit!="K"):
        return  V/(0.2+0.8*P/P0*T0/(T+273.15))
    else:
        return  V/(0.2+0.8*P/P0*T0/(T))
def calVset(Vreq,P,T,unit="K",b=0.2,a=0.8):
    P0=990
    T0=293.15
    corr=b+a*(P/P0*T0/(T))
    if (unit!="K"):
        corr=b+a*(P/P0*T0/(T+273.15))
    pcent=abs(1-corr)*100
    print(f"{Vreq:.1f} P={P:.1f} T={T:.1f} C={corr:.4f} {pcent:.2f}")

    return (Vreq*corr,pcent)
class pico_monitor:
    def __init__(self,host,port,session):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.port=port
        self.host=host
        self.session=session
        self.client=None
        self.flag_connected=0
        self.topics=[]
        self.topicm={}
        #self.topicinfos=[]
        #self.subtop=[]
        #self.hws=[]
        self.rcv_msg={}
        self.msi=None
        login = os.getenv("MGDBMON", "NONE")
        if (login != "NONE"):
            self.connectMongo()
            self.check_topics()
        ghost = os.getenv("GRAPHITEHOST", "NONE")
        self.gsender=None
        if (ghost != "NONE"):
            self.gsender=graphyte.Sender(ghost)
        logging.basicConfig(level=logging.INFO)
        self.channels={}
    def connectMongo(self):
        self.msi=ms.instance()

    def check_topics(self):
        r=self.msi.check_infos(self.session,i_type="INFO")
        for x in r:
            lt=x["topic"].split("/")
            topic=lt[0]+"/"+lt[1]+"/"+lt[2]
            if (not topic in self.topics):
                self.topics.append(topic)
                self.topicm[topic]=x["message"]  
    def on_topics(self,client, userdata, message):
        #time.sleep(1)

        #print("received topic =",str(message.topic))
        #print("received message =",str(message.payload.decode("utf-8")))
        lt=str(message.topic).split("/")
        # At least session/system/device_name/INFOS
        if (len(lt)<4):
            return
        if (lt[0]!=self.session):
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
        tfirst=time.time()
        #time.sleep(1)
        #print(message.timestamp)

        print("received topic =",str(message.topic))
        #,str(message.payload.decode("utf-8")))
        #print("received message =",str(message.payload.decode("utf-8")))

        p=message.topic.split("/")
        if (p[0]!=self.session):
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
        try:
            r_m["content"]=json.loads(str(message.payload.decode("utf-8")));
        except:
            print("cannot parse ",str(message.payload.decode("utf-8")));
            return
        self.rcv_msg[message.topic].append(r_m)
        if (len( self.rcv_msg[message.topic])>1000):
            self.rcv_msg[message.topic].pop(0)

        if (message.topic == "pico_dome/telescope_inlet/hih"):
            self.T=r_m["content"]["T"]

        if (message.topic == "pico_dome/cms_inlet/bme"):
            self.load_settings(self.csv_file)
            self.P=r_m["content"]["P"]/100.
            #self.T=r_m["content"]["T"]
            for k in self.channels.keys():
                vreq,pcent=calVset(self.channels[k]["vset"],self.P,self.T,unit="C",b=0.,a=1.0)
                print(k,self.channels[k]["vset"],vreq,self.channels[k]["vlast"])
                if (vreq<self.channels[k]["vmin"]):
                    continue
                if (vreq>self.channels[k]["vmax"]):
                    continue
                if (abs(vreq-self.channels[k]["vlast"])<5):
                    continue
                if (pcent>5):
                    continue
                self.channels[k]["vlast"]=vreq
                topic_cmd="pico_dome/dome_caen/CMD"
                #topic_cmd = pico_location + "/" + s_sub + "/CMD";
                j_msg = {};
                j_msg["device"] = "sy1527";
                j_msg["command"] = "VSET";
                j_msg["params"] = {};
                j_msg["params"]["first"] =k;
                j_msg["params"]["last"] =k;
                j_msg["params"]["vset"] =vreq
                print(topic_cmd,j_msg)
                self.client.publish(topic_cmd,json.dumps(j_msg));

        # Store in Mongo DB if connected
        if (self.msi!=None):
            #self.msi.store(p[0],p[1], r_m["ctime"], r_m["content"])
            sm=p[0]+p[1]+json.dumps(r_m)
            self.msi.store(message.topic,r_m["timestamp"],r_m["ctime"],r_m["content"])
            logging.debug(sm)
        else:
            if (False):
                print("No db storage")
            #print(r_m)
        ## BMP data
        if (self.gsender!=None):
            
            for x in r_m["content"].keys():
                print(len(p),x)
                if (len(p)>=4 and p[3]=="INFOS"):
                    continue
                if (len(p)>=4 and p[3]=="GAS"):
                    continue
                metric=""
                for i in range(len(p)):
                    metric=metric+p[i]+"."
                metric=metric+x
                #print(metric,x)
                try:
                    self.gsender.send(metric,r_m["content"][x])
                except Exception as error:
                    print("Error sending ",x,r_m["content"])
                    break
        print("%s Processing time : %.3f" % (message.topic,(time.time()-tfirst)))
    def load_settings(self,csv_file):
        f=open(csv_file)
        csv_reader = csv.DictReader(f, delimiter=',')


        for r in csv_reader:
            print(r)
            nr=int(r['channel'])
            if (not nr in self.channels):
                self.channels[nr]={}
                self.channels[nr]["vlast"]=float(r["hvset"])-10
            self.channels[nr]["vset"]=float(r["hvset"])
            self.channels[nr]["vmin"]=float(r["hvmin"])
            self.channels[nr]["vmax"]=float(r["hvmax"])
            self.channels[nr]["vreq"]=float(r["hvset"])

            
        print(self.channels)
        f.close()
    def Connect(self,csv_file):
        if (self.client!=None):
            del self.client
        id=random.randrange(1, 1000)
        self.cname="monitor-%d" % id
        self.client= paho.Client(self.cname)
        self.client.on_connect=self.on_connect
        self.client.on_disconnect=self.on_disconnect
        ######Bind function to callback
       

        print("connecting to broker ",self.host,":",self.port)
        self.client.connect(self.host,self.port,keepalive=600)#connect
        print("connected");
        self.csv_file=csv_file
        self.load_settings(csv_file)
    def ListTopics(self):
        self.client.on_message=self.on_topics
        self.client.loop_start() #start loop to process received messages
        print("subscribing all ")
        self.client.subscribe("#")#subscribe
        time.sleep(3)
        self.client.unsubscribe("#")
        #print(self.topics)
        self.client.loop_stop()
        #idt=0
        for s in self.topics:
            print("Registered topic %s \n",s)
    def loop(self):
        self.client.on_message=self.on_message
        self.client.loop_start() #start loop to process received messages

        for x in self.topics:
            t=x+"/#"
            self.client.subscribe(t)#subscribe
            print("subscribing ",t)
        
    def stop(self):
        self.client.loop_stop() #start loop to process received messages

        for x in self.topics:
            t=x+"/#"
            self.client.unsubscribe(t)#subscribe
            print("subscribing ",t)
        


            

#s=monitor("lyocms09.in2p3.fr",1883,"dome_sdhcal")
#s.Connect()
#time.sleep(4)
#s.ListTopics()
#s.loop()
#while (True):
#    time.sleep(1)
if __name__ == "__main__":
    s=pico_monitor("lyoilc07",1883,"pico_dome")
    s.Connect("drivehv.csv")
    s.ListTopics()
    s.loop()
    while 1:
        if (s.flag_connected==0):
            s.stop()
            s.Connect("drivecms.csv")
            s.loop()
        time.sleep(1)
