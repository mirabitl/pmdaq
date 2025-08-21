

import threading
import time
import logging
import sys
import time

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

    def init(self):
        """
        Initialise the setup. 
        It creates the access to the DB, access to KC705 

        """
        self.sdb=cra.instance()
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        if "vth_shift" in self.params:
            target = self.sdb.setup.boards[0].picmic.get_dac_threshold()
            target=target+self.params["vth_shift"]
            self.sdb.setup.boards[0].picmic.set_dac_threshold(target)
        # Maximal filtering
        if "filtering" in self.params:
            if (self.params["filtering"]==1):
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
        if "falling" in self.params:
            self.sdb.setup.boards[0].picmic.set("falling_en", self.params("falling"))
        # ValEvt
        if "val_evt" in self.params:
            self.sdb.setup.boards[0].picmic.set("Forced_ValEvt", self.params("val_evt"))
        # Polarity
        if "pol_neg" in self.params:
            self.sdb.setup.boards[0].picmic.set("Polarity",self.params("pol_neg") )
        # DC_PA
        if "dc_pa" in self.params:
            dc_pa=self.params("dc_pa")
            for ch in range(64):
                if dc_pa != 0:
                    self.setup.boards[0].picmic.set("DC_PA_ch", dc_pa, ch)
        self.sdb.to_csv_files()
        daq.configLogger(logging.INFO)

        self.kc705 = daq.KC705Board()
        self.kc705.init()

        self.feb = daq.FebBoard(self.kc705)
        self.feb.init()
        self.feb.loadConfigFromCsv(
            folder="/dev/shm/board_csv",
            config_name="%s_%d_f_%d_config_picmic.csv" % (self.state, 888, self.feb_id),
        )
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
        self.feb.loadConfigFromCsv(
            folder="/dev/shm/board_csv",
            config_name="%s_%d_f_%d_config_picmic.csv" % (self.state, 888, self.feb_id),
        )
        self.feb.fpga.enableDownlinkFastControl()
        # disable all liroc channels
        for ch in range(64):
            self.feb.liroc.maskChannel(ch)
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
        self.sdb.to_csv_files()

   
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
        runHeaderWordList.append(int(self.fc7.fpga_registers.get_general_register())) #[0]
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

    def change_vth_shift(self,shift):
        """ Change the PETIROC VTH_TIME threshold
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            shift (int): Shift to add to VTH_TIME DAC10 bits taken in the DB
        """
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.sdb.setup.febs[0].petiroc.shift_10b_dac(shift)
        if (self.last_paccomp!=None):
            self.sdb.setup.febs[0].petiroc.set_parameter("pa_ccomp",self.last_paccomp&0XF,asic=None)
        if (self.last_delay_reset_trigger!=None):
            self.sdb.setup.febs[0].petiroc.set_parameter("delay_reset_trigger",self.last_delay_trigger&0XF,asic=None)

        self.sdb.to_csv_files()
    def change_paccomp(self,value):
        """ Change the PETIROC PACCOMP
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            value (int): PACCOMP to all asics
        """
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.sdb.setup.febs[0].petiroc.set_parameter("pa_ccomp",value&0XF,asic=None)
        self.last_paccomp=value
        self.sdb.to_csv_files()
    def change_delay_reset_trigger(self,value):
        """ Change the PETIROC delay_reset_trigger value
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            value (int): DELAY_RESET_TRIGGER VALUE
        """
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.sdb.setup.febs[0].petiroc.set_parameter("delay_reset_trigger",value&0XF,asic=None)
        self.last_delay_reset_trigger=value
        self.sdb.to_csv_files()
    

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
        acq_ctrl = BitField()
        acq_ctrl[30] = 1
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
    def stop_acquisition(self):
        acq_ctrl = BitField()
        acq_ctrl[30] = 0
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
    def getNFrames(self):
        acq_status = BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
        assert acq_status[31] == 0, "Not trigged => no data!"
        n = acq_status[15,0]
        self.logger.info(f"Reading {n}+3 TDC frames")
        return n
    def readFrames(self,n):
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
        #self.feb.enable_tdc(True)
        for fpga in daq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('trig_ext_resync')
            self.feb0.fpga[fpga].tdcEnable(True)
            self.feb0.fpga[fpga].tdcEnableChannel()       

        fout=open("debug.out","w")

        while (self.running):
            self.writer.newEvent()
            
            #self.fc7.configure_resync_external(2) 
            #self.fc7.reset_bc0_id()
            self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=False, delay=2)
    
            self.ax7325b.fastbitResetBc0Id()
            """ 
            self.fc7.configure_acquisition(buf_size=self.params["config"]["buf_size"],
                                           triggerless=(self.params["config"]["triggerless"]==1),
                                           single=(self.params["config"]["single"]==1),
                                           keep=(self.params["config"]["keep"]==1),
                                           external_window=(self.params["config"]["external_window"]==1))
            """
            self.start_acquisition()
            nbt=0
            datawait=0
            nb_frame32=0
            nb_last=0
            nwait=0
            while (nb_frame32==0 or nb_last!=nb_frame32) and self.running:
                nb_frame32 = self.getNFrames()
                nb_last=nb_frame32
                if (nwait%1000==999):
                    print(nwait)
                nwait+=1
                if (nwait>5E4):
                    message= "Resetting BC0 and restart DAQ after number {}, event {}.".format(nacq, self.writer.eventNumber())
                    logging.error(message)
                    self.stop_acquisition()
                    time.sleep(10)
                    self.fc7.reset_bc0_id()
                    self.start_acquisition()
                    nwait=0
                time.sleep(0.001)
            if not self.running:
                break
            #print(f"Found {nb_frame32}")
            while nb_frame32!=0:
                #ntdcf=0
                #for tdc_frame in self.fc7.uplink.receive_tdc_frames(nb_frame=nb_frame32*8, timeout=0.01):
                #    print(f"{nacq} {ntdcf} {tdc_frame.raw:032X} ", file=fout)
                #    ntdcf+=1
                #time.sleep(0.01)
                nb_frame32_1 = self.getNFrames()
                if (nb_frame32%8!=0 or nb_frame32_1< nb_frame32):
                    print("oops not x 8 %d et le second %d \n" % (nb_frame32,nb_frame32_1))
                    nb_frame32 = self.getNFrames()
                    continue
                else:
                    nb_frame32=nb_frame32_1

                try:
                    message= "Nb frames to be block read {}".format(nb_frame32)
                    logging.info(message)
                    nb_frame32 = min(4096+2048, nb_frame32)
                    #print("frames :%d %d\n"% (nb_frame32//8,nb_frame32))
                    #sys.stdout.flush()
                    datas=self.readFrames(nb_frame32)
                except:
                    print("error")
                #self.fc7.fpga_registers.ipbus_device.ReadBlock("USER_OUTPUT_FIFO_TDC", nb_frame32)
                # for i in range(0,len(datas),8):
                #     fr="%d %d " % (nacq,ntdcf)
                #     for j in range(8):
                #         fr=fr+"%.8x" % datas[i+j]  
                #     print(fr,file=fout)
                #     ntdcf+=1
                self.writer.appendEventData(datas)
                #print(f"{nb_frame32} read",flush=True)
                nbt+=nb_frame32
                #time.sleep(0.01)
                nb_frame32 = self.getNFrames()
            if nbt == 0:
                message= "problem at fifo readout number {}, event written {}.".format(nacq, self.writer.eventNumber())
                logging.error(message)
                time.sleep(0.01)
            else:
                nacq+=1
                if not self.dummy:
                    self.writer.writeEvent()
           
            if (nacq % 1 == 0):
                message= "Info : Event {} (acquisition number {}) have read {} words = {} potential TDC frames.".format(self.writer.eventNumber(), nacq, nbt, nbt/8)
                logging.info(message)

                #with open("/data/trigger_count.txt", "w") as trig_output:
                #    trig_output.write(str(self.writer.eventNumber()))             
            self.stop_acquisition()
            time.sleep(0.005)
        logging.info("Thread %d: finishing", self.run)
    def stop(self):
        """ Stop the run

        It sets running to false, wait for the thread to stop and disable FEB and FC7  acquisition
        """
        self.running= False
        self.producer_thread.join()
        self.stop_acquisition()
        self.feb.enable_tdc(False)
        acq_ctrl = BitField()
        acq_ctrl[30] = 0
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
    def acquiring_data(self):
        """ Acquisition thread

        While running, it loops continously and spy data in the FC7 readout fifo
        It writes data to disk or shared memory until running is false and the run stopped. 
        """
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

    
