from __future__ import absolute_import
from __future__ import print_function
import json
import daqrc
import time


class pmdaqControl(daqrc.daqControl):
    """!
    Intermediate daqControl class handling specifity for event builder and trigger card
    """

    def BuilderStatus(self, verbose=False,mqtt=True):
        """! Print out of event builder status
        @param verbose If true print out results otherwise return string with json content of the printout
        @param mqtt if true trigger the publication of evb_builder status to mqtt (if configured to)
        @return string with json content of the printout (verbose=False)
        """
        if (not "evb_builder" in self.session.apps):
            print("No Event builder found in thi session ",self.session.name())
        rep = {}
        
        for s in self.session.apps["evb_builder"]:
            r = {}
            r['run'] = -1
            r['event'] = -1
            r['url'] = s.host
            par={"mqtt":0}
            if (mqtt):
                par={"mqtt":1}

            mr = json.loads(s.sendCommand("STATUS",par))
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
        """! Print out of trigger board (mdcc,mbmdcc, ipdc,liboard) status
        @param verbose If true print out results otherwise return string with json content of the printout
        @return string with json content of the printout (verbose=False)
        """
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
        """! Send a command SETHEADER to the evb_builder with parameters
        @param rtype Run Type
        @param rval Value of calibration parameter
        @param mask Bit mask for the value
        @return processCommand answer
        """
        l = []
        l.append(rtype)
        l.append(rval)
        l.append(mask)
        param = {}
        param["header"] = l
        return self.processCommand("SETHEADER", "evb_builder", param)
    # MDCC
    def mdcc_command(self,cmd,par):
        """! generic send command for any MDCC-like board 
        It finds in the session the trigger plugin (mdcc,mbmdcc, ipdc,liboard) and send the command
        @param cmd command name
        @param par CGI parameters
        @return processCommand answer
        """
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
        """! Generic mdcc-like STATUS command
        @return processCommand answer
        """
        return self.mdcc_command("STATUS",{})
        
            

    def mdcc_Pause(self):
        """! Generic mdcc-like PAUSE command
        @return processCommand answer
        """
        return self.mdcc_command("PAUSE",  {})

    def mdcc_Resume(self):
        """! Generic mdcc-like RESUME command
        @return processCommand answer
        """
        return self.mdcc_command("RESUME",  {})

    def mdcc_EcalPause(self):
        """! Generic mdcc-like ECALPAUSE command
        @return processCommand answer
        @deprecated
        """
        return self.mdcc_command("ECALPAUSE",  {})

    def mdcc_EcalResume(self):
        """! Generic mdcc-like ECALRESUME command
        @return processCommand answer
        @deprecated
        """
        return self.mdcc_command("ECALRESUME",  {})

    def mdcc_CalibOn(self, value):
        """! Generic mdcc-like CALIBON command
        @param value Bit 1 ON , Bit 2 reload count
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("CALIBON",  param)

    def mdcc_CalibOff(self):
        """! Generic mdcc-like CALIBOFF command
        @return processCommand answer
        """
        return self.mdcc_command("CALIBOFF",  {})

    def mdcc_ReloadCalibCount(self):
        """! Generic mdcc-like RELOADCALIB command
        @return processCommand answer
        """
        return self.mdcc_command("RELOADCALIB",  {})

    def mdcc_setCalibCount(self, value):
        """! Generic mdcc-like SETCALIBCOUNT command
        @param nclock Number of calibration windows
        @return processCommand answer
        """
        param = {}
        param["nclock"] = value
        return self.mdcc_command("SETCALIBCOUNT",  param)

    def mdcc_Reset(self):
        """! Generic mdcc-like RESET command
        @return processCommand answer
        """
        return self.mdcc_command("RESET",  {})

    def mdcc_ReadRegister(self, address):
        """! Generic mdcc-like READREG command
        @param  address register address
        @return processCommand answer
        """
        param = {}
        param["address"] = address
        return self.mdcc_command("READREG",  param)

    def mdcc_WriteRegister(self, address, value):
        """! Generic mdcc-like WRITEREG command
        @param  address register address
        @param  value register value
        @return processCommand answer
        """
        param = {}
        param["address"] = address
        param["value"] = value
        return self.mdcc_command("WRITEREG",  param)

    def mdcc_setSpillOn(self, value):
        """! Generic mdcc-like SPILLON command
        @param  nclock length of SPILLON
        @return processCommand answer
        """
        param = {}
        param["nclock"] = value
        return self.mdcc_command("SPILLON",  param)

    def mdcc_setSpillOff(self, value):
        """! Generic mdcc-like SPILLOFF command
        @param  nclock length of SPILLOFF
        @return processCommand answer
        """
        param = {}
        param["nclock"] = value
        return self.mdcc_command("SPILLOFF",  param)

    def mdcc_setResetTdcBit(self, value):
        """! Generic mdcc-like RESETTDC command
        @param value 0 or 1
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("RESETTDC",  param)

    def mdcc_resetTdc(self,bar=True):
        """! Generic mdcc-like FEBV1 RESETTDC command
        @param bar if True 0 then 1 otherwise 1 then 0
        @return processCommand answer
        """
        if (bar):
            self.mdcc_setResetTdcBit(0)
            return self.mdcc_setResetTdcBit(0XFFF)
        else:
            self.mdcc_setResetTdcBit(1)
            return self.mdcc_setResetTdcBit(0)
            

    def mdcc_setBeamOn(self, value):
        """! Generic mdcc-like BEAMON command
        @param value Beam on bit
        @deprecated
        @return processCommand answer
        """
        param = {}
        param["nclock"] = value
        return self.mdcc_command("BEAMON",  param)
    
    def mdcc_setChannelOn(self, value):
        """! Generic mdcc-like CHANNELON command
        @param value bit pattern of channels
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("CHANNELON",  param)

    def mdcc_setHardReset(self, value):
        """! Generic mdcc-like SETHARDRESET command
        @param value 0 or 1
        @deprecated Identical to ResetTdcBit
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("SETHARDRESET",  param)

    def mdcc_setSpillRegister(self, value):
        """! Generic mdcc-like SETSPILLREGISTER command
        @param value Bit pattern of running mode
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("SETSPILLREGISTER",  param)

    def mdcc_setExternal(self, value):
        """! Generic mdcc-like SETEXTERNAL command
        @param value 0/1 Enable/disbale busy on trigger
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("SETEXTERNAL",  param)

    def mdcc_setCalibRegister(self, value):
        """! Generic mdcc-like SETCALIBREGISTER command
        @param value Calibration register value 
        @return processCommand answer
        """
        param = {}
        param["value"] = value
        return self.mdcc_command("SETCALIBREGISTER",  param)

    def mdcc_setTriggerDelays(self, delay, busy):
        """! Generic mdcc-like SETTRIGEXT command
        @param delay Delay of external trigger
        @param busy Length of external trigger
        @return processCommand answer
        """
        param = {}
        param["delay"] = delay
        param["busy"] = busy
        return self.mdcc_command("SETTRIGEXT",  param)
