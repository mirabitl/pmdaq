
import rc_interface
import rc_services as sac
import time
import MongoJob as mg
import json
import os
import logging
import threading

def pmrTransitionWorker(app,transition,res):
    """!thread pmr Transition worker function"""
    s = json.loads(app.sendTransition(transition, {}))
    res = s
    logging.info('ending')
    return
logging.basicConfig(level=logging.INFO,
                    format='(%(threadName)-10s) %(message)s',
                    )

class rc_control(rc_interface.daqControl):
    def __init__(self, config):
        """! Inherits from a pmdaqControl
        It additionnally adds an acces to the MongoDB handling the runs collection
        @param config The configuration file
        """
        super().__init__(config)
        ##reset time
        self.reset = 0 
        ## Comment for a run
        self.comment = "Not yet set"
        ## Setup name
        self.experiment = "UNKNOWN"
        ## MDCC plugin name
        self.md_name = "lyon_mdcc"
        ## MongoDB MongoJob instance
        self.db = mg.instance()
        ## Current state
        self.state = self.get_stored_state()
        ## List of pmdaq url
        self.pm_hosts=[]
        j_sess=json.loads(open(config).read())
        for x in j_sess["apps"]:
            sh="http://%s:%d" % (x["host"],x["port"])
            if (not sh in self.pm_hosts):
                self.pm_hosts.append(sh)
        ## Meta donnees
        self.metadata=json.loads(open("/opt/pmdaq/pyrc/etc/rc_meta.json").read())
    # daq
    def process_transition(self,transition_name):
        rep={}
        if not transition_name in self.metadata["sequences"].keys():
            print(f"{transition_name} is not in possibles kest {self.metadata["sequences"].keys()}")
            return rep
        app_list=self.metadata["sequences"][transition_name]
        for app_name in app_list:
            if not app_name in self.session.apps:
                continue
            if not app_name in self.metadata["apps"].keys():
                continue
            threaded=self.metadata["apps"][app_name].threaded==1
            fsm_list=self.metadata["apps"][app_name][transition_name]["fsm"]
            cmd_list=self.metadata["apps"][app_name][transition_name]["commands"]
            for t in fsm_list:
                msg=self.build_message(app_name,transition_name)
                if not threaded:
                    for a in self.session.apps[app_name]:
                        s = json.loads(a.sendTransition(t,msg))
                        rep[f"{app_name}_{a.instance}"]=s
                else:
                    lt=list()
                for x in self.session.apps[app_name]:
                    rep[f"{app_name}_{x.instance}"]={}
                    thr = threading.Thread(target=pmrTransitionWorker, args=(x,t,rep[f"{app_name}_{x.instance}"],))
                    thr.start()
                    lt.append(thr)

                logging.info(f'{t} Waiting for worker threads')
                alive=True
                while (alive):
                    nalive=False
                    for thr in lt:
                        nalive=nalive or thr.is_alive()
                        logging.debug("%s %d " % (thr.getName() ,thr.is_alive()))                    
                        thr.join(1)
                    alive=nalive
            for c in cmd_list:
                msg={}
                for a in self.session.apps[app_name]:
                    s = json.loads(a.sendCommand(c,msg))
                    rep[f"{app_name}_{a.instance}_{c}"]=s
                
        self.daq_answer = json.dumps(rep)
        self.storeState()
    
    def build_message(self,app_name,transition_name):
        m={}
        if (app_name == "evb_builder" and transition_name == "START"):
            if (self.experiment == "UNKNOWN"):
                self.experiment = os.getenv("DAQSETUP", "UNKNOWN")

            jnrun = self.db.getRun(self.experiment, self.comment)
            m['run'] = jnrun['run']
        return m 
    # Initialising implementation

    def daq_initialising(self):
       self.process_transition("INITIALISE")
    def daq_configuring(self):
       self.process_transition("CONFIGURE")
    def daq_stopping(self):
       self.process_transition("STOP")
    def daq_destroying(self):
       self.process_transition("DESTROY")
    def daq_starting(self):
       self.process_transition("START")

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
        mr = json.loads(self.processCommand("STATUS",pn,{}))
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

  
    def SourceStatus(self, verbose=False):
        """!
        Print out of data source status
        @param verbose if True only printout False return a JSON string of the sources status
        @return JSON string of the sources status 
        """
        rep = {}
        for k, v in self.session.apps.items():
            if (k != "lyon_shm_data_source"):
                continue
            for s in v:
                c_mr=s.sendCommand("STATUS", {})
                #print(c_mr)
                mr = json.loads(c_mr)
                #print(mr)
                if (mr['STATUS'] != "FAILED"):
                    cc={}
                    cc["detid"]=mr["DETID"]
                    cc["sourceid"]=mr["SOURCEID"]
                    cc["gtc"]=mr["EVENT"]["event"]
                    cc["status"]=mr["EVENT"]["state"]
                    rep["%s_%d_SHM" % (s.host, s.instance)] = [cc]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr
                    
        for k, v in self.session.apps.items():
            if (k != "lyon_febv2"):
                continue
            for s in v:
                c_mr=s.sendCommand("STATUS", {})
                #print(c_mr)
                mr = json.loads(c_mr)
                #print(mr)
                if (mr['STATUS'] != "FAILED"):
                    cc={}
                    cc["detid"]=mr["DETID"]
                    cc["sourceid"]=mr["SOURCEID"]
                    cc["gtc"]=mr["EVENT"]["event"]
                    cc["status"]=mr["EVENT"]["state"]
                    rep["%s_%d_FEB" % (s.host, s.instance)] = [cc]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr

        for k, v in self.session.apps.items():
            if (k != "lyon_febv1"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                if (mr['STATUS'] != "FAILED"):
                    rep["%s_%d_FEB" % (s.host, s.instance)] = mr["TDCSTATUS"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr

        for k, v in self.session.apps.items():
            if (k != "lyon_pmr"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                
                if (mr['STATUS'] != "FAILED"):
                    rep["%s_%s_%d" % (s.host,k, s.instance)
                        ] = mr["DIFLIST"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr

        for k, v in self.session.apps.items():
            if (k != "lyon_liboard"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                print(mr)
                if (mr['STATUS'] != "FAILED"):
                    rep["%s_%s_%d" % (s.host,k, s.instance)
                        ] = mr["DIFLIST"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr

                    #rep["%s_%d" % (s.host, s.infos['instance'])] = r
        for k, v in self.session.apps.items():
            if (k != "lyon_gricv0"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                #print(mr)
                if (mr['STATUS'] != "FAILED"):
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr["GRICSTATUS"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr

                    #rep["%s_%d" % (s.host, s.infos['instance'])] = r
        for k, v in self.session.apps.items():
            if (k != "lyon_gricv1"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                #print(mr)
                if (mr['STATUS'] != "FAILED"):
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr["C3ISTATUS"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr

                    #rep["%s_%d" % (s.host, s.infos['instance'])] = r
        if (not verbose):
            return json.dumps(rep)
        # Verbose Printout
        print("""
        \t \t ******************************    
        \t \t ** Data sources information **
        \t \t ******************************
        """)
        for k, v in rep.items():
            print(k)
            if (v != None):
                for x in v:
                    print("\t \t", x)
    # RESTART
    def restart(self,url=None):
        """!
        Send an EXIT command to all pmdaq daemon 
        @param url If not None send EXIT only to this url
        @warning It is a real restarting of the whole DAQ
        """
        if (url==None):
            for x in self.pm_hosts:
                print(x+"/EXIT will be called")
                sac.executeRequest(x+"/EXIT")
        else:
            print(url+"/EXIT will be called")
            sac.executeRequest(url+"/EXIT")
    # FEBV1 specific

    def set6BDac(self, dac):
        """!
        FEBV1 only SET6BDAC
        @param value DAC6B value for all ASICs
        @return processCommand answer
        """
        param = {}
        param["value"] = dac
        return self.processCommand("SET6BDAC", "lyon_febv1", param)

    def cal6BDac(self, mask, shift):
        """!
        FEBV1 only CAL6BDAC
        @param shift DAC6B shift for all ASICs
        @param mask Channel mask
        @return processCommand answer
        """
        param = {}
        param["shift"] = shift
        param["mask"] = int(mask, 16)
        return self.processCommand("CAL6BDAC", "lyon_febv1", param)

    def setVthTime(self, Threshold):
        """!
        FEBV1 only SETVTHTIME
        @param value DAC10B VTH
        @return processCommand answer
        """
        param = {}
        param["value"] = Threshold
        return self.processCommand("SETVTHTIME", "lyon_febv1", param)

    def setTdcMode(self, mode):
        """!
        FEBV1 only SETMODE
        @param value 0/1 TDC mode
        @return processCommand answer
        """
        param = {}
        param["value"] = mode
        return self.processCommand("SETMODE", "lyon_febv1", param)

    def setTdcDelays(self, active, dead):
        """!
        FEBV1 only SETDELAY/DURATION
        @param active Dealy active value
        @param dead Delay length  value
        @return dictionnary
        """
        param = {}
        param["value"] = active
        r = {}
        r["active"] = json.loads(self.processCommand(
            "SETDELAY", "lyon_febv1", param))
        param["value"] = active
        r = {}
        r["dead"] = json.loads(self.processCommand(
            "SETDURATION", "lyon_febv1", param))
    def setTdcMask(self, channelmask, asicmask):
        """!
        FEBV1 only SETMASK
        @param channelmask 32 bits mask
        @param asicmask 1/2/3 ASIC mask
        @return processCommand answer
        """
        param = {}
        param["value"] = channelmask
        param["asic"] = asicmask
        return self.processCommand("SETMASK", "lyon_febv1", param)

    def tdcLUTCalib(self, instance, channel):
        """!
        FEBV1 only LUT calibration
        @param channel TDC channel to scan
        @param instance FEBV1Manager Instance
        @return dictionnary answer
        """
        if (not "lyon_febv1" in self.session.apps):
            return '{"answer":"NOlyon_febv1","status":"FAILED"}'
        if (len(self.session.apps["lyon_febv1"]) <= instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = self.session.apps["lyon_febv1"][instance]
        n = (1 << channel)
        param = {}
        param["value"] = "%x" % n
        param["value"] = channel
        r = {}
        r["cal_mask"] = json.loads(tdc.sendCommand("CALIBMASK", param))
        r["cal_status"] = json.loads(tdc.sendCommand("CALIBSTATUS", param))
        return json.dumps(r)

    def tdcLUTDump(self, instance, channel):
        """!
        FEBV1 only Dump Look up Table
        @param channel TDC channel to scan
        @param instance FEBV1Manager Instance
        @return dictionnary answer
        """
        if (not "lyon_febv1" in self.session.apps):
            return '{"answer":"NOlyon_febv1","status":"FAILED"}'
        if (len(self.session.apps["lyon_febv1"]) <= instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = self.session.apps["lyon_febv1"][instance]
        param = {}
        param["value"] = channel
        r = {}
        r["lut_%d" % channel] = json.loads(tdc.sendCommand("GETLUT", param))
        return json.dumps(r)

    def tdcLUTMask(self, instance, mask, feb):
        """!
        FEBV1 only SET6BDSet LUT Mask
        @param instance FEBV1Manager Instance
        @param TDC channel mask
        @param feb Feb Id
        @return dictionnary answer
        """
        if (not "lyon_febv1" in self.session.apps):
            return '{"answer":"NOlyon_febv1","status":"FAILED"}'
        if (len(self.session.apps["lyon_febv1"]) <= instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = self.session.apps["lyon_febv1"][instance]
        param = {}
        param["value"] = mask
        param["feb"] = feb
        r = {}
        r["test_mask"] = json.loads(tdc.sendCommand("TESTMASK", param))
        r["cal_status"] = json.loads(tdc.sendCommand("CALIBSTATUS", param))
        return json.dumps(r)

