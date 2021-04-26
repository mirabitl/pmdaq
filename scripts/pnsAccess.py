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
        self.process_list= self.pns_list()
        print("process  list: ",self.process_list)
        self.sessions_list=self.pns_session_list()
        print("session  list: ",self.session_list)
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
    