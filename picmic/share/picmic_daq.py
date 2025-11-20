#!/usr/bin/env python3

import numpy as np
from pprint import pprint
import datetime
import struct
import zstandard as zstd

import picmic_storage as ps


import sys, os
import logging
import time
import csv
import matplotlib.pyplot as plt
import liroc_ptdc_daq as daq
import picmic_register_access as cra
import os
import json
import agilent81160 as agp
import threading

N_ACQ = 40
N_WINDOW = 1

class picmic_normal_run:

    def __init__(self):
        # Configure logger level
        daq.configLogger(logging.INFO)

        self._thread = None
        self._running = threading.Event()
        self._lock = threading.Lock()
        self.status = {"run": 0, "event": 0}
        self.logger=logging.getLogger(__name__)
        self.pulser=None
    def initialise(self,board_id,state,version,filtering=True,falling=0,val_evt=0,pol_neg=0,dc_pa=0,mode='fine'):
        # Down load DB state and patch it
        self.board_id = board_id
        self.state = state
        self.version = version
        self.sdb = cra.instance()
        self.sdb.download_setup(state, version)
        self.sdb.to_csv_files()

        # Default threshold set at 800
        target = 800
        tlsb = target & 0xFF
        tmsb = (target >> 8) & 0xFF
        self.sdb.setup.boards[0].picmic.set("dac_threshold_lsb", tlsb)
        self.sdb.setup.boards[0].picmic.set("dac_threshold_msb", tmsb)
        
        # Maximal filtering
        if filtering:
            self.sdb.setup.boards[0].picmic.set("hrx_top_delay", 0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_top_bias", 0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_trailing", 1)
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_leading", 1)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_delay", 0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_bias", 0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_trailing", 1)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_leading", 1)
        else:
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_leading", 0)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_leading", 0)
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_trailing", 0)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_trailing", 0)
        # Falling ?
        self.sdb.setup.boards[0].picmic.set("falling_en", falling)
        # ValEvt
        self.sdb.setup.boards[0].picmic.set("Forced_ValEvt", val_evt)
        #self.sdb.setup.boards[0].picmic.set("EN-CLPS", 1)
        #self.sdb.setup.boards[0].picmic.set("EN-pE", 15)
        #self.sdb.setup.boards[0].picmic.set("PA_gain", 15)
        # Polarity
        self.sdb.setup.boards[0].picmic.set("Polarity", pol_neg)
        # DC_PA
        for ch in range(64):
            if dc_pa != 0:
                self.setup.boards[0].picmic.set("DC_PA_ch", dc_pa, ch)
        self.sdb.setup.version = 888
        self.sdb.to_csv_files()

        self.logger.info(f"Version {self.sdb.setup.boards[0].picmic_version}")

        # Open KC705
        self.kc705 = daq.KC705Board()
        self.kc705.init()

        self.feb = daq.FebBoard(self.kc705)
        self.feb.init()
        self.feb.loadConfigFromCsv(
            folder="/dev/shm/board_csv",
            config_name="%s_%d_f_%d_config_picmic.csv" % (self.state, 888, self.board_id),
        )
        self.feb.fpga.enableDownlinkFastControl()
        # disable all liroc channels
        for ch in range(64):
            self.feb.liroc.maskChannel(ch)
        self.feb.ptdc.setResMode(mode)

        self.feb.ptdc.powerup()

        self.kc705.fastbitConfigure(mode='normal',
        dig2_edge='rising', dig2_delay=1)

        # generate a few windows to flush out the agilient patterns
        self.kc705.acqSetWindow(1, 1)
        for _ in range(5): 
            self.kc705.ipbWrite('ACQ_CTRL.window_start', 1)
            self.kc705.ipbWrite('ACQ_CTRL.window_start', 0)
        # files
        self.storage=ps.storage_manager()
        #self.storage.open("unessai")
        self.runid=None

    def prepare_run(self,threshold,channel_list=[i for i in range(64)],ctest_list=[]):
        self.threshold=threshold
        for ch in range(64):
            if ch in channel_list:
                self.feb.liroc.maskChannel(ch, False)
            else:
                self.feb.liroc.maskChannel(ch, True)
            if ch in ctest_list:
                self.feb.liroc.setParam(f"Ctest_ch{ch}",True)
            else:
                self.feb.liroc.setParam(f"Ctest_ch{ch}",True)
                self.feb.liroc.maskChannel(ch, True)
        # Set the threshold
        self.feb.liroc.set10bDac(threshold)
        self.feb.liroc.stopScClock()

    def start_a_run(self,location,comment,params={"type":"NORMAL"}):
        if not "type" in params:
            self.logger.error("Run type should be specified in params")
            return
                         
        runobj = self.sdb.getRun(location,comment)
        self.runid = runobj["run"]
        if self.runid == None:
            self.runid = int(input("Enter a run number: "))

        # Store results in json
        self.storage.open(f"run_{self.runid}_{self.state}_{self.version}_{self.board_id}_{self.threshold}")

        self.run_type=1
        rh=np.array([self.run_type,self.threshold],dtype='int64')
        self.storage.writeRunHeader(self.runid,rh)
        if params["type"] == "NORMAL":
            self.normal_run()
        if params["type"] == "TIMELOOP":
            self.timeloop_run(params)
    def normal_run(self,params=None):
        with self._lock:
            if self._thread and self._thread.is_alive():
                self.logger.warning("Acquisition déjà en cours")
                return False
            self._running.set()
            self._thread = threading.Thread(target=self.normal_loop, args=(params,), daemon=True)
            self._thread.start()
            return True
    
    def normal_loop(self, params=None):
        self.logger.info("Acquisition thread démarré")
        while self._running.is_set():
            # simulate acquisition tick
            with self._lock:
                self.status["run"] = self.storage.run
                self.status["event"] = self.storage.event
                self.acquire_and_store(N_ACQ)
            if self.storage.event%100 == 0: 
                self.logger.info(f"Acquisition {self.storage.run} {self.storage.event}")
            time.sleep(0.001)
        self.logger.info("Acquisition thread arrêté")

        
    def timeloop_run(self,params=None):
        with self._lock:
            if self._thread and self._thread.is_alive():
                self.logger.warning("Acquisition déjà en cours please use stop before")
                return False
            self._running.set()
            self._thread = threading.Thread(target=self.time_loop, args=(params,), daemon=True)
            self._thread.start()
            return True
    def init_pulser(self,fname="/opt/pmdaq/picmic/etc/pulse_config.json"):
        self.pulser=agp.mod81160(fname)
        #agp.print_status()
        if self.pulser.inst!=None:
            self.pulser.print_status()
            self.pulser.configure_pulse()
            self.pulser.configure_trigger()
            self.pulser.setOFF(1)
            self.pulser.setOFF(2)
            self.pulser.print_status()
            #exit(0)
            input()
        else:
            self.logger.error("No agilent pulser")
            return
    def time_loop(self, params=None):
        self.logger.info("Acquisition thread démarré")
        if params == None:
            self.logger.error("Timeloop thread needs parameters exiting")
            return

        # Check pulser
        if self.pulser==None:
            self.logger.error("Timeloop thread needs pulser to be defined (call init_pulser before) exiting")
            return
        # Check parameters
        vmin=None
        if not "vmin" in params:
            self.logger.error("Timeloop thread needs 'vmin' parameter exiting")
            return
        else:
            vmin=params["vmin"]
        vmax=None
        if not "vmax" in params:
            self.logger.error("Timeloop thread needs 'vmax' parameter exiting")
            return
        else:
            vmax=params["vmax"]
        rise=None
        if not "rise" in params:
            self.logger.error("Timeloop thread needs 'rise' parameter exiting")
            return
        else:
            rise=params["rise"]*1.0E-9
        delay=None
        if not "delay" in params:
            self.logger.error("Timeloop thread needs 'delay' parameter exiting")
            return
        else:
            delay=params["delay"]*1.0E-9
        nstep=None
        if not "nstep" in params:
            self.logger.error("Timeloop thread needs 'nstep' parameter exiting")
            return
        else:
            nstep=params["nstep"]
        use_ctest=False
        if "ctest" in params:
            use_ctest=(params["ctest"]==1)
        nacq=N_ACQ
        if "nacq" in params:
            nacq=params["nacq"]
        # Now loop
        while self._running.is_set():
            # Set the pulser
            if (vmax>1.0 and use_ctest):
                self.logger.error(f"No automatic scan with High V {vmax} greater than 1.0 V")
            if nstep == 0:
                self.logger.error(f"nstep is 0 , only one acquistion of {nacq} event with {vmax} settings")
                self.setup_injection(vmax,rise,delay,use_ctest)
                self.acquire_and_store(nacq)
                self.status["run"] = self.storage.run
                self.status["event"] = self.storage.event
                self.pulser.setOFF(1)
                self.pulser.setOFF(2)
                break
            else:
                vstep=(vmax-vmin)/nstep
                vhigh=[round(x,3) for x in np.arange(vmin,vmax+1E-3,vstep).tolist()]
                for vset in vhigh:
                    self.logger.info(f"Step {vset}  acquistion of {nacq} events")
                    self.setup_injection(vset,rise,delay,use_ctest)
                    self.acquire_and_store(nacq)
                    self.status["run"] = self.storage.run
                    self.status["event"] = self.storage.event
                self.pulser.setOFF(1)
                self.pulser.setOFF(2)
                break
        self.logger.info("TimeLoop Acquisition thread arrêté")

    def stop(self, params=None):
        with self._lock:
            if not (self._thread and self._thread.is_alive()):
                self.logger.warning("Acquisition non démarrée")
                return False
            self._running.clear()
            # join optionnel court
            self._thread.join(timeout=2)
            return True
        self.storage.close()
    def running(self):
        return self._running.is_set()

    def get_status(self):
        with self._lock:
            return dict(self.status, running=self.running())

    def setup_injection(self,vout,rise=1.0E-9,delay=120E-9,use_ctest=False):
        if (vout>1.0 and use_ctest):
            self.logger.error(f"No automatic scan with High V {vout} greater than 1.0 V")
            self.pulser.setOFF(1)
            self.pulser.setOFF(2)
            return
        self.pulser.setVoltage(1,0,vout)
        self.pulser.setRiseTime(1,rise)
        self.pulser.setDelay(1,delay)
        self.pulser.setON(1)
        self.pulser.setON(2)
        self.run_type=0x10
        rh=np.array([self.run_type,int(vout*1000),int(rise*1E10),int(delay*1E9)],dtype='int64')
        self.storage.writeRunHeader(self.runid,rh)
        

    def acquire_and_store(self,n_acq,window_size=400,dead_time=50,n_window=1000):
        for i in range(n_acq):
            words = self.kc705.acqPtdc(window=window_size, deadtime=dead_time, window_number=n_window)
            # File storage
            self.storage.writeEvent(words)
        return
    
