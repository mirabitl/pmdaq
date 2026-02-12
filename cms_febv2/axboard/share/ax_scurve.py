#!/usr/bin/env python3
import logging
import sys
import threading
import time


import numpy as np 
from matplotlib import pyplot as plt


import cms_irpc_feb_lightdaq as lightdaq

import csv_register_access as cra
import os
import json
import queue

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/febv2debug.log", mode='w')  # ,
        # logging.StreamHandler()
    ]
)

class scurve_processor:
    def __init__(self,params):
       
        self.pb=ax_scurves(params["db"]["board"],
                              params["db"]["state"],
                              params["db"]["version"],
                              )
                       
        self.res={}
        self.res["state"]=params["db"]["state"]
        self.res["version"]=params["db"]["version"]
        self.res["feb"]=params["db"]["board"]
        self.res["thmin"]=params["thmin"]
        self.res["thmax"]=params["thmax"]
        self.res["thstep"]=params["thstep"]
        self.res["ctime"]=time.time()
        if "location" in params:
            self.res["location"]=params["location"]

        self.conf=params
        self.logger=logging.getLogger(__name__)
        self._thread = None
        self._running = threading.Event()
        self._lock = threading.Lock()
    def reset_results(self):
        params=self.conf
        self.res={}
        self.res["state"]=params["db"]["state"]
        self.res["version"]=params["db"]["version"]
        self.res["feb"]=params["db"]["board"]
        self.res["thmin"]=params["thmin"]
        self.res["thmax"]=params["thmax"]
        self.res["thstep"]=params["thstep"]
        self.res["ctime"]=time.time()
        if "location" in params:
            self.res["location"]=params["location"]
    def stop(self):
        self._running.clear()
        if self._thread:
            self._thread.join(timeout=10)
    def get_status(self):
        with self._lock:
            status_copy = self.pb.status.copy()
        status_copy["running"]=self._running.is_set()
        return status_copy
    def start_align(self,params=None):
        if not self._running.is_set():
            self.logger.info("running lock is cleared") 
        if self._thread:
            self._thread.join(timeout=10)
        if self._thread and self._thread.is_alive():
            self.logger.warning("Calibration déjà en cours")
            return False
        self._running.set()
        self.pb.status={}
        self._thread = threading.Thread(target=self.align, args=(params,), daemon=True)
        self._thread.start()
        return True    
    def align(self,params=None):
        self.pb.status["method"]="aligning"
        self.pb._running=self._running
        for an in self.pb.asicl:
            self.logger.info(f"Aligning ASIC {an}")
            target,v6_cor=self.align_asic(an)
            if hasattr(self,'_running') and not self._running.is_set():
                self.logger.info("Alignment was stop before the end")
                return
            for  ich in range(len(v6_cor)):
                self.pb.sdb.setup.febs[0].petiroc.set_6b_dac(ich,v6_cor[ich],an.upper()) 
                self.pb.sdb.setup.febs[0].petiroc.set_parameter("10b_dac_vth_discri_time",target,an.upper())
            self.logger.info(f"ASIC {an} aligned to target {target}")
            self.logger.info(f"New DAC6b values set {v6_cor}")
            # Save to DB
              
        if "comment" in self.conf:
            self.pb.sdb.setup.version=self.conf["db"]["version"]
            self.pb.sdb.upload_changes(self.conf["comment"])
        self._running.clear()
    def align_asic(self,asic_name):
        _,_,used_chan =self.pb.get_mapping(asic_name)
        v6=self.pb.sdb.setup.febs[0].petiroc.get_6b_dac(asic_name)
        turn_on=[]
        self.status["asic"]=asic_name
        self.status["raw_turnon"]=turn_on
        for idx in range(16):
            if hasattr(self,'_running') and not self._running.is_set():
                self.logger.info("Alignment was stop before the end")
                return 0,[]
            to=self.pb.pedestal_one_channel(asic_name,idx,self.conf["thmin"],self.conf["thmax"],v6)
            turn_on.append(to)
        self.logger.info(f" Turn ON {turn_on}")
        nto=np.array(turn_on)
        # Target
        target=round(np.median(nto))
        self.logger.info(f"Median target {target}")
        ## Check minimal gain value
        too_low=False
        too_high=False
        vexp=[]
        for idx in range(16):
            petiroc_chan,_ =used_chan[idx]
            gc=v6[ petiroc_chan]+round((target-turn_on[idx])/3.9)
            vexp.append(gc)
            too_low=too_low or (gc<3)
            too_high=too_high or (gc>61)
        if (too_low):
            target=target+10
        if (too_high):
            target=target-10
        self.logger.info(f"Median target final {target} dac6b {vexp}")
        self.status["target"]=target
        self.status["dac_local"]=[0 for i in range(32)]
        #val=input("Second round ? ")
        v6_cor=v6
        turn_on_cor=[]
        for idx in range(16):
            if hasattr(self,'_running') and not self._running.is_set():
                self.logger.info("Alignment was stop before the end")
                return 0,[]
            petiroc_chan,tdc_ch =used_chan[idx]
            gc=v6[ petiroc_chan]+round((target-turn_on[idx])/3.9)
            td=[]
            for ig in range(5):
                g=gc-2+ig
                vc=v6
                if (g<=1):
                    g=1
                if (g>=63):
                    g=63
                vc[ petiroc_chan]=int(g)
                
                to=self.pb.pedestal_one_channel(asic_name,idx,target-15,target+15,vc,two_steps=False)
                td.append(to)
            #print(gc)
            #print(td)
            # find best value
            mindist=1000
            imin=2
            for ig in range(5):
                if (abs(td[ig]-target)<mindist):
                    mindist=abs(td[ig]-target)
                    imin=ig
            turn_on_cor.append(td[imin])
            newdac=int(gc-2+imin)
            if (newdac<1):
                newdac=1
            if (newdac>63):
                newdac=63
            v6_cor[petiroc_chan]=newdac
            self.status["dac_local"][petiroc_chan]=newdac
        self.logger.info(f" Turn ON {turn_on_cor}")
        ntoc=np.array(turn_on_cor)
        # Target
        target1=int(round(np.median(ntoc)))
        self.logger.info(f"Median target {target1}")
        self.logger.info(v6_cor)
        return target1,v6_cor
       
    def start_scurves(self,params={"analysis":"SCURVE_A","plot_fig":None}):
        if not self._running.is_set():
            self.logger.info("running lock is cleared") 
        if self._thread:
            self._thread.join(timeout=10)
        if self._thread and self._thread.is_alive():
            self.logger.warning("Calibration déjà en cours")
            return False
        self._running.set()
        self.pb.status={}
        self._thread = threading.Thread(target=self.get_scurves, args=(params,), daemon=True)
        self._thread.start()
        return True            
    #def get_scurves(self,analysis="SCURVE_A",plot_fig=None):
    def get_scurves(self,params,doPlot=False):
        # Now get a run id
        analysis=params.get("analysis","SCURVE_A")
        plot_fig=params.get("plot_fig",None)
        self.logger.info(f"Plot fig set to {plot_fig}")
        self.pb.status["method"]=f"{analysis}"
        self.pb._running=self._running
        if hasattr(self, 'queue'):
            self.pb.queue=self.queue
        runid=None
        if "location" in self.conf and "comment" in self.conf:
            runobj=self.pb.sdb.getRun(self.conf["location"],self.conf["comment"])
            runid=runobj['run']
                                      
        for an in self.pb.asicl:
            if (hasattr(self,'_running') and  (not self._running.is_set())):
                break
            self.reset_results()
            _,_,used_chan =self.pb.get_mapping(an)
            self.res["analysis"]=analysis
            self.res["asic"]=an.upper()
            self.res["channels"]=[]
            scurves=None
            ax=None    
            if doPlot:
            # Plots
                if plot_fig!=None:
                    # Efface l'ancienne figure
                    plot_fig.clear()
                    ax=plot_fig.add_subplot(111)
            # Check the analysis
            if analysis == "SCURVE_A":
                self.logger.info(f'start={self.conf["thmin"]},stop={self.conf["thmax"]},step={self.conf["thstep"]},dac_loc=0')
                #input()
                scurves=self.pb.make_pedestal_all_channels(asic_name=an,thmin=self.conf["thmin"],thmax=self.conf["thmax"],thstep=self.conf["thstep"],v6=None)
                self.logger.info(scurves)
            elif analysis == "SCURVE_1":
                scurves=self.pb.make_pedestal_one_by_one(asic_name=an,thmin=self.conf["thmin"],thmax=self.conf["thmax"],thstep=self.conf["thstep"],v6=None)
            else:
                return False
            local_plot=False
            for petiroc_chan, tdc_chan in used_chan:
                rc={}
                rc["prc"]=petiroc_chan
                rc["tdc"]=tdc_chan
                rc["scurve"]=scurves[tdc_chan]
                if (len(scurves[tdc_chan])==0):
                    continue
                #self.logger.warning(rc)
                self.res["channels"].append(rc)
                if doPlot:
                    if plot_fig==None or local_plot:
                        plt.plot(range(self.conf["thmin"], self.conf["thmax"],1), scurves[tdc_chan], '+-', label=f"ch{tdc_chan}")
                    else:
                        ax.plot(
                        range(self.conf["thmin"], self.conf["thmax"], 1),
                            scurves[tdc_chan],
                            '+-',
                            label=f"ch{tdc_chan}"
                    )
            if doPlot: 
                if plot_fig==None or local_plot:
                    plt.grid()
                    plt.legend(loc="upper right")
                    plt.show()
                else:
                    ax.grid()
                    ax.legend(loc="upper right")
                    if hasattr(self, 'queue'):
                        self.queue.put("update_plot")
            if hasattr(self,'_running') and not self._running.is_set():
                self.logger.info("Calibration was stop before the end")
                break
            # Store results in json
            res_dir='/tmp/results/%s_%d_f_%d' % (self.conf["db"]["state"],self.conf["db"]["version"],self.conf["db"]["board"])
            if runid==None:
                runid=int(input("Enter a run number: "))
            os.system("mkdir -p %s" % res_dir)
            fout=open(f"{res_dir}/{an}_scurves_all_channels_{runid}.json","w")
            fout.write(json.dumps(self.res))
            fout.close()
            
            if "location" in self.conf and "comment" in self.conf:
                self.pb.sdb.upload_results(self.res["state"],self.res["version"],self.res["feb"],self.res["analysis"],self.res,comment=self.conf["comment"],runid=runid)
                self.logger.info(f'Upload done {self.res["state"]} {self.res["version"]} {self.res["feb"]} {self.res["analysis"]} {self.conf["comment"]} {runid}')
                #input("Next asic ?")
        return True
                
    

class ax_scurves:
    def __init__(self,feb_id,state,version):
        self.status={}
        self.dbstate=state
        self.dbversion=version
        self.feb_id=feb_id
        self.sdb=cra.instance()
        self.sdb.download_setup(state,version)
        self.sdb.setup.febs[0].petiroc.set_parameter("10b_dac_vth_discri_time",800)
        for i in range(16):
            self.sdb.setup.febs[0].fpga.set_pair_filtering_en(i,0)
        self.sdb.setup.version=998
        self.sdb.to_csv_files()
        self.logger=logging.getLogger(__name__)

        self.logger.info(self.sdb.setup.febs[0].fpga_version,self.sdb.setup.febs[0].petiroc_version)
        lightdaq.configLogger(loglevel=logging.INFO)
        self.daqlogger = logging.getLogger('CMS_IRPC_FEB_LightDAQ')
        self.daqlogger.setLevel(logging.INFO)

        try:
            self.ax7325b = lightdaq.AX7325BBoard()
            self.feb = lightdaq.FebV2Board(self.ax7325b, febid='FEB0', fpga_fw_ver='4.8')
            self.ax7325b.init(feb0=True, feb1=False)
            ### Test
            self.sdb.setup.febs[0].fpga_version='4.8'
            self.feb.init()
            self.feb.loadConfigFromCsv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (state,998,feb_id))
        except NameError as e:
            self.logger.info(f"Test failed with message: {e}")
        #lightdaq.configLogger(logging.INFO)
        #logger=logging.getLogger('CMS_IRPC_FEB_LightDAQ')
        self.logger.setLevel(logging.INFO)

        self.asicl=["left_top","left_bot","middle_top","middle_bot","right_top","right_bot"]

    def get_mapping(self,asic_name):
        try:

            used_chan = [  
                (5  , 0),
                (6  , 1),
                (7  , 2),
                (8  , 3),
                (9  , 4),
                (10 , 5),
                (11 , 6),
                (12 , 7),
                (19 , 8),
                (20 , 9),
                (21 , 10),
                (22 , 11),
                (23 , 12),
                (24 , 13),
                (25 , 14),
                (26 , 15)
            ]
            used_chan_top = [
                (5  , 0),
                (6  , 1),
                (7  , 2),
                (8  , 3),
                (9  , 4),
                (10 , 5),
                (11 , 6),
                (12 , 7),
                (19 , 8),
                (20 , 9),
                (21 , 10),
                (22 , 11),
                (23 , 12),
                (24 , 13),
                (25 , 14),
                (26 , 15)

            ]
            used_chan_bot = [  
                (5  , 16),
                (6  , 17),
                (7  , 18),
                (8  , 19),
                (9  , 20),
                (10 , 21),
                (11 , 22),
                (12 , 23),
                (19 , 24),
                (20 , 25),
                (21 , 26),
                (22 , 27),
                (23 , 28),
                (24 , 29),
                (25 , 30),
                (26 , 31)
            ]
            

            f_tdc=None
            f_asic=None
            used_chan=used_chan_top
            if (asic_name=='left_top'):
                f_tdc=self.feb.fpga_left
                f_asic=self.feb.asic_left_top
            if (asic_name=='left_bot'):
                f_tdc=self.feb.fpga_left
                f_asic=self.feb.asic_left_bot
                used_chan=used_chan_bot
            if (asic_name=='middle_top'):
                f_tdc=self.feb.fpga_middle
                f_asic=self.feb.asic_middle_top
            if (asic_name=='middle_bot'):
                f_tdc=self.feb.fpga_middle
                f_asic=self.feb.asic_middle_bot
                used_chan=used_chan_bot
            if (asic_name=='right_bot'):
                f_tdc=self.feb.fpga_right
                f_asic=self.feb.asic_right_bot
                used_chan=used_chan_bot
            if (asic_name=='right_top'):
                f_tdc=self.feb.fpga_right
                f_asic=self.feb.asic_right_top

            return (f_tdc,f_asic,used_chan)
        except NameError as e:
            self.logger.info(f"Test failed with message: {e}")

    def make_pedestal_all_channels(self,asic_name,thmin,thmax,thstep,v6=None):
        try:
            f_tdc,f_asic,used_chan =self.get_mapping(asic_name)

            used_petiroc_chan = [x[0] for x in used_chan]
            for petiroc_chan in range(32):
                en = petiroc_chan not in used_petiroc_chan
                f_asic.set(f"mask_discri_time_ch{petiroc_chan}", en)
                #self.feb.asic_right_top.set(f"6b_dac_ch{petiroc_chan}", 32)
            if (v6!=None):
                for o_chan in range(32):
                    f_asic.set(f"6b_dac_ch{o_chan}", v6[o_chan])
            f_tdc.tdcSetInjectionMode('standard')
            f_tdc.tdcEnable(False)
            f_tdc.tdcSetCounterTimeWindow(int(1e-3/8.33e-9)) # 1ms


            scurves = [ [] for _ in range(32) ]
            for dac10b_val in range(thmin, thmax, thstep):
                f_asic.set('10b_dac_vth_discri_time', dac10b_val)
                self.feb.sendPetirocConfig(asic_name)
                f_tdc.tdcEnable()
                f_tdc.tdcStartCounter()
                time.sleep(0.001)
                f_tdc.tdcEnable(False)
                counters = f_tdc.tdcGetCounterData()
                for _, tdc_ch in used_chan:
                    scurves[tdc_ch].append(counters[tdc_ch])
                self.logger.info(f"{dac10b_val} / {2**10} {counters}")
            if hasattr(self, 'queue'):
                self.status["asic"]=asic_name
                self.status["thmin"]=thmin
                self.status["thmax"]=thmax
                self.status["thstep"]=thstep
                self.status["scurves"]= scurves
                self.queue.put("update_scurves")
                time.sleep(1)
            return scurves
        except NameError as e:
            self.logger.info(f"Test failed with message: {e}")

    def make_pedestal_one_by_one(self,asic_name,thmin,thmax,thstep,v6=None):
        try:
            f_tdc,f_asic,used_chan =self.get_mapping(asic_name)

            scurves = [ [] for _ in range(32) ]
            for petiroc_chan,tdc_ch in used_chan:
                if (hasattr(self,'_running') and  (not self._running.is_set())):
                    return scurves
                    break
                en=1
                for o_chan in range(32):

                    if (o_chan==petiroc_chan):
                        en = 0
                    else:
                        en=1
                    f_asic.set(f"mask_discri_time_ch{o_chan}", en)
                if (v6!=None):
                    for o_chan in range(32):

                        if (o_chan==petiroc_chan):
                            en = v6[o_chan]
                        else:
                            en=1
                        f_asic.set(f"6b_dac_ch{o_chan}", en)
                                    
                f_tdc.tdcSetInjectionMode('standard')
                f_tdc.tdcEnable(False)
                f_tdc.tdcSetCounterTimeWindow(int(1e-3/8.33e-9)) # 1ms

                for dac10b_val in range(thmin, thmax, thstep):
    #            for dac10b_val in range(thmax, thmin, -1*thstep):
                    f_asic.set('10b_dac_vth_discri_time', dac10b_val)
                    self.feb.sendPetirocConfig(asic_name)
                    f_tdc.tdcEnable()
                    f_tdc.tdcStartCounter()
                    time.sleep(0.001)
                    f_tdc.tdcEnable(False)
                    counters = f_tdc.tdcGetCounterData()
                    
                    scurves[tdc_ch].append(counters[tdc_ch])
                    self.logger.info(f"{dac10b_val} / {2**10} {counters}")
                if hasattr(self, 'queue'):
                    self.status["asic"]=asic_name
                    self.status["thmin"]=thmin
                    self.status["thmax"]=thmax
                    self.status["thstep"]=thstep
                    self.status["petiroc"]=petiroc_chan
                    self.status["tdc"]=tdc_ch
                    self.status["scurve"]= scurves[tdc_ch]
                    self.queue.put("update_scurve")
            if hasattr(self, 'queue'):
                self.status["asic"]=asic_name
                self.status["thmin"]=thmin
                self.status["thmax"]=thmax
                self.status["thstep"]=thstep
                self.status["scurves"]= scurves
                self.queue.put("update_scurves")
                time.sleep(1)
             
            return scurves


        except TestError as e:
            self.logger.info(f"Test failed with message: {e}")

    def pedestal_one_channel(self,asic_name,index,thmin,thmax,v6=None,two_steps=True,dac6=32):
        f_tdc,f_asic,used_chan =self.get_mapping(asic_name)
        petiroc_chan,tdc_ch =used_chan[index]
        
        en=1
        for o_chan in range(32):
            
            if (o_chan==petiroc_chan):
                en = 0
            else:
                en=1
            f_asic.set(f"mask_discri_time_ch{o_chan}", en)
        if (v6!=None):
            p_chans=[]
            for ic in range(len(used_chan)):
                p_chans.append(used_chan[ic][0])
            for ic in range(32):
                if not (ic in p_chans):
                    v6[ic]=dac6
                f_asic.set(f"6b_dac_ch{ic}",v6[ic])
    #        for o_chan in range(32):
    #            if (o_chan==petiroc_chan):
    #                en = v6[o_chan]
    #            else:
    #                en=1
    #            f_asic.set(f"6b_dac_ch{o_chan}", en)
                    
        f_tdc.tdcSetInjectionMode('standard')
        f_tdc.tdcEnable(False)
        f_tdc.tdcSetCounterTimeWindow(int(1e-3/8.33e-9)) # 1ms


        # First round thstep=5
        ti=thmin
        ta=thmax
        to_0=(thmax+thmin)//2
        if (two_steps):
            thstep=5
            scurve1_th=[]
            scurve1=[]
            for dac10b_val in range(thmin, thmax, thstep):
    #            for dac10b_val in range(thmax, thmin, -1*thstep):
                f_asic.set('10b_dac_vth_discri_time', dac10b_val)
                self.feb.sendPetirocConfig(asic_name)
                f_tdc.tdcEnable()
                f_tdc.tdcStartCounter()
                time.sleep(0.001)
                f_tdc.tdcEnable(False)
                counters = f_tdc.tdcGetCounterData()
                    
                scurve1.append(counters[tdc_ch])
                scurve1_th.append(dac10b_val)
                #print(f"{dac10b_val} / {2**10}", counters)
                # Now find the transition
            cmax=max(scurve1)
            #print(scurve1)
            #print(cmax)
            to_0=(thmax-thmin+1)//2
            for t in range(len(scurve1)):
                if (scurve1[t]<0.7*cmax):
                    to_0=scurve1_th[t]
                    break
            # now repeat scan between to_0-10 and to_0+10
            ti=round(max(to_0-15,thmin))
            ta=round(min(to_0+15,thmax))
            
        scurve2=[]
        scurve2_th=[]

        thstep=1
        for dac10b_val in range(int(ti), int(ta), thstep):
    #            for dac10b_val in range(thmax, thmin, -1*thstep):
            f_asic.set('10b_dac_vth_discri_time', dac10b_val)
            self.feb.sendPetirocConfig(asic_name)
            f_tdc.tdcEnable()
            f_tdc.tdcStartCounter()
            time.sleep(0.001)
            f_tdc.tdcEnable(False)
            counters = f_tdc.tdcGetCounterData()
                    
            scurve2.append(counters[tdc_ch])
            scurve2_th.append(dac10b_val)
            #print(f"{dac10b_val} / {2**10}", counters)    
    # Now find the transition
        cmax1=max(scurve2)
        to_1i=to_0
        for t in range(len(scurve2)):
            if (scurve2[t]<0.7*cmax1):
                to_1i=scurve2_th[t]
                break
        to_1a=to_1i
        for t in range(len(scurve2)):
            if (scurve2[t]<0.3*cmax1):
                to_1a=scurve2_th[t]
                break
        to_1=(to_1a+to_1i)//2
        self.logger.info(f"Seuil raw {to_0}")
        self.logger.info(f"Scan {ti}-{ta}  {scurve2}")
        self.logger.info(f"Seuil fin {to_1}")
        #val=input("Next channel? ")
        return to_1

        
def pedestal_one_channel(feb,asic_name,index,thmin,thmax,v6=None,two_steps=True,dac6=32):
    f_tdc,f_asic,used_chan =get_mapping(asic_name,feb)
    petiroc_chan,tdc_ch =used_chan[index]
    
    en=1
    for o_chan in range(32):

        if (o_chan==petiroc_chan):
            en = 0
        else:
            en=1
        f_asic.set(f"mask_discri_time_ch{o_chan}", en)
    if (v6!=None):
        p_chans=[]
        for ic in range(len(used_chan)):
            p_chans.append(used_chan[ic][0])
        for ic in range(32):
            if not (ic in p_chans):
                v6[ic]=dac6
            f_asic.set(f"6b_dac_ch{ic}",v6[ic])
#        for o_chan in range(32):
#            if (o_chan==petiroc_chan):
#                en = v6[o_chan]
#            else:
#                en=1
#            f_asic.set(f"6b_dac_ch{o_chan}", en)
                
    f_tdc.tdcSetInjectionMode('standard')
    f_tdc.tdcEnable(False)
    f_tdc.tdcSetCounterTimeWindow(int(1e-3/8.33e-9)) # 1ms


    # First round thstep=5
    ti=thmin
    ta=thmax
    to_0=(thmax+thmin)//2
    if (two_steps):
        thstep=5
        scurve1_th=[]
        scurve1=[]
        for dac10b_val in range(thmin, thmax, thstep):
#            for dac10b_val in range(thmax, thmin, -1*thstep):
            f_asic.set('10b_dac_vth_discri_time', dac10b_val)
            self.feb.sendPetirocConfig(asic_name)
            f_tdc.tdcEnable()
            f_tdc.tdcStartCounter()
            time.sleep(0.001)
            f_tdc.tdcEnable(False)
            counters = f_tdc.tdcGetCounterData()
                
            scurve1.append(counters[tdc_ch])
            scurve1_th.append(dac10b_val)
            #print(f"{dac10b_val} / {2**10}", counters)
            # Now find the transition
        cmax=max(scurve1)
        #print(scurve1)
        #print(cmax)
        to_0=(thmax-thmin+1)//2
        for t in range(len(scurve1)):
            if (scurve1[t]<0.7*cmax):
                to_0=scurve1_th[t]
                break
        # now repeat scan between to_0-10 and to_0+10
        ti=round(max(to_0-15,thmin))
        ta=round(min(to_0+15,thmax))
        
    scurve2=[]
    scurve2_th=[]

    thstep=1
    for dac10b_val in range(int(ti), int(ta), thstep):
#            for dac10b_val in range(thmax, thmin, -1*thstep):
        f_asic.set('10b_dac_vth_discri_time', dac10b_val)
        self.feb.sendPetirocConfig(asic_name)
        f_tdc.tdcEnable()
        f_tdc.tdcStartCounter()
        time.sleep(0.001)
        f_tdc.tdcEnable(False)
        counters = f_tdc.tdcGetCounterData()
                
        scurve2.append(counters[tdc_ch])
        scurve2_th.append(dac10b_val)
        #print(f"{dac10b_val} / {2**10}", counters)    
# Now find the transition
    cmax1=max(scurve2)
    to_1i=to_0
    for t in range(len(scurve2)):
        if (scurve2[t]<0.7*cmax1):
            to_1i=scurve2_th[t]
            break
    to_1a=to_1i
    for t in range(len(scurve2)):
        if (scurve2[t]<0.3*cmax1):
            to_1a=scurve2_th[t]
            break
    to_1=(to_1a+to_1i)//2
    self.logger.info(f"Seuil raw {to_0}")
    self.logger.info(f"Scan {ti}-{ta}  {scurve2}")
    self.logger.info(f"Seuil fin {to_1}")
    #val=input("Next channel? ")
    return to_1

def make_pedestal_one_by_one(state,version,feb_id,feb,asic_name,thmin,thmax,thstep,v6=None):
    try:

        used_chan = [  
            (5  , 0),
            (6  , 1),
            (7  , 2),
            (8  , 3),
            (9  , 4),
            (10 , 5),
            (11 , 6),
            (12 , 7),
            (19 , 8),
            (20 , 9),
            (21 , 10),
            (22 , 11),
            (23 , 12),
            (24 , 13),
            (25 , 14),
            (26 , 15)
        ]
        used_chan_top = [
            (5  , 0),
            (6  , 1),
            (7  , 2),
            (8  , 3),
            (9  , 4),
            (10 , 5),
            (11 , 6),
            (12 , 7),
            (19 , 8),
            (20 , 9),
            (21 , 10),
            (22 , 11),
            (23 , 12),
            (24 , 13),
            (25 , 14),
            (26 , 15)

        ]
        used_chan_bot = [  
            (5  , 16),
            (6  , 17),
            (7  , 18),
            (8  , 19),
            (9  , 20),
            (10 , 21),
            (11 , 22),
            (12 , 23),
            (19 , 24),
            (20 , 25),
            (21 , 26),
            (22 , 27),
            (23 , 28),
            (24 , 29),
            (25 , 30),
            (26 , 31)
        ]
        

        f_tdc=None
        f_asic=None
        used_chan=used_chan_top
        if (asic_name=='left_top'):
            f_tdc=self.feb.fpga_left
            f_asic=self.feb.asic_left_top
        if (asic_name=='left_bot'):
            f_tdc=self.feb.fpga_left
            f_asic=self.feb.asic_left_bot
            used_chan=used_chan_bot
        if (asic_name=='middle_top'):
            f_tdc=self.feb.fpga_middle
            f_asic=self.feb.asic_middle_top
        if (asic_name=='middle_bot'):
            f_tdc=self.feb.fpga_middle
            f_asic=self.feb.asic_middle_bot
            used_chan=used_chan_bot
        if (asic_name=='right_bot'):
            f_tdc=self.feb.fpga_right
            f_asic=self.feb.asic_right_bot
            used_chan=used_chan_bot
        if (asic_name=='right_top'):
            f_tdc=self.feb.fpga_right
            f_asic=self.feb.asic_right_top


        used_petiroc_chan = [x[0] for x in used_chan]
        scurves = [ [] for _ in range(32) ]         
        res={}
        res["state"]=state
        res["version"]=version
        res["feb"]=feb_id
        res["thmin"]=thmin
        res["thmax"]=thmax
        res["thstep"]=thstep
        res["asic"]=asic_name.upper()
        res["ctime"]=time.time()
        res["analysis"]="SCURVE_1"

        res["channels"]=[]
        
        for petiroc_chan,tdc_ch in used_chan:
            en=1
            for o_chan in range(32):

                if (o_chan==petiroc_chan):
                    en = 0
                else:
                    en=1
                f_asic.set(f"mask_discri_time_ch{o_chan}", en)
            if (v6!=None):
                for o_chan in range(32):

                    if (o_chan==petiroc_chan):
                        en = v6[o_chan]
                    else:
                        en=1
                    f_asic.set(f"6b_dac_ch{o_chan}", en)
                
            
            f_tdc.tdcSetInjectionMode('standard')
            f_tdc.tdcEnable(False)
            f_tdc.tdcSetCounterTimeWindow(int(1e-3/8.33e-9)) # 1ms


            for dac10b_val in range(thmin, thmax, thstep):
#            for dac10b_val in range(thmax, thmin, -1*thstep):
                f_asic.set('10b_dac_vth_discri_time', dac10b_val)
                self.feb.sendPetirocConfig(asic_name)
                f_tdc.tdcEnable()
                f_tdc.tdcStartCounter()
                time.sleep(0.001)
                f_tdc.tdcEnable(False)
                counters = f_tdc.tdcGetCounterData()
                
                scurves[tdc_ch].append(counters[tdc_ch])
                self.logger.info(f"{dac10b_val} / {2**10} {counters}")


            rc={}
            rc["prc"]=petiroc_chan
            rc["tdc"]=tdc_ch
            rc["scurve"]=scurves[tdc_ch]
            res["channels"].append(rc)
            plt.plot(range(thmin, thmax, thstep), scurves[tdc_ch], '+-', label=f"top right ch{petiroc_chan}")
        plt.grid()
        plt.legend(loc="upper right")
        plt.show() 
        # Store results in json
        res_dir='results/%s_%d_f_%d' % (state,version,feb_id)
        os.system("mkdir -p %s" % res_dir)
        fout=open("%s/scurves_one_by_one_%s.json" % (res_dir,asic_name),"w")
        fout.write(json.dumps(res))
        fout.close
        return res


    except TestError as e:
        self.logger.info(f"Test failed with message: {e}")




def make_pedestal_all_channels(state,version,feb_id,feb,asic_name,thmin,thmax,thstep,v6=None):
    try:
        used_chan_top = [
            (5  , 0),
            (6  , 1),
            (7  , 2),
            (8  , 3),
            (9  , 4),
            (10 , 5),
            (11 , 6),
            (12 , 7),
            (19 , 8),
            (20 , 9),
            (21 , 10),
            (22 , 11),
            (23 , 12),
            (24 , 13),
            (25 , 14),
            (26 , 15)

        ]
        used_chan_bot = [  
            (5  , 16),
            (6  , 17),
            (7  , 18),
            (8  , 19),
            (9  , 20),
            (10 , 21),
            (11 , 22),
            (12 , 23),
            (19 , 24),
            (20 , 25),
            (21 , 26),
            (22 , 27),
            (23 , 28),
            (24 , 29),
            (25 , 30),
            (26 , 31)
        ]
        nstep=thmax-thmin
        
        f_tdc=None
        f_asic=None
        used_chan=used_chan_top
        if (asic_name=='left_top'):
            f_tdc=self.feb.fpga_left
            f_asic=self.feb.asic_left_top
        if (asic_name=='left_bot'):
            f_tdc=self.feb.fpga_left
            f_asic=self.feb.asic_left_bot
            used_chan=used_chan_bot
        if (asic_name=='middle_top'):
            f_tdc=self.feb.fpga_middle
            f_asic=self.feb.asic_middle_top
        if (asic_name=='middle_bot'):
            f_tdc=self.feb.fpga_middle
            f_asic=self.feb.asic_middle_bot
            used_chan=used_chan_bot
        if (asic_name=='right_bot'):
            f_tdc=self.feb.fpga_right
            f_asic=self.feb.asic_right_bot
            used_chan=used_chan_bot
        if (asic_name=='right_top'):
            f_tdc=self.feb.fpga_right
            f_asic=self.feb.asic_right_top

        used_petiroc_chan = [x[0] for x in used_chan]
        for petiroc_chan in range(32):
            en = petiroc_chan not in used_petiroc_chan
            f_asic.set(f"mask_discri_time_ch{petiroc_chan}", en)
            #self.feb.asic_right_top.set(f"6b_dac_ch{petiroc_chan}", 32)
        if (v6!=None):
            for o_chan in range(32):
                f_asic.set(f"6b_dac_ch{o_chan}", v6[o_chan])
        f_tdc.tdcSetInjectionMode('standard')
        f_tdc.tdcEnable(False)
        f_tdc.tdcSetCounterTimeWindow(int(1e-3/8.33e-9)) # 1ms


        scurves = [ [] for _ in range(32) ]
        for dac10b_val in range(thmin, thmax, thstep):
            f_asic.set('10b_dac_vth_discri_time', dac10b_val)
            self.feb.sendPetirocConfig(asic_name)
            f_tdc.tdcEnable()
            f_tdc.tdcStartCounter()
            time.sleep(0.001)
            f_tdc.tdcEnable(False)
            counters = f_tdc.tdcGetCounterData()
            for _, tdc_ch in used_chan:
                scurves[tdc_ch].append(counters[tdc_ch])
            self.logger.info(f"{dac10b_val} / {2**10} {counters}")

        res={}
        res["state"]=state
        res["version"]=version
        res["feb"]=feb_id
        res["thmin"]=thmin
        res["thmax"]=thmax
        res["thstep"]=thstep
        res["asic"]=asic_name.upper()
        res["ctime"]=time.time()
        res["analysis"]="SCURVE_A"
        res["channels"]=[]
        for petiroc_chan, tdc_chan in used_chan:
            rc={}
            rc["prc"]=petiroc_chan
            rc["tdc"]=tdc_chan
            rc["scurve"]=scurves[tdc_chan]
            res["channels"].append(rc)
            plt.plot(range(thmin, thmax, thstep), scurves[tdc_chan], '+-', label=f"top right ch{petiroc_chan}")
        plt.grid()
        plt.legend(loc="upper right")
        plt.show() 
        # Store results in json
        res_dir='results/%s_%d_f_%d' % (state,version,feb_id)
        os.system("mkdir -p %s" % res_dir)
        fout=open("%s/scurves_all_channels_%s.json" % (res_dir,asic_name),"w")
        fout.write(json.dumps(res))
        fout.close
        return res


    except NameError as e:
        self.logger.info(f"Test failed with message: {e}")


def main(params):
    state=params.state
    version=params.version
    feb_id=params.feb
    all_channels=params.allchan
    one_by_one=params.one
    thi=params.thmin
    tha=params.thmax
    upload=params.upload
    comment =params.comment
    
    sdb=cra.instance()
    self.sdb.download_setup(state,version)
    self.sdb.setup.febs[0].petiroc.set_parameter("10b_dac_vth_discri_time",800)
    for i in range(16):
        self.sdb.setup.febs[0].fpga.set_pair_filtering_en(i,0)
    self.sdb.setup.version=998
    self.sdb.to_csv_files()

    self.logger.info(f" {self.sdb.setup.febs[0].fpga_version}, {self.sdb.setup.febs[0].petiroc_version}")
    lightdaq.configLogger(loglevel=logging.INFO)
    logger = logging.getLogger('CMS_IRPC_FEB_LightDAQ')
    try:
        ax7325b = lightdaq.AX7325BBoard()
        feb = lightdaq.FebV2Board(ax7325b, febid='FEB0', fpga_fw_ver='4.8')
        self.ax7325b.init(feb0=True, feb1=False)
        ### Test
        self.sdb.setup.febs[0].fpga_version='4.8'
        self.feb.init()
        
        self.feb.loadConfigFromCsv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (state,998,feb_id))
        asicl=["left_top","left_bot","middle_top","middle_bot","right_top","right_bot"]
        #asicl=["middle_bot"]
        for an in asicl:
            print(an)
            self.feb.loadConfigFromCsv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (state,998,feb_id))

            if (all_channels):
                results=make_pedestal_all_channels(state,version,feb_id,feb,an,thi,tha,1)
                self.sdb.upload_results(results["state"],results["version"],results["feb"],results["analysis"],results,comment)
                continue
            if (one_by_one):
                results=make_pedestal_one_by_one(state,version,feb_id,feb,an,thi,tha,1)
                self.sdb.upload_results(results["state"],results["version"],results["feb"],results["analysis"],results,comment)
                continue
            else:
                _,_,used_chan =get_mapping(an,feb)
                v6=self.sdb.setup.febs[0].petiroc.get_6b_dac(an)
                turn_on=[]
                for idx in range(16):
                    to=pedestal_one_channel(feb,an,idx,thi,tha,v6)
                    turn_on.append(to)
                self.logger.info(f" Turn ON {turn_on}")
                nto=np.array(turn_on)
                # Target
                target=round(np.median(nto))
                self.logger.info(f"Median target {target}")
                ## Check minimal gain value
                too_low=False
                too_high=False
                vexp=[]
                for idx in range(16):
                    petiroc_chan,tdc_ch =used_chan[idx]
                    gc=v6[ petiroc_chan]+round((target-turn_on[idx])/3.9)
                    vexp.append(gc)
                    too_low=too_low or (gc<3)
                    too_high=too_high or (gc>61)
                if (too_low):
                    target=target+10
                if (too_high):
                    target=target-10
                self.logger.info(f"Median target final {target} dac6b {vexp}")
                #val=input("Second round ? ")
                v6_cor=v6
                turn_on_cor=[]
                for idx in range(16):
                    petiroc_chan,tdc_ch =used_chan[idx]
                    gc=v6[ petiroc_chan]+round((target-turn_on[idx])/3.9)
                    td=[]
                    for ig in range(5):
                        g=gc-2+ig
                        vc=v6
                        if (g<=1):
                            g=1
                        if (g>=63):
                            g=63
                        vc[ petiroc_chan]=int(g)
                        
                        to=pedestal_one_channel(feb,an,idx,target-15,target+15,vc,two_steps=False)
                        td.append(to)
                    #print(gc)
                    #print(td)
                    # find best value
                    mindist=1000
                    imin=2
                    for ig in range(5):
                        if (abs(td[ig]-target)<mindist):
                            mindist=abs(td[ig]-target)
                            imin=ig
                    turn_on_cor.append(td[imin])
                    newdac=int(gc-2+imin)
                    if (newdac<1):
                        newdac=1
                    if (newdac>63):
                        newdac=63
                    v6_cor[petiroc_chan]=newdac
                self.logger.info(f" Turn ON {turn_on_cor}")
                ntoc=np.array(turn_on_cor)
                # Target
                target1=int(round(np.median(ntoc)))
                self.logger.info(f"Median target {target1}")
                self.logger.info(v6_cor)
                #v_stop=input()
                # Upload to DB
                for  ich in range(len(v6_cor)):
                   self.sdb.setup.febs[0].petiroc.set_6b_dac(ich,v6_cor[ich],an.upper())
                self.sdb.setup.febs[0].petiroc.set_parameter("10b_dac_vth_discri_time",target1,an.upper())
                results=make_pedestal_all_channels(state,version,feb_id,feb,an,thi,tha,1,v6=v6_cor)
                #val=input("Next ASIC ? ")
                
        
        if (upload):
            cm=comment
            self.sdb.setup.version=version
            self.sdb.upload_changes(cm)
        else:
            self.sdb.setup.version=999
            self.sdb.setup.to_csv_files()
                
                

    except NameError as e:
        self.logger.info(f"Test failed with message: {e}")

    
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    
    # configure all the actions
    grp_action = parser.add_mutually_exclusive_group()
    grp_action.add_argument('--allchan', dest='allchan', default=False, action='store_true',help="Make Scurve allchannels on")
    grp_action.add_argument('--one', dest='one', default=False, action='store_true',help="Make Scurve one by one")
    grp_action.add_argument('--upload', dest='upload', default=False, action='store_true',help="Upload new state to DB")
    # Arguments
    parser.add_argument('--state', action='store', type=str,default=None, dest='state', help='DB State')
    parser.add_argument('--version', action='store',type=int,default=None,dest='version',help='DB state version' )
    parser.add_argument('--feb', action='store', type=int,default=None, dest='feb', help='FEB id')
    parser.add_argument('--min', action='store', type=int,default=200, dest='thmin', help='Minimal 10b dac')
    parser.add_argument('--max', action='store', type=int,default=600, dest='thmax', help='Maximal 10b dac')
    
    parser.add_argument('--comment', action='store', type=str,default=None, dest='comment', help='Upload or test comment')

    results = parser.parse_args()
    print(results)

    if (results.state==None):
        print("--state should be specified")
        exit(0)
    if (results.version==None):
        print("--version should be specified")
        exit(0)
    if (results.feb==None):
        print("--feb should be specified")
        exit(0)

        
    state=results.state
    version=results.version
    feb_id=results.feb
    all_channels=results.allchan
    thi=results.thmin
    tha=results.thmax
    cm_up=None
    if (results.upload!=None):
        cm_up=results.upload
    #main(state,version,feb_id,all_channels,gbt_init,upload_comment=cm_up)
    main(results)
