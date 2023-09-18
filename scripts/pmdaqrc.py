from __future__ import absolute_import
from __future__ import print_function
import json
import daqrc
import time


class pmdaqControl(daqrc.daqControl):


    def BuilderStatus(self, verbose=False):
        if (not "evb_builder" in self.session.apps):
            print("No Event builder found in thi session ",self.session.name())
        rep = {}
        
        for s in self.session.apps["evb_builder"]:
            r = {}
            r['run'] = -1
            r['event'] = -1
            r['url'] = s.host
            mr = json.loads(s.sendCommand("STATUS",{}))
            #print("Event Builder",mr)
            if (mr['status'] != "FAILED"):
                r["run"] = mr["answer"]["run"]
                r["event"] = mr["answer"]["event"]
                r["builder"] = mr["answer"]["difs"]
                r["built"] = mr["answer"]["build"]
                r["total"] = mr["answer"]["total"]
                r["compressed"] = mr["answer"]["compressed"]
                r["time"] = time.time()
                rep["%s%s" % (s.host, s.path)] = r
            else:
                rep["%s%s" % (s.host, s.path)] = mr
        if (not verbose):
            return json.dumps(rep)
        print("""
        \t \t *************************    
        \t \t ** Builder information **
        \t \t *************************
        """)
        #rep = json.loads(sr)
        for k, v in rep.items():
            print(k)
            for xk, xv in v.items():
                    if (xk != "builder"):
                        print("\t", xk, xv)
                    else:
                        if (xv != None):
                            for y in xv:
                                print("\t \t ID %x => %d " % (int(y['id'].split('-')[2]), y['received']))

    def TriggerStatus(self,verbose=False):
        pn=None
        if ("lyon_mdcc" in self.session.apps): 
            pn="lyon_mdcc"
        if ("lyon_mbmdcc" in self.session.apps): 
            pn="lyon_mbmdcc"
        if ("lyon_ipdc" in self.session.apps): 
            pn="lyon_ipdc"
        if ("lyon_liboard" in self.session.apps): 
            pn="lyon_liboard"
        if (pn==None):
            print("""
            \t \t ****************************    
            \t \t ** No Trigger information **
            \t \t ****************************
            """)
            return
        mr = json.loads(self.mdcc_Status())
        #print(mr)
        #print("ON DEBUG ",mr)
        #print("ON DEBUG ",mr)
        if (not verbose):
            return json.dumps(mr)
        else:
            print("""
            \t \t *************************    
            \t \t ** Trigger information **
            \t \t *************************
            """)
            for tk,tv in mr.items():
                print("\t ",tk)
                if ("COUNTERS" in tv):
                    for k,v in tv["COUNTERS"].items():
                        if (k =="version"):
                            print("\t \t %s %x" % (k,v))
                        else:
                            print("\t \t ",k,v)
                        
    # Builder

    def builder_setHeader(self, rtype, rval, mask):
        l = []
        l.append(rtype)
        l.append(rval)
        l.append(mask)
        param = {}
        param["header"] = l
        return self.processCommand("SETHEADER", "evb_builder", param)
    # MDCC
    def mdcc_command(self,cmd,par):
        self.pn=None
        if ("lyon_mbmdcc" in self.session.apps):
            self.pn="lyon_mbmdcc"
        if ("lyon_mdcc" in self.session.apps):
            self.pn="lyon_mdcc"
        if ("lyon_ipdc" in self.session.apps):
            self.pn="lyon_ipdc"
        if ("lyon_liboard" in self.session.apps):
            self.pn="lyon_liboard"
            if (cmd=="STATUS"):
                cmd="MDCCSTATUS"
        if (self.pn==None):
            print("===> No trigger control")
            return {}
        return self.processCommand(cmd,  self.pn,par)      
    def mdcc_Status(self):
        return self.mdcc_command("STATUS",{})
        
            

    def mdcc_Pause(self):
       return self.mdcc_command("PAUSE",  {})

    def mdcc_Resume(self):
        return self.mdcc_command("RESUME",  {})

    def mdcc_EcalPause(self):
        return self.mdcc_command("ECALPAUSE",  {})

    def mdcc_EcalResume(self):
        return self.mdcc_command("ECALRESUME",  {})

    def mdcc_CalibOn(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("CALIBON",  param)

    def mdcc_CalibOff(self):
        return self.mdcc_command("CALIBOFF",  {})

    def mdcc_ReloadCalibCount(self):
        return self.mdcc_command("RELOADCALIB",  {})

    def mdcc_setCalibCount(self, value):
        param = {}
        param["nclock"] = value
        return self.mdcc_command("SETCALIBCOUNT",  param)

    def mdcc_Reset(self):
        return self.mdcc_command("RESET",  {})

    def mdcc_ReadRegister(self, address):
        param = {}
        param["address"] = address
        return self.mdcc_command("READREG",  param)

    def mdcc_WriteRegister(self, address, value):
        param = {}
        param["address"] = address
        param["value"] = value
        return self.mdcc_command("READREG",  param)

    def mdcc_setSpillOn(self, value):
        param = {}
        param["nclock"] = value
        return self.mdcc_command("SPILLON",  param)

    def mdcc_setSpillOff(self, value):
        param = {}
        param["nclock"] = value
        return self.mdcc_command("SPILLOFF",  param)

    def mdcc_setResetTdcBit(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("RESETTDC",  param)

    def mdcc_resetTdc(self,bar=True):
        if (bar):
            self.mdcc_setResetTdcBit(0)
            return self.mdcc_setResetTdcBit(0XFFF)
        else:
            self.mdcc_setResetTdcBit(1)
            return self.mdcc_setResetTdcBit(0)
            

    def mdcc_setBeamOn(self, value):
        param = {}
        param["nclock"] = value
        return self.mdcc_command("BEAMON",  param)
    
    def mdcc_setChannelOn(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("CHANNELON",  param)

    def mdcc_setHardReset(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("SETHARDRESET",  param)

    def mdcc_setSpillRegister(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("SETSPILLREGISTER",  param)

    def mdcc_setExternal(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("SETEXTERNAL",  param)

    def mdcc_setCalibRegister(self, value):
        param = {}
        param["value"] = value
        return self.mdcc_command("SETCALIBREGISTER",  param)

    def mdcc_setTriggerDelays(self, delay, busy):
        param = {}
        param["delay"] = delay
        param["busy"] = busy
        return self.mdcc_command("SETTRIGEXT",  param)
