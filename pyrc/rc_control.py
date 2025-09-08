
import rc_interface
import rc_services as sac
import time
import MongoJob as mg
import json
import os
import logging
import threading
from tabulate import tabulate
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
        self.metadata=json.loads(open("/usr/local/pmdqa/etc/rc_meta.json").read())
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
    def set_parameters(self):
        rep={}
        if (self.daq_params_file == "UNKNOWN"):
                self.daq_params_file = os.getenv("DAQ_PARAMS_FILE", "UNKNOWN")
        if (self.daq_params_set == "UNKNOWN:UNKNOWN"):
                self.daq_params_set = os.getenv("DAQ_PARAMS_SET", "UNKNOWN:UNKNOWN")
        if (self.daq_params_file == "UNKNOWN" or self.daq_params_set == "UNKNOWN:UNKNOWN"):
            return rep
        j_params=json.loads(open(self.daq_params_file).read())
        pset=self.daq_params_set.split(":")
        if (not pset[0] in j_params["setup"].keys()):
            print(f"Missing experiment {pset[0]} in file {self.daq_params_file} ({j_params["setup"].keys()})")
            return rep
        if (not pset[1] in j_params["setup"][pset[0]].keys()):
            print(f"Missing parameters set {pset[1]} in {pset[0]} experiment in the file {self.daq_params_file} ({j_params["setup"].keys()})")
            return rep
        p_apps=j_params["setup"][pset[0]][pset[1]]["apps"]
        for x in p_apps:
            if not x["name"] in self.session.apps.keys():
                continue
            for a in self.session.apps[x["name"]]:
                    s = json.loads(a.sendCommand("SETPARAMS",x["params"]))
                    rep[f"{x["name"]}_{a.instance}"]=s
        return rep
        

    # Initialising implementation

    def daq_initialising(self):
       self.process_transition("INITIALISE")
    def daq_configuring(self):
       self.set_parameters()
       self.process_transition("CONFIGURE")
    def daq_stopping(self):
       self.process_transition("STOP")
    def daq_destroying(self):
       self.process_transition("DESTROY")
    def daq_starting(self):
       self.process_transition("START")

    def TriggerCommand(self,command,param={}):
        """! Send command to trigger board (mdcc,mbmdcc, ipdc,liboard) status
        """
        pn=None
        tboards=["lyon_mdcc","lyon_mbmdcc","lyon_ipdc","lyon_liboard"]
        for b in tboards:
            if b in self.session.apps:
                pn=b
                break
        if (pn==None):
            print("""
            \t \t ****************************
            \t \t ** No Trigger information **
            \t \t ****************************
            """)
            return
        mr = json.loads(self.processCommand(command,pn,param))
        
        return json.dumps(mr)

    def TriggerStatus(self,verbose=False):
        """! Print out of trigger board (mdcc,mbmdcc, ipdc,liboard) status
        @param verbose If true print out results otherwise return string with json content of the printout
        @return string with json content of the printout (verbose=False)
        """
        pn=None
        tboards=["lyon_mdcc","lyon_mbmdcc","lyon_ipdc","lyon_liboard"]
        for b in tboards:
            if b in self.session.apps:
                pn=b
                break
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
    def DataSourceStatus(self, verbose=False):
        """!
        Print out of data source status
        @param verbose if True only printout False return a JSON string of the sources status
        @return JSON string of the sources status
        """
        dslist=["lyon_shm_data_source","lyon_febv1","lyon_febv2","lyon_gricv0","lyon_gricv1","lyon_liboard","lyon_pmr"]
        rep = {}
        
        for k, v in self.session.apps.items():
            if (not k in dslist):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("DSLIST", {}))

                if (mr['STATUS'] != "FAILED"):
                    rep["%s_%s_%d" % (s.host,k, s.instance)
                        ] = mr["DSLIST"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.instance)] = mr


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
                print(tabulate(v,headers="keys",tablefmt="simple"))
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
    