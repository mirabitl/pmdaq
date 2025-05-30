from __future__ import absolute_import
from __future__ import print_function
import pmdaqrc
import serviceAccess as sac
import time
import MongoJob as mg
import json
import os
import math
from six.moves import range
import logging
import threading

def pmrTransitionWorker(app,transition,res):
    """!thread pmr Transition worker function"""
    t = threading.currentThread()
    s = json.loads(app.sendTransition(transition, {}))
    res = s
    logging.info('ending')
    return
logging.basicConfig(level=logging.INFO,
                    format='(%(threadName)-10s) %(message)s',
                    )

class combRC(pmdaqrc.pmdaqControl):
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
        self.location = "UNKNOWN"
        ## MDCC plugin name
        self.md_name = "lyon_mdcc"
        ## MongoDB MongoJob instance
        self.db = mg.instance()
        ## Current state
        self.state = self.getStoredState()
        ## List of pmdaq url
        self.pm_hosts=[]
        j_sess=json.loads(open(config).read())
        for x in j_sess["apps"]:
            sh="http://%s:%d" % (x["host"],x["port"])
            if (not sh in self.pm_hosts):
                self.pm_hosts.append(sh)
    # daq
    # Initialising implementation

    def daq_initialising(self):
        """! 
        Initialisation for all boards

        @verbatim
        In this order if found in the configuration
        -------------------------------------------    
        GPIO =  CONFIGURE VMEON VMEOFF VMEONN
        SDCC= OPEN INITIALISE CONFIGURE STOP CCCRESET DIFRESET
        MDCC/IPDC/MBMDCC =INITIALISE
        EVB_BUILDER =CONFIGURE
        FEBV1 = RESETTDC INITIALISE
        FEBV2 = CREATEFEB INITIALISE
        SHM_DATA_SOURCE = CREATEBOARD INITIALISE
        PMR = Threaded: SCAN INITIALISE
        LIBOARD = SCAN INITIALISE
        GRICV0/GRICV1 =  INITIALISE
        DIF = SCAN INITIALISE
        @endverbatim
        """
        m = {}
        r = {}
        # old DIF Fw
        if ("lyon_gpio" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_gpio'][0].sendTransition("CONFIGURE", m))
            r["lyon_gpio"] = s
            json.loads(self.session.apps['lyon_gpio']
                       [0].sendCommand("VMEON", {}))
            json.loads(self.session.apps['lyon_gpio']
                       [0].sendCommand("VMEOFF", {}))
            json.loads(self.session.apps['lyon_gpio']
                       [0].sendCommand("VMEON", {}))
            time.sleep(5)
        if ("lyon_sdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_sdcc'][0].sendTransition("OPEN", m))
            s = json.loads(
                self.session.apps['lyon_sdcc'][0].sendTransition("INITIALISE", m))
            s = json.loads(
                self.session.apps['lyon_sdcc'][0].sendTransition("CONFIGURE", m))
            json.loads(self.session.apps['lyon_sdcc']
                       [0].sendTransition("STOP", m))
            time.sleep(1.)
            json.loads(self.session.apps['lyon_sdcc']
                       [0].sendCommand("CCCRESET", {}))
            json.loads(self.session.apps['lyon_sdcc']
                       [0].sendCommand("DIFRESET", {}))
            r["lyon_sdcc"] = s
        # Mdcc
        if ("lyon_mdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_mdcc'][0].sendTransition("INITIALISE", m))
            r["lyon_mdcc"] = s
        if ("lyon_ipdc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_ipdc'][0].sendTransition("INITIALISE", m))
            r["lyon_ipdc"] = s
            self.md_name = "lyon_ipdc"
        if ("lyon_mbmdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_mbmdcc'][0].sendTransition("INITIALISE", m))
            r["lyon_mbmdcc"] = s
            self.md_name = "lyon_mbmdcc"
        # Mbdaq0
        if ("lyon_mbdaq0" in self.session.apps):
            for x in self.session.apps["lyon_mbdaq0"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_mbdaq0_%d" % x.instance] = s

        # evb_builder
        for x in self.session.apps["evb_builder"]:
            s = json.loads(x.sendTransition("CONFIGURE", m))
            r["evb_builder_%d" % x.instance] = s
        # Reset for FEB V1
        if (self.reset != 0):
            self.mdcc_resetTdc((self.reset > 0),)
            time.sleep(abs(self.reset)/1000.)

        if ("lyon_febv1" in self.session.apps):
            for x in self.session.apps["lyon_febv1"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_febv1_%d" % x.instance] = s

        if ("lyon_febv2" in self.session.apps):
            print("ON A VU DES FEBV2")
            for x in self.session.apps["lyon_febv2"]:
                print("FEBV2 CREATEFEB")
                s1=x.sendTransition("CREATEFEB", m)
                print(s1)
                
                #s = json.loads(x.sendTransition("CREATEFEB", m))
                r["lyon_febv2_%d" % x.instance] = s
                print(s)

            for x in self.session.apps["lyon_febv2"]:
                print("FEBV2 INITIALISE")
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_febv2_%d" % x.instance] = s
                print(s)
                
        if ("lyon_shm_data_source" in self.session.apps):
            print("ON A VU DES SHM_DATA_SOURCE")
            for x in self.session.apps["lyon_shm_data_source"]:
                print("SHMDS CREATEBOARD")
                s1=x.sendTransition("CREATEBOARD", m)
                print(s1)
                
                #s = json.loads(x.sendTransition("CREATEFEB", m))
                r["lyon_shm_data_source_%d" % x.instance] = s
                print(s)

            for x in self.session.apps["lyon_shm_data_source"]:
                print("SHMDS INITIALISE")
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_shm_data_source_%d" % x.instance] = s
                print(s)
                
        if ("lyon_pmr" in self.session.apps):
            #for x in self.session.apps["lyon_pmr"]:
            #    s = json.loads(x.sendTransition("SCAN", m))
            #    r["lyon_pmr_%d" % x.instance] = s
            lt=list()
            for x in self.session.apps["lyon_pmr"]:
                r["lyon_pmr_%d" % x.instance]={}
                t = threading.Thread(target=pmrTransitionWorker, args=(x,"SCAN",r["lyon_pmr_%d" % x.instance],))
                #t.setDaemon(True)
                t.start()
                lt.append(t)

            logging.info('SCAN Waiting for worker threads')
            alive=True
            while (alive):
                nalive=False
                for t in lt:
                    nalive=nalive or t.is_alive()
                    logging.debug("%s %d " % (t.getName() ,t.is_alive()))                    
                    t.join(1)
                alive=nalive
                
            #for x in self.session.apps["lyon_pmr"]:
                #s = json.loads(x.sendTransition("INITIALISE", m))
                #r["lyon_pmr_%d" % x.instance] = s
            lt=list()
            for x in self.session.apps["lyon_pmr"]:
                r["lyon_pmr_%d" % x.instance]={}
                t = threading.Thread(target=pmrTransitionWorker, args=(x,"INITIALISE",r["lyon_pmr_%d" % x.instance],))
                #t.setDaemon(True)
                t.start()
                lt.append(t)

            logging.info('INITIALISE Waiting for worker threads')
            alive=True
            while (alive):
                nalive=False
                for t in lt:
                    nalive=nalive or t.is_alive()
                    logging.debug("%s %d " % (t.getName() ,t.is_alive()))                    
                    t.join(1)
                alive=nalive

        if ("lyon_liboard" in self.session.apps):
            for x in self.session.apps["lyon_liboard"]:
                s = json.loads(x.sendTransition("SCAN", m))
                r["lyon_liboard_%d" % x.instance] = s

            for x in self.session.apps["lyon_liboard"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_liboard_%d" % x.instance] = s

        if ("lyon_gricv0" in self.session.apps):
            for x in self.session.apps["lyon_gricv0"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_gricv0_%d" % x.instance] = s

        if ("lyon_gricv1" in self.session.apps):
            for x in self.session.apps["lyon_gricv1"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_gricv1_%d" % x.instance] = s
        # Old DIF Fw
        if ("lyon_dif" in self.session.apps):
            for x in self.session.apps["lyon_dif"]:
                s = json.loads(x.sendTransition("SCAN", m))
                r["lyon_dif_SCAN_%d" % x.instance] = s

            for x in self.session.apps["lyon_dif"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["lyon_dif_INIT_%d" % x.instance] = s

        self.daq_answer = json.dumps(r)
        self.storeState()

    def daq_configuring(self):
        """! 
        Configuration for all boards

        @verbatim
        In this order if found in the configuration
        -------------------------------------------    
        FEBV1 = CONFIGURE
        SHM_DATA_SOURCE = CONFIGURE
        FEBV2 = CONFIGURE
        PMR = Threaded: CONFIGURE
        LIBOARD/GRICV0/GRICV1 =  CONFIGURE
        SDCC=  CCCRESET DIFRESET
        DIF = CONFIGURE
        @endverbatim
        """
        m = {}
        r = {}
        if ("lyon_febv1" in self.session.apps):
            for x in self.session.apps["lyon_febv1"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["lyon_febv1_%d" % x.instance] = s
        if ("lyon_shm_data_source" in self.session.apps):
            for x in self.session.apps["lyon_shm_data_source"]:
                rr=x.sendTransition("CONFIGURE", m)
                print(rr)
                #s = json.loads(x.sendTransition("CONFIGURE", m))
                #r["lyon_shm_data_source_%d" % x.instance] = s
        if ("lyon_febv2" in self.session.apps):
            for x in self.session.apps["lyon_febv2"]:
                rr=x.sendTransition("CONFIGURE", m)
                print(rr)
                #s = json.loads(x.sendTransition("CONFIGURE", m))
                #r["lyon_febv2_%d" % x.instance] = s
        if ("lyon_pmr" in self.session.apps):
            #for x in self.session.apps["lyon_pmr"]:
            #    s = json.loads(x.sendTransition("CONFIGURE", m))
            #    r["lyon_pmr_%d" % x.instance] = s
            lt=list()
            for x in self.session.apps["lyon_pmr"]:
                r["lyon_pmr_%d" % x.instance]={}
                t = threading.Thread(target=pmrTransitionWorker, args=(x,"CONFIGURE",r["lyon_pmr_%d" % x.instance],))
                #t.setDaemon(True)
                t.start()
                lt.append(t)

            logging.info('CONFIGURE Waiting for worker threads')
            alive=True
            while (alive):
                nalive=False
                for t in lt:
                    nalive=nalive or t.is_alive()
                    logging.debug("%s %d " % (t.getName() ,t.is_alive()))                    
                    t.join(1)
                alive=nalive

        if ("lyon_liboard" in self.session.apps):
            for x in self.session.apps["lyon_liboard"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["lyon_liboard_%d" % x.instance] = s
        if ("lyon_gricv0" in self.session.apps):
            for x in self.session.apps["lyon_gricv0"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["lyon_gricv0_%d" % x.instance] = s
        if ("lyon_gricv1" in self.session.apps):
            for x in self.session.apps["lyon_gricv1"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["lyon_gricv1_%d" % x.instance] = s
        # Old DIF Firmware
        if ("lyon_sdcc" in self.session.apps):
            json.loads(self.session.apps['lyon_sdcc']
                       [0].sendCommand("CCCRESET", {}))
            s = json.loads(
                self.session.apps['lyon_sdcc'][0].sendCommand("DIFRESET", {}))
            r["lyon_sdcc"] = s
        if ("lyon_dif" in self.session.apps):
            for x in self.session.apps["lyon_dif"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["lyon_dif_%d" % x.instance] = s
        self.daq_answer = json.dumps(r)
        self.storeState()

    def daq_stopping(self):
        """! 
        Stopping run and boards

        @verbatim
        In this order if found in the configuration
        -------------------------------------------    
        FEBV1 = STOP
        SHM_DATA_SOURCE = STOP
        FEBV2 = STOP
        PMR =  STOP
        LIBOARD/GRICV0/GRICV1 =  STOP
        SDCC =  STOP
        DIF = STOP
        EVB_BUILDER = STOP 
        @endverbatim
        """
        m = {}
        r = {}

        self.mdcc_Pause()

        if ("lyon_febv1" in self.session.apps):
            for x in self.session.apps["lyon_febv1"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_febv1_%d" % x.instance] = s
        if ("lyon_shm_data_source" in self.session.apps):
            for x in self.session.apps["lyon_shm_data_source"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_shm_data_source_%d" % x.instance] = s

        if ("lyon_febv2" in self.session.apps):
            for x in self.session.apps["lyon_febv2"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_febv2_%d" % x.instance] = s

        if ("lyon_pmr" in self.session.apps):
            for x in self.session.apps["lyon_pmr"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_pmr_%d" % x.instance] = s

        if ("lyon_liboard" in self.session.apps):
            for x in self.session.apps["lyon_liboard"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_liboard_%d" % x.instance] = s
        if ("lyon_gricv0" in self.session.apps):
            for x in self.session.apps["lyon_gricv0"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_gricv0_%d" % x.instance] = s
        if ("lyon_gricv1" in self.session.apps):
            for x in self.session.apps["lyon_gricv1"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_gricv1_%d" % x.instance] = s
        # Old DIF fw
        if ("lyon_sdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_sdcc'][0].sendTransition("STOP", m))
            r["lyon_sdcc"] = s
        if ("lyon_dif" in self.session.apps):
            for x in self.session.apps["lyon_dif"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_dif_%d" % x.instance] = s

        for x in self.session.apps["evb_builder"]:
            s = json.loads(x.sendTransition("STOP", m))
            r["evb_builder_%d" % x.instance] = s

        self.daq_answer = json.dumps(r)
        self.storeState()

    def daq_destroying(self):
        """! 
       destroying transition 

        @verbatim
        In this order if found in the configuration
        -------------------------------------------    
        FEBV1 = DESTROY
        SHM_DATA_SOURCE = DESTROY
        FEBV2 = DESTROY
        PMR =  DESTROY
        LIBOARD/GRICV0/GRICV1/DIF =  DESTROY
        MBMDCC =  DESTROY
        @endverbatim
        """
        m = {}
        r = {}
        if ("lyon_febv1" in self.session.apps):
            for x in self.session.apps["lyon_febv1"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_febv1_%d" % x.instance] = s
        if ("lyon_shm_data_source" in self.session.apps):
            for x in self.session.apps["lyon_shm_data_source"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_shm_data_source_%d" % x.instance] = s
        if ("lyon_febv2" in self.session.apps):
            for x in self.session.apps["lyon_febv2"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_febv2_%d" % x.instance] = s
        if ("lyon_pmr" in self.session.apps):
            for x in self.session.apps["lyon_pmr"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_pmr_%d" % x.instance] = s
        if ("lyon_liboard" in self.session.apps):
            for x in self.session.apps["lyon_liboard"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_liboard_%d" % x.instance] = s
        if ("lyon_gricv0" in self.session.apps):
            for x in self.session.apps["lyon_gricv0"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_gricv0_%d" % x.instance] = s
        if ("lyon_gricv1" in self.session.apps):
            for x in self.session.apps["lyon_gricv1"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_gricv1_%d" % x.instance] = s
        # old DIF Fw
        if ("lyon_dif" in self.session.apps):
            for x in self.session.apps["lyon_dif"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_dif_%d" % x.instance] = s

        if ("lyon_mbmdcc" in self.session.apps):
            for x in self.session.apps["lyon_mbmdcc"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["lyon_mbmdcc_%d" % x.instance] = s

        self.daq_answer = json.dumps(r)
        self.storeState()

    def daq_starting(self):
        """! 
        starting transition 

        It first connect to MongoDB and get a run number for the session location
        defined in $DAQSETUP


        @verbatim
        In this order if found in the configuration
        -------------------------------------------    
        EVB_BUIDER = START with 'run' parameter
        FEBV1 = START
        SHM_DATA_SOURCE = START
        FEBV2 = START
        PMR =  START
        LIBOARD/GRICV0/GRICV1/DIF =  START
        MDCC = RESET (counters) ECALRESUME
        IPDC = INITIALISE
        MBMDCC =  RESET (counters) 
        DIF = START
        SDCC = START
        @endverbatim
        """
        if (self.location == "UNKNOWN"):
            self.location = os.getenv("DAQSETUP", "UNKNOWN")

        jnrun = self.db.getRun(self.location, self.comment)
        r = {}
        m = {}
        # print "EVENT evb_builder",jnrun['run']
        m['run'] = jnrun['run']
        for x in self.session.apps["evb_builder"]:
            print("Sending Start to vent evb_builder")
            s = json.loads(x.sendTransition("START", m))
            r["evb_builder_%d" % x.instance] = s

        m = {}
        if ("lyon_febv1" in self.session.apps):
            for x in self.session.apps["lyon_febv1"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_febv1_%d" % x.instance] = s
        if ("lyon_shm_data_source" in self.session.apps):
            for x in self.session.apps["lyon_shm_data_source"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_shm_data_source_%d" % x.instance] = s
        if ("lyon_febv2" in self.session.apps):
            for x in self.session.apps["lyon_febv2"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_febv2_%d" % x.instance] = s
        if ("lyon_pmr" in self.session.apps):
            for x in self.session.apps["lyon_pmr"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_pmr_%d" % x.instance] = s
        if ("lyon_liboard" in self.session.apps):
            for x in self.session.apps["lyon_liboard"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_liboard_%d" % x.instance] = s
        if ("lyon_gricv0" in self.session.apps):
            for x in self.session.apps["lyon_gricv0"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_gricv0_%d" % x.instance] = s
        if ("lyon_gricv1" in self.session.apps):
            for x in self.session.apps["lyon_gricv1"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_gricv1_%d" % x.instance] = s
        if ("lyon_mdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_mdcc'][0].sendCommand("RESET", m))
            s = json.loads(
                self.session.apps['lyon_mdcc'][0].sendCommand("ECALRESUME", m))
            r["lyon_mdcc"] = s
        if ("lyon_ipdc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_ipdc'][0].sendTransition("INITIALISE", m))
            r["lyon_ipdc"] = s

        if ("lyon_mbmdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_mbmdcc'][0].sendCommand("RESET", m))
            r["lyon_mbmdcc"] = s

        # old firmware
        if ("lyon_dif" in self.session.apps):
            for x in self.session.apps["lyon_dif"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_dif_%d" % x.instance] = s

        if ("lyon_sdcc" in self.session.apps):
            s = json.loads(
                self.session.apps['lyon_sdcc'][0].sendTransition("START", m))
            r["lyon_sdcc"] = s

        self.daq_answer = json.dumps(r)
        self.storeState()

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
        for k, v in self.session.apps.items():
            if (k != "lyon_dif"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                # print(mr)
                if (mr['status'] != "FAILED"):
                    rep["%s_%s_%d" % (s.host,k, s.infos['instance'])
                        ] = mr["DIFLIST"]
                else:
                    rep["%s_%s_%d" % (s.host,k, s.infos['instance'])] = mr
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
    # FEBV2 specific
    def febv2_start(self):
        """!
        FEBV2 only start a run (No event builder/MDCC)
        @deprecated Not used anywhere
        """
        r={}
        m={}
        if ("lyon_febv2" in self.session.apps):
            for x in self.session.apps["lyon_febv2"]:
                s = json.loads(x.sendTransition("START", m))
                r["lyon_febv2_%d" % x.instance] = s
            return json.dumps(r)
    def febv2_stop(self):
        """!
        FEBV2 only stop a run (No event builder/MDCC)
        @deprecated Not used anywhere
        """
        r={}
        m={}
        if ("lyon_febv2" in self.session.apps):
            for x in self.session.apps["lyon_febv2"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["lyon_febv2_%d" % x.instance] = s
            return json.dumps(r)
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

    def febScurve(self, ntrg, ncon, ncoff, thmin, thmax, step):
        """! Scurve FEBV1 in run control
        @deprecated Obsolete, use SCURVE command instead
        """
        r = {}
        self.mdcc_Pause()
        self.mdcc_setSpillOn(ncon,)
        print(" Clock On %d Off %d" % (ncon, ncoff))
        self.mdcc_setSpillOff(ncoff,)
        self.mdcc_setSpillRegister(4,)
        self.mdcc_CalibOn(1,)
        self.mdcc_setCalibCount(ntrg,)
        self.mdcc_Status()
        thrange = (thmax - thmin + 1) // step
        for vth in range(0, thrange+1):
            self.mdcc_Pause()
            self.setVthTime(thmax - vth * step)
            time.sleep(0.2)
            self.builder_setHeader(2, thmax - vth * step, 0xFF)

            # Check Last built event
            sr = json.loads(self.builderStatus())

            firstEvent = 0
            for k, v in sr.items():
                if (v["event"] > firstEvent):
                    firstEvent = v["event"]
            # print sr,firstEvent
            # Resume Calibration
            self.mdcc_ReloadCalibCount()
            self.mdcc_Resume()
            self.mdcc_Status()

            # Wait for ntrg events capture
            lastEvent = firstEvent
            nloop = 0
            while (lastEvent < (firstEvent + ntrg - 20)):
                sr = json.loads(self.builderStatus())
                lastEvent = 0
                for k, v in sr.items():
                    if (v["event"] > lastEvent):
                        lastEvent = v["event"]

                print(" %d First %d  Last %d Step %d" %
                      (thmax-vth*step, firstEvent, lastEvent, step))
                time.sleep(0.2)
                nloop = nloop+1
                if (nloop > 20):
                    break

            r["TH_%d" % (thmax-vth*step)] = lastEvent - firstEvent + 1

            # End Point
            self.mdcc_CalibOn(0,)
            self.mdcc_CalibOff()
            self.mdcc_Pause()
        return json.dumps(r)

    def runScurve(self, run, ch, spillon, spilloff, beg, las, step=2, asic=255, Comment="PR2 Calibration", Location="UNKNOWN", nevmax=50):
        """! Scurve FEBV1 in run control
        @deprecated Obsolete, use SCURVE command instead
        """
        oldfirmware = [3, 4, 5, 6, 7, 8, 9, 10, 11,
                       12, 20, 21, 22, 23, 24, 26, 28, 30]
        firmware = [0, 2, 4, 6, 7, 8, 10, 12,
                    14, 16, 18, 20, 22, 24, 26, 28, 30]

        comment = Comment + \
            " Mode %d ON %d Off %d TH min %d max %d Step %d" % (
                ch, spillon, spilloff, beg, las, step)
        self.comment = comment
        self.start()
        r = {}
        r["run"] = run
        if (ch == 255):
            print("Run Scurve on all channel together")
            mask = 0
            for i in firmware:
                mask = mask | (1 << i)
            self.setTdcMask(mask, asic)
            r["S_%d" % ch] = json.loads(self.febScurve(
                nevmax, spillon, spilloff, beg, las, step))
            self.stop()
            return json.dumps(r)
        if (ch == 1023):
            print("Run Scurve on all channel one by one")
            mask = 0
            for i in firmware:
                print("SCurve for channel %d " % i)
                mask = mask | (1 << i)
                self.setTdcMask(mask, asic)
                r["S_%d" % ch] = json.loads(self.febScurve(
                    nevmax, spillon, spilloff, beg, las, step))
            self.stop()
            return json.dumps(r)
        print("Run Scurve on  channel %d " % ch)
        mask = 0
        mask = mask | (1 << ch)
        self.setTdcMask(mask, asic)
        r["S_%d" % ch] = json.loads(self.febScurve(
            nevmax, spillon, spilloff, beg, las, step))
        self.stop()
        return json.dumps(r)
# DIF Specific

    def setControlRegister(self, ctrlreg):
        """! 
        DIF only Set the control register
        @param ctrlreg Control register value
        @return processCommand answer
        """
        param = {}
        param["value"] = int(ctrlreg, 16)
        return self.processCommand("CTRLREG", "lyon_dif", param)
