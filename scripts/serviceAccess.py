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

try:
    from urllib.parse import urlparse
except ImportError:
     from six.moves.urllib.parse import urlparse
# Sock support
sockport = None
sp = os.getenv("SOCKPORT", "Not Found")
if (sp != "Not Found"):
    sockport = int(sp)
if (sockport != None):
    # print "Using SOCKPORT ",sockport
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", sockport)
    socket.socket = socks.socksocket
    # socks.wrapmodule(urllib2)
def executeRequest(url):
    """
    Access to an url
    
   @param surl: The url
   @return: url answer as a text
   """
    try:
        r = requests.get(url)
    except requests.exceptions.RequestException as e:
        print(e)
        p_rep={}
        p_rep["STATE"]="DEAD"
        p_rep["http_error"] = e.code
        return json.dumps(p_rep,sort_keys=True)
        #print(r.url)
        #print(r.headers)
#        if (r.headers['Content-Type']=="application/json"):
#            print(r.text)
#            return r.text
        #print(r.text)
    return r.text

    #print "Access to %s " % url
    req = six.moves.urllib.request.Request(url)
    try:
        r1 = six.moves.urllib.request.urlopen(req)
    except URLError as e:
        p_rep = {}
        print(e.code)
        print(e.reason)
        p_rep["http_error"] = e.code
        return json.dumps(p_rep, sort_keys=True)
    #print("RC :",r1.status)
    return r1.read()

def executeCMD(host,port,path,params):
    """
        Access to a command or a transition of a pmdaq service
        
        @param host: Host name
        @param port: Application port
        @param path: The complete PATH of the service session/pluggin/instance/command
        @param params: CGI additional parameters
        @return: url answer as text
    """

    if (params!=None ):
        myurl = "http://"+host+ ":%d" % (port)

        lq={}
        for x,y in six.iteritems(params):
            if (type(y) is dict):
                y=json.dumps(y).replace(" ","").encode("utf8")
                #print("STRING ",y)
            lq[x]=y

        #lqs=six.moves.urllib.parse.urlencode(lq)


        #payload = lq
        #print(payload)
        try:
            r = requests.get(myurl+path, params=lq)
        except requests.exceptions.RequestException as e:
            print(e)
            p_rep={}
            p_rep["STATE"]="DEAD"
            p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
        #print(r.url)
        #print(r.headers)
#        if (r.headers['Content-Type']=="application/json"):
#            print(r.text)
#            return r.text
        #print(r.text)
        return r.text



        
        #saction = '%s?%s' % (path,lqs)
        #myurl=myurl+saction
        #print(myurl)
        

        
        #req=six.moves.urllib.request.Request(myurl)
        #try:
        #    r1=six.moves.urllib.request.urlopen(req)
        #except URLError as e:
        #    print(e)
        #    print(e.code)
        #    print(e.reason)

        #    p_rep={}
        #    p_rep["STATE"]="DEAD"
        #    p_rep["http_error"] = e.code
        #    return json.dumps(p_rep,sort_keys=True)
        #else:
        #    #print("RC :",r1.status)
        #    return r1.read()

    else:
        myurl = "http://"+host+ ":%d%s" % (port,path)
        #print(myurl)
        try:
            r = requests.get(myurl)
        except requests.exceptions.RequestException as e:
            print(e)
            p_rep={}
            p_rep["STATE"]="DEAD"
            p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
        return r.text
        #print(myurl)
        req=six.moves.urllib.request.Request(myurl)
        try:
            r1=six.moves.urllib.request.urlopen(req)
        except URLError as e:
            print(e)
            print(e.code)
            print(e.reason)
            p_rep={}
            p_rep["STATE"]="DEAD"
            p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
        else:
            #print("RC :",r1.status)
            return r1.read()

        
def strip_pns_string(pns_string):
    state=pns_string.split("?")[1]
    v=pns_string.split("?")[0].split(":")
    host=v[0];
    port=int(v[1]);
    path =v[2]
    vp=path.split("/")
    session=vp[1]
    name=vp[2]
    instance=int(vp[3])
    o = type('serviceAccessDescription', (object,), 
                 {'host':host, 'port':port,'path':path,'session':session,'name':name,'instance':instance,'state':state})()
    #print(o)
    #print("=>",o.host,o.port,o.path,o.session,o.name,o.instance,o.state)
    return o

def create_access(pns_string):
     o=strip_pns_string(pns_string)
     return serviceAccess(o.host,o.port,o.session,o.name,o.instance)
    
def pns_info_command():
    pns_host=os.getenv("PNS_NAME","NONE")
    if (pns_host == "NONE"):
        print("The ENV varaible PNS_NAME mut be set")
        exit(0)
    r_pns_list=executeCMD(pns_host,8888,"/PNS/LIST",{})
    if (type(r_pns_list) is bytes):
        r_pns_list=r_pns_list.decode("utf-8")
    pns_list=json.loads(r_pns_list)
    if ("http_error" in r_pns_list):
        return
    #print(pns_list)
    if ("REGISTERED" in pns_list):
        #print(pns_list["REGISTERED"])
        if ( pns_list["REGISTERED"]!=None):
            for x in pns_list["REGISTERED"]:
                o =strip_pns_string(x)
                print("=>",o.host,o.port,o.path,o.session,o.name,o.instance,o.state)

class serviceAccess:
    def __init__(self, vhost, vport,vsession,vname,vinstance):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.host = vhost
        self.port = vport
        self.session = vsession
        self.name = vname
        self.instance = vinstance
        self.url = "http://%s:%d" % (vhost, vport)
        self.state="VOID"
        self.path="/%s/%s/%d/" % (vsession,vname,vinstance)
        self.pns_request()
        self.services=[]
        self.commands=[]
        self.allowed=[]
        self.transitions=[]
        self.params=None
        if (self.state !="VOID" and self.state!="DEAD"):
            self.services_request()
    def pns_request(self,cmd="LIST",par=None):
        pns_host=os.getenv("PNS_NAME","NONE")
        if (pns_host == "NONE"):
            print("The ENV varaible PNS_NAME mut be set")
            exit(0)
        r_pns_list=executeCMD(pns_host,8888,"/PNS/%s" % cmd,par)
        if (type(r_pns_list) is bytes):
            r_pns_list=r_pns_list.decode("utf-8")
        pns_list=json.loads(r_pns_list)
        if ("http_error" in r_pns_list):
            return
        #print(pns_list)
        if ("REGISTERED" in pns_list):
            #print(pns_list["REGISTERED"])
            if ( pns_list["REGISTERED"]!=None):
                for x in pns_list["REGISTERED"]:
                    o =strip_pns_string(x)
                    #print( iho,ipo,ipa,ises,ina,iin)
                    #st=x.split('?')[0].split(':')[2]
                    if (o.path==self.path):
                        self.state=o.state
    def services_request(self):
        r_services=executeCMD(self.host,self.port,"/SERVICES",{})
        #print(r_services)
        if (type(r_services) is bytes):
            r_services=r_services.decode("utf-8")
        services=json.loads(r_services);
        if ("http_error" in services):
            return
        #print(services)
        #print(services)
        for x in services:
            v=x.split(self.path)
            #print(len(v),v,self.services)
            if (len(v)==2):
                self.services.append(v[1])
        if (len(self.services)<3):
            print("Not a FSMW\n")
            return
        # Get info
        r_info=executeCMD(self.host,self.port,"%sINFO" % self.path,{})
        if (type(r_info) is bytes):
            r_info=r_info.decode("utf-8")
        info=json.loads(r_info);
        if ("http_error" in info):
            return

        #print(info)
        # test STATE
        if (info["STATE"]!=self.state):
            print("PNS incoherency PNS=%s Process=%s" % (info["STATE"],self.state))
            return
        # Store available commads, transitions and allowed
        self.commands=[]
        self.allowed=[]
        self.transitions=[]
        for x in info['COMMANDS']:
            v=x.split("/")
            self.commands.append(v[len(v)-1])
        for x in info['TRANSITIONS']:
            v=x.split("/")
            self.transitions.append(v[len(v)-1])
        for x in info['ALLOWED']:
            v=x.split("/")
            self.allowed.append(v[len(v)-1])
        #Get parameter
        r_params=executeCMD(self.host,self.port,"%sPARAMS" % self.path,{})
        if (type(r_params) is bytes):
            r_params=r_params.decode("utf-8")
        jpar=json.loads(r_params);
        if ("http_error" in jpar):
            return
        self.params=jpar
        #print(params)
    def create(self,param_s=None):
        if (self.state=='VOID' or self.state=='DEAD'):
            par={}
            par["session"]=self.session
            par["name"]=self.name
            par["instance"]=self.instance
            par["params"]=""
            if (param_s!=None):
                par["params"]=param_s
            #print(self.host,self.port,par)
            r_services=executeCMD(self.host,self.port,"/REGISTER",par)
            #if (type(r_services) is bytes):
            #    r_services=r_services.decode("utf-8")
            #jserv=json.loads(r_services);
            #if ("http_error" in jserv):
            #    return
            
            #print(r_services)
            self.pns_request()
            if (self.state !="VOID"):
                self.services_request()
    def remove(self,par=None):
        if (self.state!='VOID'):
            par={}
            par["session"]=self.session
            par["name"]=self.name
            par["instance"]=self.instance
            print("Sending remove",par)
            r_services=executeCMD(self.host,self.port,"/REMOVE",par)
            #if (type(r_services) is bytes):
            #    r_services=r_services.decode("utf-8")
            #jserv=json.loads(r_services);
            #if ("http_error" in jserv):
            #    return
            #print(r_services)
            self.pns_request()
            print("Removed state is %s \n" % self.state)

    def getInfo(self):
        self.pns_request()
        self.services=[]
        self.commands=[]
        self.allowed=[]
        self.transitions=[]
        self.params=None
        if (self.state !="VOID"):
            self.services_request()

    def allInfos(self):
        jdict={}
        jdict['state']=self.state
        jdict['params']=self.params
        jdict['services']=self.services
        return jdict
    def isBaseApplication(self):
        return "PARAMS" in self.commands

    def sendCommand(self, name, content):
        self.getInfo()
        isValid = name in self.commands
        if (not isValid):
            return '{"answer":"invalid command ","status":"FAILED"}'
        rep=executeCMD(self.host,self.port,"%s%s" % (self.path,name),content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")
        return rep
    def sendTransition(self, name, content):
        self.getInfo()
        #print "Send Transition",self.procInfos
        isValid = False
        isValid = name in self.allowed
        if (not isValid):
            return '{"answer":"invalid transition","status":"FAILED"}'

        rep=executeCMD(self.host,self.port,"%s%s" % (self.path,name),content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")

        # update state (published is asynchronous)
        #time.sleep(10000/1000000.0)
        self.getInfo()
        #print("New State is ",self.state)
        return rep

    def printInfos(self, vverb):
        if (vverb):
            print("FSM is %s on %s,  Service %s"  % (self.state, self.url, self.path))
            # print COMMAND and TRANSITION
            s= " \t Commands \t"
            for x in self.commands:
                s=s+x+" "
            print(s)
            s= " \t Transitions \t"
            for x in self.transitions:
                s=s+x+" "
            print(s)
            s= " \t Allowed \t"
            for x in self.allowed:
                s=s+x+" "
            print(s)
            print("\t Parameters")
            for k, v in self.params.items():
                print("\t \t", k, v)
        else:
            print("FSM is %s on %s,  Service %s"  % (self.state, self.url, self.path))
            r={}
            r["state"]=self.state
            r["url"]=self.url
            r["path"]=self.path
            r["commands"]=self.commands
            r["transitions"]=self.transitions
            r["allowed"]=self.allowed
            r["paramss"]=self.params
            return r
