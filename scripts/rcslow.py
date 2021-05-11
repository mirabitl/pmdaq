from __future__ import absolute_import
from __future__ import print_function
import serviceAccess as sac
import pnsAccess as pns
import session
# import MongoJob as mg
import json
import time
import os



class slowControl:
    def __init__(self, config_file):
        self.config_file = config_file
        # parse config file
        self.session = session.create_session(config_file)
        # DB access
        # SLOW PART
        self.slow_answer = None
        

    def updateInfo(self,printout,vverbose):
        self.session.Print(versbose)
    def processCommand(self,cmd,appname,param):
        r=self.session.commands(cmd,appname,param)
        return json.dumps(r)
  
    def configure(self):
        for k, v in self.session.apps.items():
            print("configuring",k)
            for s in v:
                m = {}
                mr = json.loads(s.sendTransition("CONFIGURE", {}))
                print(mr)

    def start(self):
        for k, v in self.session.apps.items():
            print("starting",k)
            for s in v:
                m = {}
                mr = json.loads(s.sendTransition("START", {}))
                print(mr)

    def stop(self):
        for k, v in self.session.apps.items():
            print("stopping",k)
            for s in v:
                m = {}
                mr = json.loads(s.sendTransition("STOP", {}))
                print(mr)
        self.storeState()
    def destroy(self):
        for k, v in self.session.apps.items():
            print("halting",k)
            for s in v:
                m = {}
                mr = json.loads(s.sendTransition("HALT", {}))
                print(mr)
        self.storeState()
    def Status(self,verbose):
        for k, v in self.session.apps.items():
            print("Status",k)
            for s in v:
                print(s.path)
                m = {}
                mr = json.loads(s.sendCommand("STATUS", {}))
                if (k == 'app_bmp'):
                    x=mr["status"]
                    print(" P=%.2f mbar T=%.2f K " % (x["pressure"],x["temperature"]+273.15))
                    continue
                if (k == 'app_genesys'):
                    x=mr["status"]
                    print(" VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (x["vset"],x["vout"],x["iout"],x["status"]))
                    continue
                if (k == 'app_wiener'):
                    x=mr["status"]
                    for y in x["channels"]:
                        sstat=y["status"].split("=")[1]
                    
                        print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))   
                    continue
                print(mr)

