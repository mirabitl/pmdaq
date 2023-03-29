import time
import json
import paho.mqtt.client as paho
import os
import MongoMqtt as ms
import logging
import graphyte

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
        while self.flag_connected==0:
            self.client.reconnect()
            print("reconnecting ",self.flag_connected)
            time.sleep(1)
            if (self.flag_connected==1):
                 self.client.loop_stop()
                 self.loop()

    def on_message(self,client, userdata, message):
        #time.sleep(1)
        #print(message.timestamp)

        print("received topic =",str(message.topic))
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
                print(metric,x)
                try:
                    self.gsender.send(metric,r_m["content"][x])
                except Exception as error:
                    print("Error sending ",x,r_m["content"])
                    break
    def Connect(self):
        self.cname="monitor-%d" % os.getpid()
        self.client= paho.Client(self.cname)
        self.client.on_connect=self.on_connect
        self.client.on_disconnect=self.on_disconnect
        ######Bind function to callback
       

        print("connecting to broker ",self.host,":",self.port)
        self.client.connect(self.host,self.port)#connect
        print("connected");
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
        


            

#s=monitor("lyocms09.in2p3.fr",1883,"dome_sdhcal")
#s.Connect()
#time.sleep(4)
#s.ListTopics()
#s.loop()
#while (True):
#    time.sleep(1)
if __name__ == "__main__":
    s=pico_monitor("lyoilc07",1883,"pico_test")
    s.Connect()
    #s.ListTopics()
    s.loop()
    while 1:
        time.sleep(1)
