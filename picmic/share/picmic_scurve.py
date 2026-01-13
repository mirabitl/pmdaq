import logging
import sys
import threading
import time
import matplotlib.pyplot as plt
import liroc_ptdc_daq as daq
import numpy as np 
import picmic_register_access as cra
import os
import json
import queue
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/daq.log", mode='w')  # ,
        # logging.StreamHandler()
    ]
)

class scurve_processor:
    def __init__(self,params):
        #self.queue=None
        resmode=None
        if "mode" in params:
            resmode=params["mode"]
        self.pb=picmic_scurve(params["db"]["board"],
                              params["db"]["state"],
                              params["db"]["version"],
                              dc_pa=params["dc_pa"],
                              res_mode=resmode)
                       
        self.res={}
        self.res["state"]=params["db"]["state"]
        self.res["version"]=params["db"]["version"]
        self.res["feb"]=params["db"]["board"]
        self.res["thmin"]=params["thmin"]
        self.res["thmax"]=params["thmax"]
        self.res["dcpa"]=params["dc_pa"]
        self.res["thstep"]=params["thstep"]
        self.res["asic"]="LIROC"
        self.res["ctime"]=time.time()
        if "mode" in params:
            self.res["mode"]=params["mode"]
        if "location" in params:
            self.res["location"]=params["location"]

        self.conf=params
        self.logger=logging.getLogger(__name__)
        self._thread = None
        self._running = threading.Event()
        self._lock = threading.Lock()

    def start_align(self,params=None):
        if not self._running.is_set():
            self.logger.info("running lock is cleared") 
        if self._thread:
            self._thread.join(timeout=10)
        if self._thread and self._thread.is_alive():
            self.logger.warning("Calibration déjà en cours")
            return False
        self._running.set()
        self._thread = threading.Thread(target=self.align, args=(params,), daemon=True)
        self._thread.start()
        return True
        
    def align(self,params=None):
        target,v_dac_local=self.pb.calib_iterative_dac_local(self.conf["thmin"],self.conf["thmax"])
        print(target)
        print(v_dac_local)
        # Update the DB
        for  ich in range(len(v_dac_local)):
            self.pb.sdb.setup.boards[0].picmic.set("DAC_local_ch",v_dac_local[ich],ich)
            print(f"DAC_local_ch{ich}",v_dac_local[ich])
        tlsb=target&0xFF
        tmsb=(target>>8)&0xFF
        self.pb.sdb.setup.boards[0].picmic.set("dac_threshold_lsb",tlsb)
        self.pb.sdb.setup.boards[0].picmic.set("dac_threshold_msb",tmsb)
        #results=make_pedestal_all_channels(state,version,feb_id,feb,an,thi,tha,1,v6=v6_cor)
        #val=input("Next ASIC ? ")
            
        if "location" in self.conf and "comment" in self.conf:
            self.pb.sdb.setup.version=self.conf["db"]["version"]
            self.pb.sdb.upload_changes(self.conf["comment"])
        else:
            self.pb.sdb.setup.version=999
            self.pb.sdb.setup.to_csv_files()
                
        
    def start_scurves(self,params={"analysis":"SCURVE_A","plot_fig":None}):
        if not self._running.is_set():
            self.logger.info("running lock is cleared") 
        if self._thread:
            self._thread.join(timeout=10)
        if self._thread and self._thread.is_alive():
            self.logger.warning("Calibration déjà en cours")
            return False
        self._running.set()
        self._thread = threading.Thread(target=self.get_scurves, args=(params,), daemon=True)
        self._thread.start()
        return True            
    #def get_scurves(self,analysis="SCURVE_A",plot_fig=None):
    def get_scurves(self,params):
        # Now get a run id
        analysis=params.get("analysis","SCURVE_A")
        plot_fig=params.get("plot_fig",None)
        self.logger.info(f"Plot fig set to {plot_fig}")
        self.res["analysis"]=analysis
        self.res["channels"]=[]
        scurves=None

        # Plots
        ax=None
        if plot_fig!=None:
            # Efface l'ancienne figure
            plot_fig.clear()
            ax=plot_fig.add_subplot(111)
        # Check the analysis
        if analysis == "SCURVE_A":
            print(f'start={self.conf["thmin"]},stop={self.conf["thmax"]},step={self.conf["thstep"]},dac_loc=0')
            #input()
            scurves=self.pb.scurve_all_channels(start=self.conf["thmin"],stop=self.conf["thmax"],step=self.conf["thstep"],dac_loc=0)
            print(scurves)
            #input()
        elif analysis == "SCURVE_1":
            scurves=self.pb.scurve_loop_one(start=self.conf["thmin"],stop=self.conf["thmax"],step=self.conf["thstep"],dac_loc=0)
        else:
            return False
        for liroc_chan in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN.keys():
            rc={}
            rc["prc"]=liroc_chan
            rc["tdc"]=daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[liroc_chan]
            rc["scurve"]=scurves[daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[liroc_chan]]
            self.res["channels"].append(rc)
            if plot_fig==None:
                plt.plot(range(self.conf["thmin"], self.conf["thmax"],1), scurves[liroc_chan], '+-', label=f"ch{liroc_chan}")
            else:
               ax.plot(
                   range(self.conf["thmin"], self.conf["thmax"], 1),
                   scurves[liroc_chan],
                   '+-',
                   label=f"ch{liroc_chan}"
               ) 
        if plot_fig==None:
            plt.grid()
            plt.legend(loc="upper right")
            plt.show()
        else:
            ax.grid()
            ax.legend(loc="upper right")
            self.logger.info(f"Plot fig set to {plot_fig}")
            # Envoyer un message pour mettre à jour le plot dans le thread principal
            if hasattr(self, 'queue'):
                self.queue.put("update_plot")
        # Now get a run id
        runid=None
        if "location" in self.conf and "comment" in self.conf:
            runobj=self.pb.sdb.getRun(self.conf["location"],self.conf["comment"])
            runid=runobj['run']
                                      
        # Store results in json
        res_dir='/tmp/results/%s_%d_f_%d' % (self.conf["db"]["state"],self.conf["db"]["version"],self.conf["db"]["board"])
        if runid==None:
            runid=int(input("Enter a run number: "))
        os.system("mkdir -p %s" % res_dir)
        fout=open(f"{res_dir}/scurves_all_channels_{runid}.json","w")
        fout.write(json.dumps(self.res))
        fout.close()
        
        if "location" in self.conf and "comment" in self.conf:
            self.pb.sdb.upload_results(runid,self.conf["location"],self.res["state"],self.res["version"],self.res["feb"],self.res["analysis"],self.res,self.conf["comment"])
        return True                              
        
            
class picmic_scurve:

    def __init__(self,id,state,version,filtering=True,falling=False,val_evt=False,pol_neg=False,dc_pa=0,res_mode=None):
        daq.configLogger(logging.DEBUG)

        self.feb_id=id
        self.state=state
        self.version=version
        self.sdb=cra.instance()
        self.sdb.download_setup(state,version)
        self.sdb.to_csv_files()
        target=800
        tlsb=target&0xFF
        tmsb=(target>>8)&0xFF
        self.sdb.setup.boards[0].picmic.set("dac_threshold_lsb",tlsb)
        self.sdb.setup.boards[0].picmic.set("dac_threshold_msb",tmsb)
        #Maximal filtering
        if (filtering):
            self.sdb.setup.boards[0].picmic.set("hrx_top_delay",0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_top_bias",0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_trailing",1)
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_leading",1)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_delay",0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_bias",0xF)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_trailing",1)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_leading",1)
        else:
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_leading",0)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_leading",0)
            self.sdb.setup.boards[0].picmic.set("hrx_top_filter_trailing",0)
            self.sdb.setup.boards[0].picmic.set("hrx_bot_filter_trailing",0)
        # Falling ?
        self.sdb.setup.boards[0].picmic.set("falling_en",0)
        # ValEvt
        self.sdb.setup.boards[0].picmic.set("Forced_ValEvt",0)
        # Polarity
        self.sdb.setup.boards[0].picmic.set("Polarity",0)
        # DC_PA
        for ch in range(64):
            if (dc_pa!=0):
                self.sdb.setup.boards[0].picmic.set('DC_PA_ch',dc_pa,ch)
        self.sdb.setup.version=998
        self.sdb.to_csv_files()

        
        print(self.sdb.setup.boards[0].picmic_version)
    
        
        

        self.kc705 = daq.KC705Board()
        self.kc705.init()
    
        self.feb = daq.FebBoard(self.kc705)
        self.feb.init()
        self.feb.loadConfigFromCsv(folder='/dev/shm/board_csv', config_name='%s_%d_f_%d_config_picmic.csv' % (self.state,998,self.feb_id))
        self.feb.fpga.enableDownlinkFastControl()
        # disable all liroc channels
        for ch in range(64): self.feb.liroc.maskChannel(ch)
        if res_mode!=None:
            self.feb.ptdc.setResMode(res_mode)

        self.feb.ptdc.powerup()
        
        # self.kc705.fastbitConfigure(mode='normal', flush_delay=100,
            # dig0_edge='rising', dig0_delay=0, 
            # dig1_edge='rising', dig1_delay=0)
        self.kc705.fastbitConfigure(mode='normal',
        dig2_edge='rising', dig2_delay=0 )
        # generate a few windows to flush out the agilient patterns
        # generate a few windows to flush out the agilient patterns
        self.kc705.acqSetWindow(1, 1)
        for _ in range(5): 
            self.kc705.ipbWrite('ACQ_CTRL.window_start', 1)
            self.kc705.ipbWrite('ACQ_CTRL.window_start', 0)

            
    def sweep_dac10b(self,start,stop,step):
        scurves = [[] for _ in range(64)]
        for dac10b_val in range(start,stop,step):
            self.feb.liroc.set10bDac(dac10b_val)
            self.feb.liroc.stopScClock()
            counters=self.kc705.acqPtdcCounters(window=5, deadtime=500, window_number=1000)
            for ch, counter in enumerate(counters):
                scurves[ch].append(counter)
        return scurves


    def scurve_single_chan(self,lichan,start=450,stop=750,step=1,dac_loc=32):
        for ch in range(64):
            if (dac_loc!=0):
                self.feb.liroc.setChannelDac(ch, dac_loc)
            self.feb.liroc.maskChannel(ch, ch != lichan)
            #self.feb.liroc.maskChannel(ch,False)
            #self.feb.liroc.setParam(f'DC_PA_ch{ch}',2)
            """
            if ch in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN:
                if ch!=lichan:
                    self.feb.ptdc.setParam(f'channel_enable_{daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[ch]}',0)
                else:
                    self.feb.ptdc.setParam(f'channel_enable_{daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[ch]}',1)
            """
        scurve = self.sweep_dac10b(start, stop, step)[daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[lichan]]
        return scurve

    def scurve_all_channels(self,start=450,stop=750,step=1,dac_loc=32):
        for ch in range(64):
            if dac_loc!=0 : self.feb.liroc.setChannelDac(ch, dac_loc)
            if ch>-1:
                self.feb.liroc.maskChannel(ch,False)
            else:
                self.feb.liroc.maskChannel(ch,True)
        scurves = self.sweep_dac10b(start, stop, step)
        return scurves
    def scurve_loop_one(self,start=450,stop=750,step=1,dac_loc=32):
        scurves = [[] for _ in range(64)]
        for ch in range(64):
            if ch not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN: continue
            scurves[ch]=self.scurve_single_chan(ch,start,stop,step,dac_loc)
            print(f"Channel {ch} {scurves[ch]}")
        return scurves
    def scurve_one_channel(self,index,thmin,thmax,thstep=1,dac_loc=64):
        
        # First round thstep=5
        
        scurve2=self.scurve_single_chan(index,start=thmin,stop=thmax,step=thstep,dac_loc=dac_loc)
        scurve2_th=[i for i in range(thmin,thmax,thstep)]
 
        # Now find the transition
        to_0=thmin
        cmax1=max(scurve2)
        cmax_index = scurve2.index(max(scurve2))
        to_1i=to_0
        for t in range(cmax_index,len(scurve2)):
            if (scurve2[t]<0.7*cmax1):
                to_1i=scurve2_th[t]
                break
        to_1a=to_1i
        for t in range(cmax_index,len(scurve2)):
            if (scurve2[t]<0.3*cmax1):
                to_1a=scurve2_th[t]
                break
        to_1=(to_1a+to_1i)//2
        print(f"Seuil raw {to_0}")
        print(f"Scan {thmin}-{thmax}  {scurve2}")
        print(f"Seuil fin {to_1}")
        #val=input("Next channel? ")
        return to_1

    def calib_dac_local(self,thi,tha):
        turn_on=[0 for i in range(64)]
        v6=[0 for i in range(64)]
        for idx in range(64):
            v6[idx]=self.feb.liroc.getParam(f'DAC_local_ch{idx}')
            if idx not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN: continue
            to=self.scurve_one_channel(idx,thi,tha,dac_local=0)
            turn_on[idx]=to
        print(f" Turn ON {turn_on}")
        nto=np.array(turn_on)
        # Target
        target=round(np.median(nto))
        print(f"Median target {target}")
        ## Check minimal gain value
        too_low=False
        too_high=False
        vexp=[v6[i] for i in range(64)]
        for idx in range(64):
            gc=v6[idx]
            if (turn_on[idx]!=0):
                gc=v6[idx]+round((target-turn_on[idx])/2.0)
                vexp[idx]=max(1,min(127,gc))
            else:
                continue
            too_low=too_low or (gc<3)
            too_high=too_high or (gc>1020)
        if (too_low):
            target=target+10
        if (too_high):
            target=target-10
        print(f"Median target final {target} \n v6 {v6} \n vexp {vexp}")
        return target,vexp
    def calib_iterative_dac_local(self,thi,tha):
        turn_on=[0 for i in range(64)]
        v6=[0 for i in range(64)]
        
        for idx in range(64):
            v6[idx]=self.feb.liroc.getParam(f'DAC_local_ch{idx}')
            if idx not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN: continue
            # First step =5
            to_0=self.scurve_one_channel(idx,thi,tha,thstep=5,dac_loc=0)
            ti0=round(max(to_0-15,thi))
            ta0=round(min(to_0+15,tha))
            to_1=self.scurve_one_channel(idx,ti0,ta0,thstep=1,dac_loc=0)
            turn_on[idx]=to_1
        print(f" Turn ON {turn_on}")
        nto=np.array(turn_on)
        # Target
        target=round(np.median(nto))
        print(f"Median target {target}")
        ## Check minimal gain value
        too_low=False
        too_high=False
        vexp=[v6[i] for i in range(64)]
        for idx in range(64):
            gc=v6[idx]
            if (turn_on[idx]!=0):
                gc=v6[idx]+round((target-turn_on[idx])/2.0)
                vexp[idx]=max(1,min(127,gc))
            else:
                continue
            too_low=too_low or (gc<3)
            too_high=too_high or (gc>127)
        if (too_low):
            target=target+10
        if (too_high):
            target=target-10
        print(f"Median target final {target} \n v6 {v6} \n vexp {vexp}")
        for idx in range(64):
            if idx not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN: continue
            dac_map={}
            """
            for gg in range(max(1,vexp[idx]-10),min(128,vexp[idx]+10)):
                self.feb.liroc.setParam(f'DAC_local_ch{idx}',gg)  
                to_fine=self.scurve_one_channel(idx,target-35,target+35,thstep=1,dac_loc=0)
                print(f"c {idx} TO {to_fine} {target-to_fine}")
                dac_map[gg]=abs(target-to_fine)
                
                print(f"\t Channel {idx} target {target} dac {gg} turn on {to_fine} diff {dac_map[gg]}")
            """
            gg=vexp[idx]
            for _ in range(10):
                self.feb.liroc.setParam(f'DAC_local_ch{idx}',gg)  
                to_fine=self.scurve_one_channel(idx,target-35,target+35,thstep=1,dac_loc=0)
                print(f"DAC {gg} c {idx} TO {to_fine} {target-to_fine}")
                dac_map[gg]=abs(target-to_fine)
                ngg=gg+round((target-to_fine)/2.0)
                if ngg<3: ngg=3
                if ngg>125: ngg=125 
                if abs(ngg-gg)<1: break
                gg=ngg   
            print(f"Channel scan {idx} -> {dac_map}")
            op_dac=min(dac_map, key = dac_map.get)
            print(f"Channel scan {idx} -> {op_dac}")

            vexp[idx]=op_dac
        return target,vexp
