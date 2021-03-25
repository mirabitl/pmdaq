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

def create_session(config):
    j_sess=json.loads(open(config).read())
    #print(j_sess)
    vsession=j_sess['session']
    if ('apps' in j_sess):
        for x in j_sess['apps']:
            a=sac.serviceAccess(x['host'],x['port'],vsession,x['name'],x['instance'])
            if (a.state=='VOID'):
                par={}
                if ('params' in x):
                    par=x['params']
                a.create(par)
                #par={}
                #par['params']=x['params']
                #a.sendCommand("SETPARAMS",par)
    return sessionAccess(vsession)
                
            
class sessionAccess:
    def __init__(self,vsession):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.session = vsession
        self.apps={}
        self.pns_host=os.getenv("PNS_NAME","NONE")
        if (self.pns_host == "NONE"):
            print("The ENV varaible PNS_NAME mut be set")
            exit(0)
        pl= self.pns_list()
        #json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/LIST",{}))
        #.decode("utf-8"))
        if ("REGISTERED" in pl):
            if ( pl["REGISTERED"]!=None):
                for x in pl["REGISTERED"]:
                    o =sac.strip_pns_string(x)
                    if (o.session!=self.session):
                        continue
                    #print( iho,ipo,ipa,ises,ina,iin)
                    #st=x.split('?')[0].split(':')[2]
                    if (o.name in self.apps):
                        self.apps[o.name].append(sac.serviceAccess(o.host,o.port,o.session,o.name,o.instance))
                    else:
                        self.apps[o.name]=[]
                        self.apps[o.name].append(sac.serviceAccess(o.host,o.port,o.session,o.name,o.instance))
    def name(self):
        return self.session
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
    def Print(self,verbose=False):
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            for y in app:
                y.printInfos(verbose)
                
    def remove(self,obj_name=None):
        # First skip event builder (ZMQ issue)
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            if (obj_name!=None and obj_name!=name):
                continue
            if (name == "evb_builder"):
                continue
            for y in app:
                y.remove()
        # Now remove evb_builder
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            if (obj_name!=None and obj_name!=name):
                continue
            if (name != "evb_builder"):
                continue
            for y in app:
                y.remove()
        # clear PNS
        print("purging PNS")
        sac.executeCMD(self.pns_host,8888,"/PNS/PURGE",{})
        # removing session
        print("purging PNS/SESSION")
        sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/UPDATE",{"session":self.name(),"state":"DEAD"})
        sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/PURGE",{})
    def commands(self,cmd,obj_name,params=None):
        rep={}
        for name,app in six.iteritems(self.apps):
            if (obj_name!=None and obj_name!=name):
                continue
            for y in app:
                r= y.sendCommand(cmd,params)
                rep[y.path]=json.loads(r)
        # clear PNS
        return rep
    def transitions(self,cmd,obj_name,params=None):
        rep={}
        for name,app in six.iteritems(self.apps):
            if (obj_name!=None and obj_name!=name):
                continue
            for y in app:
                r= y.sendTransition(cmd,params)
                rep[y.path]=r
                #print(y.path,r)
                
        # clear PNS
        return rep

