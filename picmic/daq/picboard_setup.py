

import threading
import time
import logging
import sys
import time
import copy
from pprint import pprint
import numpy as np 
from matplotlib import pyplot as plt

import liroc_ptdc_daq as daq
import picmic_register_access as cra
import BoardWriter as FW

import os
import json
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/picmicdebug%d.log" % os.getpid(), mode='w')  # ,
        # logging.StreamHandler()
    ]
)


class picboard_setup:
    """The Light FEBV2 setup interface class.
    It hides access to MINIDAQ driver classes and implements basics function as
    init,configure,start,stop ....
    """
    def __init__(self, config,verbose=False):
        """
        Object creation

        Args:
            config: a python object conatining the daq configuration
            verbose (bool): False by default, tune to true for debug printout
        """
        self.params=config
        self.verbose=verbose
        self.run=0
        self.detectorId = config["writer"]["detector_id"]
        self.sourceId = config["writer"]["source_id"]
        self.running = False
        self.dummy=False
        self.writer=None
        daq.configLogger(loglevel=logging.INFO)
        self.logger = logging.getLogger('PICBOARD_DAQ')
        # Effective parameters
        self.patches={"dac_threshold_shift":None,"filtering":None,"falling":None,"val_evt":None,"pol_neg":None,"dc_pa":None}
        if "dac_threshold_shift" in self.params:
            self.patches["dac_threshold_shift"]=self.params["dac_threshold_shift"]
        if "filtering" in self.params:
            self.patches["filtering"]=self.params["filtering"]
        if "falling" in self.params:
            self.patches["falling"]=self.params["falling"]
        if "val_evt" in self.params:
            self.patches["val_evt"]=self.params["val_evt"]
        if "pol_neg" in self.params:
            self.patches["pol_neg"]=self.params["pol_neg"]
        if "dc_pa" in self.params:
            self.patches["dc_pa"]=self.params["dc_pa"]

    def patch_db(self,state=None,version=0):
        """
        Patch DB version and  build csv file with dummy_version number if not 0
        """
        if self.patches["dac_threshold_shift"] != None:
            #always shift from db setting
            target = self.db_registers.picmic.get_dac_threshold()
            target=target+self.patches["dac_threshold_shift"]
            self.registers.picmic.set_dac_threshold(target)
        # Maximal filtering
        if self.patches["filtering"] != None:
            if (self.patches["filtering"]==1):
                self.registers.picmic.set("hrx_top_delay", 0xF)
                self.registers.picmic.set("hrx_top_bias", 0xF)
                self.registers.picmic.set("hrx_top_filter_trailing", 1)
                self.registers.picmic.set("hrx_top_filter_leading", 1)
                self.registers.picmic.set("hrx_bot_delay", 0xF)
                self.registers.picmic.set("hrx_bot_bias", 0xF)
                self.registers.picmic.set("hrx_bot_filter_trailing", 1)
                self.registers.picmic.set("hrx_bot_filter_leading", 1)
            else:
                self.registers.picmic.set("hrx_top_filter_leading", 0)
                self.registers.picmic.set("hrx_bot_filter_leading", 0)
                self.registers.picmic.set("hrx_top_filter_trailing", 0)
                self.registers.picmic.set("hrx_bot_filter_trailing", 0)
        # Falling ?
        if self.patches["falling"] != None:
            self.registers.picmic.set("falling_en", self.patches("falling"))
        # ValEvt
        if self.patches["val_evt"] != None:
            self.registers.picmic.set("Forced_ValEvt", self.patches("val_evt"))
        # Polarity
        if self.patches["pol_neg"] != None:
            self.registers.picmic.set("Polarity",self.patches("pol_neg") )
        # DC_PA
        if self.patches["dc_pa"] != None:
            dc_pa=self.patches("dc_pa")
            for ch in range(64):
                if dc_pa != 0:
                    self.registers.picmic.set("DC_PA_ch", dc_pa, ch)
        if version > 0:
            # Patched version from config file
            self.registers.to_csv_files(state,version)

    def loadConfigInBoard(self,state,version):
        self.feb.loadConfigFromCsv(
            folder="/dev/shm/board_csv",
            config_name="%s_%d_f_%d_config_picmic.csv" % (state, version, self.feb_id),
        )
    def init(self):
        """
        Initialise the setup. 
        It creates the access to the DB, access to KC705 

        """
        self.sdb=cra.instance()
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.db_registers=self.sdb.setup.boards[0]
        self.registers=copy.deepcopy(self.sdb.setup.boards[0])
        # Bare version
        self.db_registers.to_csv_files(self.params["db_state"],self.params["db_version"])
        # Now patch the registers from the config
        self.patch_db(self.params["db_state"],777)
        
        
        daq.configLogger(logging.INFO)

        self.kc705 = daq.KC705Board()
        self.kc705.init()

        self.feb = daq.FebBoard(self.kc705)
        self.feb.init()
        self.loadConfigInBoard(self.params["db_state"],777)
        
        self.feb.fpga.enableDownlinkFastControl()
        # disable all liroc channels
        for ch in range(64):
            self.feb.liroc.maskChannel(ch)
        self.feb.ptdc.setResMode("fine")

        self.feb.ptdc.powerup()
    def configure(self):
        """ Configure the setup

        It configures the Board and prepare the KC705 for a run setting the orbit and trigger definition
        """
        self.loadConfigInBoard(self.params["db_state"],777)
        
        self.feb.fpga.enableDownlinkFastControl()
        # disable all liroc channels
        # for ch in range(64):
        #     self.feb.liroc.maskChannel(ch)
        self.feb.ptdc.setResMode("fine")

        self.feb.ptdc.powerup()

        self.kc705.fastbitConfigure(
            mode="normal",
            flush_delay=100,
            dig0_edge="rising",
            dig0_delay=0,
            dig1_edge="rising",
            dig1_delay=0,
        )
        # generate a few windows to flush out the agilient patterns
        self.kc705.acqSetWindow(1, 1)
        for _ in range(5):
            self.kc705.ipbWrite("ACQ_CTRL_2", 1)
            self.kc705.ipbWrite("ACQ_CTRL_2", 0)
        """
        La configuration des fenetres et des orbits est a redefinir
        #enableforces2=True
        if ("disable_force_s2" in self.params):
            enableforces2=not (self.params["disable_force_s2"]==1)
        for fpga in daq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('trig_ext_resync')
            self.feb0.fpga[fpga].tdcEnable(False)       

        
        self.ax7325b.fastbitFsmConfigure(
            s0_duration=self.params["orbit_fsm"]["s0"],
            s1_duration=self.params["orbit_fsm"]["s1"],
            s2_duration=self.params["orbit_fsm"]["s2"],
            s3_duration=self.params["orbit_fsm"]["s3"],
            s4_duration=self.params["orbit_fsm"]["s4"],
            enable_force_s2=enableforces2)

        self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=False, delay=100)
    
        self.ax7325b.fastbitResetBc0Id()
        #self.fc7.configure_resync_external(100)
        #self.fc7.reset_bc0_id()

        if ("trigger" in self.params):
            trg=self.params["trigger"]
            if "n_bc0" in trg:
                #self.fc7.configure_nBC0_trigger(trg["n_bc0"])
                self.ax7325b.triggerBc0Configure(True, trg["n_bc0"])
            #if "n_data" in trg:
            #    self.fc7.configure_ndata_trigger(trg["n_data"])

            if "external" in trg:
                #self.fc7.configure_external_trigger(trg["external"])
                self.ax7325b.triggerExternalConfigure(True, trg["external"])                
            #if "periodic" in trg:
            #    self.fc7.configure_periodic_trigger(trg["periodic"])

        """  
    def change_db(self,state_name,version):
        """ Download and store a new DB version
            The FPGA/PETIROC parameters to be used are stored but not load (configure needed)
        Args:
            state_name (str): name of the state
            version (int)" version number
        """
        self.params["db_state"]=state_name
        self.params["db_version"]=version
        
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.db_registers=self.sdb.setup.boards[0]
        self.registers=copy.deepcopy(self.sdb.setup.boards[0])
        # Bare version
        self.db_registers.to_csv_files(self.params["db_state"],self.params["db_version"])

   
    def setWriter(self,fw):
        """ Set the FEBWriter object to be used

        Args:
            fw: The FebWriter object
        """
        self.writer=fw
    def writeRunHeader(self):
        """  Create and write the run header using the FebWriter object

        """
        if (self.writer==None):
            logging.fatal("no writer defined")
            return
        runHeaderWordList=[]
        runHeaderWordList.append(int(0)) #[0]
        # self.feb.feb_ctrl_and_status.get_temperature()
        # temperatures=self.feb.feb_ctrl_and_status.temperature_value
        # temperatures_int_values=[]
        # for i in range(1,6):
        #     key = "LM75_SENSOR"+str(i)
        #     temperatures_int_values.append(int(2*temperatures[key][0])) # this should be a 9 bit word
        # runHeaderWordList.append(( (temperatures_int_values[0]<<18)+(temperatures_int_values[1]<<9)+temperatures_int_values[2])) #[5]
        # runHeaderWordList.append(( (temperatures_int_values[3]<<9)+temperatures_int_values[4]))  #[6]
        # #reading back channel enabled word for TDC
        # message="Information on potential enabled channel : {}".format(chanset)
        #logging.info(message)
        message="runHeader Word List : {}".format(runHeaderWordList)
        logging.debug(message)
        self.writer.writeRunHeader(runHeaderWordList)

    def change_params(self,pname,pval):
        """ Change the PETIROC VTH_TIME threshold
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            shift (int): Shift to add to VTH_TIME DAC10 bits taken in the DB
        """
        if not pname in self.patches:
            return
        
        self.patches[pname]=pval
        self.patch_db(self.params["db_state"],777)
        

    def start(self,run=0):
        """ Start a run.

        It launches a thread that spy the data in the readout FIFO of the FEB
        If run is different from 0, it creates a new run (file) and writes the run header
        Args:
            run (int): By default it's 0 and shared memory is used. 
            
        """
        if (self.writer==None):
            logging.fatal("no writer defined")
            return
        daq.configLogger(logging.WARN)
        self.writer.setIds(self.detectorId, self.sourceId)
        if (run != 0):
            self.run= run
            self.writer.newRun(self.run)
            logging.info("Data are written in file %s." %
                       self.writer.file_name())
            self.writeRunHeader()
        self.running= True
        self.producer_thread = threading.Thread(target=self.acquiring_data)
        self.producer_thread.start()
        logging.info("Daq is started")
        message = "Run number : "+str(self.run)
        logging.info(message)

    def status(self):
        """ returns the status of the acquisition
        Returns:
            A dictionnary object with status and event number
        """
        r={}
        if (self.writer!=None):
            r["state"]="running"
            r["event"]=self.writer.eventNumber()
        else:
            r["state"]="not_running"
            r["event"]=-1
        return r
    def start_acquisition(self):
        self.kc705.acqSetWindow(1, 1)
        self.kc705.ipbWrite("ACQ_CTRL_2", 1)
    def stop_acquisition(self):
        self.kc705.acqSetWindow(1, 1)
        self.kc705.ipbWrite("ACQ_CTRL_2", 0)
    def getNFrames(self):
        status1 = daq.BitField(self.kc705.ipbRead('ACQ_STATUS_1'))
        status2 = daq.BitField(self.kc705.ipbRead('ACQ_STATUS_2'))
        nwords = []
        nwords.append(status1[31,16])
        nwords.append(status1[15,0])
        nwords.append(status2[31,16])
        nwords.append(status2[15,0])
        return nwords
    def readFrames(self):
        ptdc_words = []
        status1 = daq.BitField(self.ipbRead('ACQ_STATUS_1'))
        status2 = daq.BitField(self.ipbRead('ACQ_STATUS_2'))
        nwords = []
        nwords.append(status1[31,16])
        nwords.append(status1[15,0])
        nwords.append(status2[31,16])
        nwords.append(status2[15,0])

        for p in range(4):
            if nwords[p]:
                ptdc_words.append(self.kc705.ipbReadBlock(f'FEB0_PTDC_PORT{p}_DATA', nwords[p]))
            else:
                ptdc_words.append([])

        return ptdc_words

        self.logger.info(f"Reading {n+3} TDC frames")
        rawdata = self.ipbReadBlock('FEBS_TDC_DATA_WORDS', (n+3)*8) # 3 tdcframes are stuck between ringbuffer and ipbreadout
        return rawdata
    def acquiring_data(self):
        """ Acquisition thread

        While running, it loops continously and spy data in the FC7 readout fifo
        It writes data to disk or shared memory until running is false and the run stopped. 
        """
        nacq= 0
        ntrig = 0
        self.logger.setLevel(logging.WARN)
        fout=open("debug.out","w")

        while (self.running):
            self.writer.newEvent()
            datas = self.kc705.acqPtdc(window=self.params["window"], deadtime=self.params["deadtime"], window_number=self.params["nbwindows"])
            
            self.writer.appendEventData(datas)
            if len(datas) == 0:
                message= "problem at fifo readout number {}, event written {}.".format(nacq, self.writer.eventNumber())
                logging.error(message)
                time.sleep(0.01)
            else:
                nacq+=1
                if not self.dummy:
                    self.writer.writeEvent()
           
            if (nacq % 1 == 0):
                message= "Info : Event {} (acquisition number {}) have read {} words.".format(self.writer.eventNumber(), nacq,len(datas) )
                logging.info(message)

                #with open("/data/trigger_count.txt", "w") as trig_output:
                #    trig_output.write(str(self.writer.eventNumber()))             
            time.sleep(0.005)
        logging.info("Thread %d: finishing", self.run)
    def stop(self):
        """ Stop the run

        It sets running to false, wait for the thread to stop and disable FEB and FC7  acquisition
        """
        self.running= False
        self.producer_thread.join()
        self.stop_acquisition()
        nacq= 0
        ntrig = 0
        daq.configLogger(loglevel=logging.WARN)
        logger = logging.getLogger('FEB_minidaq')
        logger.setLevel(logging.WARN)
        #self.feb.enable_tdc(True)
        for fpga in daq.FPGA_ID:
            self.feb0.fpga[fpga].tdcEnable(False)

        daq.configLogger(logging.INFO)
        self.writer.endRun()
        logging.info("Daq is stopped")

    
