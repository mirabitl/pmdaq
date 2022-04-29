from __future__ import absolute_import
from __future__ import print_function
import serviceAccess as sac
import pnsAccess as pns
import session
import MongoSlow as mgs
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
                if (k == 'app_syx27'):
                    x=mr["status"]
                    for y in x["channels"]:                    
                        print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %d" %(y["id"],y["vset"],y["iset"],y["rampup"],y["vout"],y["iout"],y["status"]))   
                    continue

                print(mr)

    def keepHv(self,vwanted,channel,period=600,bmpname="app_bmp",hvname="app_syx27",ntimes=10000):
        sbmp=None
        shv=None
        for k, v in self.session.apps.items():
            print("Status",k)
            if (k==bmpname):
                sbmp=v[0]
            if (k==hvname):
                shv=v[0]
        if (sbmp==None or shv==None):
            print("Can find "+bmpname+" or "+hvname)
            return
        # Find pressure
        for i in range(ntimes):
            mrbmp=json.loads(sbmp.sendCommand("STATUS", {}))
            x=mrbmp["status"]
            P=x["pressure"]
            T=x["temperature"]+273.15
            v2apply=vwanted*(1.0*P/990.*293./T)
            if (v2apply>vwanted+200):
                print(v2apply,vwanted," too large",P," ",T)
                time.sleep(period)
                continue
            print("applying %d for %d P=%5.1f T=%5.1f " % (int(v2apply),vwanted,P,T))
            m={}
            m["first"]=channel
            m["last"]=channel
            m["value"]=int(v2apply)
            mrhvset = json.loads(shv.sendCommand("VSET",m))
            #print(mrhvset)
            for y in mrhvset["status"]["channels"]:
                if (hvname == 'app_wiener'):
                    sstat=y["status"].split("=")[1]
                    
                    print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
                else:
                    print(mrhvset)
            #mrhv = json.loads(shv.sendCommand("STATUS", {}))
            #print(mrhv)
            time.sleep(period)
            
    def checkHv(self,fname,period=600,ntimes=10000):
        st=json.loads(open(fname).read())
        if ( not 'monitoring' in st):
            print("Missing monitoring section")
            return
        if ( not 'conditions' in st):
            print("Missing conditions section")
            return
        smon=st["monitoring"]
        scond=st["conditions"]
        if ( not 'account' in smon):
            print("Missing account in monitoring section")
            return
        os.environ['MGDBMON']=smon['account']
        _wmon=mgs.instance()
        for i in range(ntimes):
            mrbmp=_wmon.last(smon["bmppath"])

            x=mrbmp["status"]
            P=x["pressure"]
            T=x["temperature"]+273.15
            vcorr=(1.0*P/990.*293.15/T)
            sti=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(mrbmp["ctime"]))
            print("%s ::::  Correction for P %5.1f and T %5.1f is %5.3f " %(sti,P,T,vcorr))
            mhv=_wmon.last(smon["hvpath"])
            if (not 'HV' in scond):
                return
            shv=scond['HV']
            if (not 'channels' in shv):
                return
            for x in shv['channels']:
                ch=x["id"]
                vreq=x["hv"]
                vset=0
                for y in mhv["status"]["channels"]:
                    if (y["id"]==ch):
                        vset=y["vset"]
                        break
                print(ch,vreq,vset,vcorr*vreq)
                param={}
                param["first"]=ch
                param["last"]=ch
                param["value"]=vcorr*vreq
                rep=sac.executeCMD(shv["host"],shv["port"],shv["path"]+"VSET",param)
                print(rep)
            time.sleep(period)
