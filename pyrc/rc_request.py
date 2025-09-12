import os
import socks
import socket
import json
from copy import deepcopy
import base64
import time
import requests

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
    return r.text


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
        for x,y in params.items():
            if (type(y) is dict):
                y=json.dumps(y).replace(" ","").encode("utf8")
                #print("STRING ",y)
            lq[x]=y
        try:
            r = requests.get(myurl+path, params=lq)
        except requests.exceptions.RequestException as e:
            print(e)
            p_rep={}
            p_rep["STATE"]="DEAD"
            p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
        return r.text
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

