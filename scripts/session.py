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
    """!
    Parse the configuration files, build the corresponding serviceAccess and REGISTER all plugins if needed
    @param config The configuration file
    @return The sessionAccess object built with the configuration dictionnary
    """
    j_sess=json.loads(open(config).read())
    #print(j_sess)
    vsession=j_sess['session']
    if ('apps' in j_sess):
        for x in j_sess['apps']:
            a=sac.serviceAccess(x['host'],x['port'],vsession,x['name'],x['instance'])
            #print(x['host'],x['port'],vsession,x['name'],x['instance'],a.state)
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
        """!
        Handle all application of the session. The applictaion are stored in a map of pluggin names each entry conatining a list of serviceAccess to each instance of the plugin
        @param vsession The session dictionnary
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
        """! getter of session name"""
        return self.session
    def pns_list(self,req_session="NONE"):
        """! Call PNS/LIST for a session name
        @param req_session Session name
        @return Python object built from PNS/LIST answer
        """
        par={}
        par["session"]=req_session
        #print(par)
        pl=json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/LIST",par))
        return pl
    def pns_session_list(self,req_session="NONE"):
        """! Call PNS/SESSION/LIST for a session name
        @param req_session Session name
        @return Python object built from PNS/SESSION/LIST answer
        """
        pl=json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/LIST",{"session":req_session}))
        return pl
    def pns_session_update(self,new_state):
        """!  Update PNS/SESSION state 
        @param new_state New value of the state
        @return Python object built from PNS/SESSION/UPDATE answer
        """
        par={}
        par["session"]=self.session
        par["state"]=new_state
        print("Updating session ",par)
        pl=json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/SESSION/UPDATE",par))
        return pl
    def Print(self,verbose=False):
        """! Print out of all serviceAccess
        """
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            for y in app:
                y.printInfos(verbose)
                
    def remove(self,obj_name=None):
        """! Call the REMOVE command for all services
        @param obj_name if not None remove only plugins with this name

        It first removes all plugins but the evb_builder
        It then removes the evb_builder plugins
        Then it purges the PNS, tag the PNS/SESSION as DEAD and purge the PNS/SESSION

        """
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
        """! Send a command to a plugin group
        @param cmd Command name
        @param obj_name The plugin name
        @param params The CGI parameters
        @return Python object of the answer
        """
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
        """! Send a transition to a plugin group
        @param cmd Transition name
        @param obj_name The plugin name
        @param params The CGI parameters
        @return Python object of the answer
        """
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

