import time
import json
import paho.mqtt.client as paho
import os
import MongoSlow as ms
import logging

class monitor:
    def __init__(self,host,port,session):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.port=port
        self.host=host
        self.session=session
        self.client=None
        self.topics=[]
        self.topicinfos=[]
        self.subtop=[]
        self.hws=[]
        self.rcv_msg={}
        self.msi=None
        self.lv_hw="ZupPaho"
        self.hv_hw="WienerPaho"
        logging.basicConfig(level=logging.INFO)

    def connectMongo(self):
        self.msi=ms.instance()
        
    def on_topics(self,client, userdata, message):
        #time.sleep(1)

        print("received topic =",str(message.topic))
        print("received message =",str(message.payload.decode("utf-8")))
        if (not message.topic in self.topics):
            self.topics.append(message.topic)
            self.topicinfos.append(message.payload.decode("utf-8"))

    def on_message(self,client, userdata, message):
        #time.sleep(1)
        #print(message.timestamp)

        #print("received topic =",str(message.topic))
        #print("received message =",str(message.payload.decode("utf-8")))

        p=message.topic.split("/")
        if (p[0]!=self.session):
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
            self.msi.store(p[0],p[1], r_m["ctime"], r_m["content"])
            sm=p[0]+p[1]+json.dumps(r_m)
            logging.debug(sm)
        else:
            if (False):
                print("No db storage")
            #print(r_m)
        ## BMP data
        if (p[1]=="BmpPaho"):
            #print(p[3],message.payload.decode("utf-8"))
            if (p[3]=="STATUS"):
                loc=p[0]
                hw=p[1]
                ti=message.timestamp
                sta=json.loads(message.payload.decode("utf-8"))
                #print(loc,hw,ti,sta)
    def Connect(self):
        self.cname="monitor-%d" % os.getpid()
        self.client= paho.Client(self.cname) 
        ######Bind function to callback
       

        print("connecting to broker ",self.host,":",self.port)
        self.client.connect(self.host,self.port)#connect
        print("connected");
    def ListTopics(self):
        self.client.on_message=self.on_topics
        self.client.loop_start() #start loop to process received messages
        print("subscribing all ")
        self.client.subscribe("#")#subscribe
        time.sleep(6)
        self.client.unsubscribe("#")
        print(self.topics)
        self.client.loop_stop()
        idt=0
        for s in self.topics:
            p=s.split("/")
            idt=idt+1
            if (p[0]!=self.session):
                continue
            print("Process ",s,p[1]);
            if (p[3]=="INFOS"):
                print("Command",self.topicinfos[idt-1])
                ss=p[0]+"/"+p[1]+"/"+p[2]+"/STATUS"
                if (not ss in self.subtop):
                    self.subtop.append(ss)
                    self.hws.append(p[1])
    def UpdateInfos(self):
        for hw in self.hws:
            logging.info("sending Status to %s \n",hw)
            self.sendCommand(hw,"STATUS",{})

    def loop(self):
        self.client.on_message=self.on_message
        self.client.loop_start() #start loop to process received messages

        for x in self.subtop:
            self.client.subscribe(x)#subscribe
            print("subscribing ",x)
    def sendCommand(self,process,command,params):
        topic=self.session+"/"+process+"/0/CMD"
        msg={}
        msg["name"]=command
        msg["params"]=params
        self.client.publish(topic, payload=json.dumps(msg), qos=0, retain=False)
    def printBmp(self,npmax=1):
        topic=self.session+"/BmpPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
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
        topic=self.session+"/BmpPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
        nm=len(self.rcv_msg[topic])
        nr=0
        res=[]
        for i in range(nm-1,-1,-1):
            if (nr>=npmax):
                break
            nr=nr+1
            x=self.rcv_msg[topic][i]
            res.append(x)
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            #if (device=="BMP" and x["status"]["name"]==device):
        return res

    def printZup(self,npmax=1):
        topic=self.session+"/ZupPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
        nm=len(self.rcv_msg[topic])
        nr=0
        for i in range(nm-1,-1,-1):
            if (nr>=npmax):
                break
            nr=nr+1
            x=self.rcv_msg[topic][i]
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            #if (device=="BMP" and x["status"]["name"]==device):
            print("%s Vset %.2f  Vout %.2f  Iout %.2f Status %d " % (sti,x["content"]["vset"],x["content"]["vout"],x["content"]["iout"],x["content"]["status"]))
            
    def zupInfo(self,npmax=1):
        topic=self.session+"/ZupPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
        nm=len(self.rcv_msg[topic])
        nr=0
        res=[]
        for i in range(nm-1,-1,-1):
            if (nr>=npmax):
                break
            nr=nr+1
            x=self.rcv_msg[topic][i]
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            res.append(x)
            #if (device=="BMP" and x["status"]["name"]==device):
            #print("%s Vset %.2f  Vout %.2f  Iout %.2f Status %d " % (sti,x["content"]["vset"],x["content"]["vout"],x["content"]["iout"],x["content"]["status"]))
        return res
            
    def printGenesys(self,npmax=1):
        topic=self.session+"/GenesysPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
        nm=len(self.rcv_msg[topic])
        nr=0
        for i in range(nm-1,-1,-1):
            if (nr>=npmax):
                break
            nr=nr+1
            x=self.rcv_msg[topic][i]
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            #if (device=="BMP" and x["status"]["name"]==device):
            print("%s Vset %.2f  Vout %.2f  Iout %.2f  " % (sti,x["content"]["vset"],x["content"]["vout"],x["content"]["iout"]))

    def LV_ON(self,hw=None):
        if (hw!=None):
            self.lv_hw=hw
        print("Hardware ",self.lv_hw)
        self.sendCommand(self.lv_hw,"ON",{})
    def LV_OFF(self,hw=None):
        if (hw!=None):
            self.lv_hw=hw
        self.sendCommand(self.lv_hw,"OFF",{})
    def HV_ON(self,channels,hw=None):
        if (hw!=None):
            self.hv_hw=hw

        for ch in channels:
            self.sendCommand(self.hv_hw,"ON",{"first":ch,"last":ch})
    def HV_OFF(self,channels,hw=None):
        if (hw!=None):
            self.hv_hw=hw
        for ch in channels:
            self.sendCommand(self.hv_hw,"OFF",{"first":ch,"last":ch})
    def HV_VSET(self,channels,vset,hw=None):
        if (hw!=None):
            self.hv_hw=hw
        for ch in channels:
            self.sendCommand(self.hv_hw,"VSET",{"first":ch,"last":ch,"vset":vset})
    def HV_ISET(self,channels,vset,hw=None):
        if (hw!=None):
            self.hv_hw=hw
        for ch in channels:
            self.sendCommand(self.hv_hw,"ISET",{"first":ch,"last":ch,"iset":vset})

    def HV_CLEAR(self,channels,hw=None):
        if (hw!=None):
            self.hv_hw=hw
        for ch in channels:
            self.sendCommand(self.hv_hw,"CLEARALARM",{"first":ch,"last":ch})



    def printWiener(self,npmax=1,channel=-1):
        topic=self.session+"/WienerPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
        nm=len(self.rcv_msg[topic])
        nr=0
        for i in range(nm-1,-1,-1):
            if (nr>=npmax):
                break
            nr=nr+1
            x=self.rcv_msg[topic][i]
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            print(sti)
            for y in x["content"]["channels"]:
                if (channel!=-1 and y["id"]!=channel):
                    continue
                if (not isinstance(y["status"],str)):
                    print(y)
                    continue
                sstat=y["status"].split("=")[1]
                    
                    #print("ch%.3d %12.2f %12.2f %12.2f %s" %(y["id"],y["vset"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))

    def WienerInfo(self,npmax=1):
        topic=self.session+"/WienerPaho/0/STATUS"
        if (not topic in self.rcv_msg.keys()):
            return
        nm=len(self.rcv_msg[topic])
        nr=0
        res=[]
        for i in range(nm-1,-1,-1):
            if (nr>=npmax):
                break
            nr=nr+1
            x=self.rcv_msg[topic][i]
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x["ctime"]))
            res.append(x)
        return res
        


            

#s=monitor("lyocms09.in2p3.fr",1883,"dome_sdhcal")
#s.Connect()
#time.sleep(4)
#s.ListTopics()
#s.loop()
#while (True):
#    time.sleep(1)
