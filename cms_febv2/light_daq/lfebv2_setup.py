

import threading
import time
import logging
import sys
import time

from pprint import pprint
import numpy as np 
from matplotlib import pyplot as plt

import cms_irpc_feb_lightdaq as lightdaq

import csv_register_access as cra
import FebWriter as FW

import os
import json
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/febv2debug%d.log" % os.getpid(), mode='w')  # ,
        # logging.StreamHandler()
    ]
)


class lfebv2_setup:
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
        self.last_paccomp=None
        self.last_delay_reset_trigger=None
        lightdaq.configLogger(loglevel=logging.INFO)
        self.logger = logging.getLogger('CMS_IRPC_FEB_LightDAQ')

    def init(self):
        """
        Initialise the setup. 
        It creates the access to the DB, access to FC7 and boot the FEB

        """
        self.sdb=cra.instance()
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        if "vth_shift" in self.params:
            self.sdb.setup.febs[0].petiroc.shift_10b_dac(self.params["vth_shift"])
        
        self.sdb.to_csv_files()
        lightdaq.configLogger(logging.INFO)

        try:

            self.ax7325b = lightdaq.AX7325BBoard()
            self.feb0 = lightdaq.FebV2Board(self.ax7325b, febid='FEB0', fpga_fw_ver='4.8')
            self.ax7325b.init(feb0=True, feb1=False)
            ### Test
            self.sdb.setup.febs[0].fpga_version='4.8'
            self.feb0.init()
            #fmc_mapping="dome"
            #if "mapping" in self.params["config"]:
            #    fmc_mapping=self.params["config"]["mapping"]
            #self.fc7.init(init_gbt=True,mapping=fmc_mapping)
            #self.feb.boot(app_fw=False)

        except NameError as e:
            print(f"Test failed with message: {e}")

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

    def configure(self):
        """ Configure the setup

        It configures the FEB and prepare the FC7 for a run setting the orbit and trigger definition
        """
        self.feb0.loadConfigFromCsv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (self.params["db_state"],self.params["db_version"],self.params["feb_id"]))
        #enableforces2=True
        if ("disable_force_s2" in self.params):
            enableforces2=not (self.params["disable_force_s2"]==1)
        for fpga in lightdaq.FPGA_ID:
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
        #self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=True, delay=100)
    
        self.ax7325b.fastbitResetBc0Id()
        #self.fc7.configure_resync_external(100)
        #self.fc7.reset_bc0_id()

        if ("trigger" in self.params):
            trg=self.params["trigger"]
            if "n_bc0" in trg:
                #self.fc7.configure_nBC0_trigger(trg["n_bc0"])
                self.ax7325b.triggerBc0Configure(int(trg["n_bc0"]) != 0 , trg["n_bc0"]) 
            #if "n_data" in trg:
            #    self.fc7.configure_ndata_trigger(trg["n_data"])

            if "external" in trg:
                #self.fc7.configure_external_trigger(trg["external"])
                self.ax7325b.triggerExternalConfigure(True, trg["external"])                
            #if "periodic" in trg:
            #    self.fc7.configure_periodic_trigger(trg["periodic"])

      
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
        #runHeaderWordList.append(int(self.ax7325b.ipbRead('GENERAL')) #[0]
        runHeaderWordList.append(int(0))                         
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
        lightdaq.configLogger(logging.WARN)
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
        acq_ctrl = lightdaq.BitField()
        acq_ctrl[30] = 1
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
        logging.debug(f"start_acquisition")
    def stop_acquisition(self):
        acq_ctrl = lightdaq.BitField()
        acq_ctrl[30] = 0
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
        logging.debug(f"stop_acquisition")

    def hasTrigger(self):
        acq_status = lightdaq.BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
        return (acq_status[31] == 0)
    
    def getNFrames(self):
        acq_status = lightdaq.BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
        istat=self.ax7325b.ipbRead('ACQ_STATUS')
        logging.debug(f"Status {acq_status:08b} {bin(istat)[2:6]}")
        try:
            assert acq_status[31] == 0, "Not trigged => no data!"
        except:
            logging.warning("Bit 31 set")
            return 0
        n = acq_status[15,0]
        #logging.info(f"Reading {n}+3 TDC frames")
        return n
    def readFrames(self,n):
        logging.info(f"Reading {n+3} TDC frames")
        try:
            rawdata = self.ax7325b.ipbReadBlock('FEBS_TDC_DATA_WORDS', (n+3)*8) # 3 tdcframes are stuck between ringbuffer and ipbreadout
        except:
            logging.error("cannot read block {(n+3)*8} ")

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
        for fpga in lightdaq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('standard')
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
            logging.debug(f"{(nb_frame32==0 or nb_last!=nb_frame32) and self.running}")
            while (nb_frame32==0 or nb_last!=nb_frame32) and self.running:
                if not self.hasTrigger():
                    continue
                nb_frame32 = self.getNFrames()
                logging.debug(f"Read  getNFrames {nb_frame32}")

                nb_last=nb_frame32
                if (nwait%1000==999):
                    print(nwait)
                nwait+=1
                if (nwait>1E4):
                    message= "Resetting BC0 and restart DAQ after number {}, event {}.".format(nacq, self.writer.eventNumber())
                    logging.error(message)
                    self.stop_acquisition()
                    time.sleep(10)
                    self.ax7325b.fastbitResetBc0Id()
                    self.start_acquisition()
                    nwait=0
                time.sleep(0.001)
            if not self.running:
                break
            logging.info(f"Found {nb_frame32}")
            while nb_frame32!=0:
                #ntdcf=0
                #for tdc_frame in self.fc7.uplink.receive_tdc_frames(nb_frame=nb_frame32*8, timeout=0.01):
                #    print(f"{nacq} {ntdcf} {tdc_frame.raw:032X} ", file=fout)
                #    ntdcf+=1
                #time.sleep(0.01)
                nb_frame32_1 = self.getNFrames()
                logging.debug(f"Found {nb_frame32_1}")

                #if (nb_frame32%8!=0 or nb_frame32_1< nb_frame32):
                if (nb_frame32_1< nb_frame32):
                    logging.info("oops not x 8 %d et le second %d \n" % (nb_frame32,nb_frame32_1))
                    time.sleep(5)
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
                    logging.error("error")
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
        for fpga in lightdaq.FPGA_ID:
            self.feb0.fpga[fpga].tdcEnable(False) 
        self.stop_acquisition()

        nacq= 0
        ntrig = 0
        lightdaq.configLogger(loglevel=logging.WARN)
        logger = logging.getLogger('FEB_minidaq')        
        lightdaq.configLogger(logging.INFO)
        self.writer.endRun()
        logging.info("Daq is stopped")

    
