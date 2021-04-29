from __future__ import absolute_import
from __future__ import print_function
import os
import socks
import socket
import six.moves.http_client
import six.moves.urllib.request, six.moves.urllib.parse, six.moves.urllib.error
import six.moves.urllib.request, six.moves.urllib.error, six.moves.urllib.parse
from six.moves.urllib.error import URLError, HTTPError
import json
from copy import deepcopy
import base64
import time
import requests
import six
import serviceAccess as sac

class pnsAccess:
    def __init__(self):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.pns_host=os.getenv("PNS_NAME","NONE")
        if (self.pns_host == "NONE"):
            print("The ENV varaible PNS_NAME mut be set")
            exit(0)
        self.check()
    def check(self):
        self.process_list= self.pns_list()
        print("process  list: ",self.process_list)
        self.session_list=self.pns_session_list()
        print("session  list: ",self.session_list)
        self.registered=[]
        if (self.process_list["REGISTERED"]!= None):
            for x in self.process_list["REGISTERED"]:
                print(x)
                self.registered.append(sac.strip_pns_string(x))

        #xf=None
        for x in self.registered:
            print("Host:",x.host,"Port :",x.port,"Path :",x.path,"State :",x.state)
            rep=json.loads(sac.executeCMD(x.host,x.port,x.path+"INFO",None))
            if ( 'error' in rep):
                print(rep)
                x.state="DEAD"
            else:
                if (rep["STATE"]!= x.state):
                    print(rep["STATE"]," found different from store one",x.state)
                    x.state=rep["STATE"]
        #    if (x.path=="/last_feb/evb_builder/0/"):
        #        xf=x
        #if (xf!=None):
        #    self.registered.remove(xf)
        for x in self.registered:
            print("After Host:",x.host,"Port :",x.port,"Path :",x.path,"State :",x.state," Session :",x.session," Name : ",x.name," Instance ",x.instance )

        #json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/LIST",{}))
        #.decode("utf-8"))
    def pns_list(self,req_session="NONE"):
        par={}
        par["session"]=req_session
        #print(par)
        pl=json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/LIST",par))
        return pl
    def pns_session_list(self,req_session="NONE"):
        pl=json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/LIST",{"session":req_session}))
        return pl
    def pns_session_update(self,new_state):
        par={}
        par["session"]=self.session
        par["state"]=new_state
        print("Updating session ",par)
        pl=json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/UPDATE",par))
        return pl
    def pns_remove(self,req_session=None):

        # One loop for all but evb_builder
        for x in self.registered:
            if (req_session!=None and x.session!=req_session):
                continue
            if (x.name=="evb_builder"):
                continue;
            print("removing Host:",x.host,"Port :",x.port,"Path :",x.path,"State :",x.state," Session :",x.session," Name : ",x.name," Instance ",x.instance )
            par={}
            par["session"]=x.session
            par["name"]=x.name
            par["instance"]=x.instance
            r_services=sac.executeCMD(x.host,x.port,"/REMOVE",par)
        #  One loop for evb
        for x in self.registered:
            if (req_session!=None and x.session!=req_session):
                continue
            if (x.name!="evb_builder"):
                continue;
            print("removing Host:",x.host,"Port :",x.port,"Path :",x.path,"State :",x.state," Session :",x.session," Name : ",x.name," Instance ",x.instance )
            par={}
            par["session"]=x.session
            par["name"]=x.name
            par["instance"]=x.instance
            r_services=sac.executeCMD(x.host,x.port,"/REMOVE",par)
        # PURGE
        sac.executeCMD(self.pns_host,8888,"/PNS/PURGE",{})
        if (req_session!= None):
            sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/UPDATE",{"session":req_session,"state":"DEAD"})
        sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/PURGE",{})
        self.check()
        return    

    def Print(self,session,verbose=False):
        for x in self.registered:
            if (x.session!=session):
                 continue
            y=sac.serviceAccess(x.host,x.port,x.session,x.name,x.instance)
            y.printInfos(verbose)
