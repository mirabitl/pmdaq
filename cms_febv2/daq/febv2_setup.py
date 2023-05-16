

import threading
import time
import logging
import sys
import time

from pprint import pprint
import numpy as np 
from matplotlib import pyplot as plt

from minidaq.fc7 import fc7_board
from minidaq.feb import feb_v2_cycloneV
from minidaq.tools import *
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


class febv2_setup:
    def __init__(self, config,verbose=False):
        self.params=config
        self.verbose=verbose
        self.run=0
        self.detectorId = config["writer"]["detector_id"]
        self.sourceId = config["writer"]["source_id"]
        self.running = False
        self.dummy=False
        self.writer=None
    def init(self):
        """
        Initialise the setup

        :param usefc7: trigger the initialisiation of fc7
        :param use_auto_reset_asic: Use reset of asic instead of FPGA FSM
        """
        self.sdb=cra.instance()
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        if "vth_shift" in self.params:
            self.sdb.setup.febs[0].petiroc.shift_10b_dac(self.params["vth_shift"])
        
        self.sdb.to_csv_files()
        configFEBMinidaqLogger(logging.INFO)

        try:
            self.fc7 = fc7_board(False)
            ### Test
            self.sdb.setup.febs[0].fpga_version='4.0'
            self.feb = feb_v2_cycloneV(self.fc7, fpga_fw_ver=self.sdb.setup.febs[0].fpga_version, petiroc_ver=self.sdb.setup.febs[0].petiroc_version, verbose=True)

            self.fc7.init(self.params["gbt_init"]==1)
            self.feb.boot(app_fw=False)

        except TestError as e:
            print(f"Test failed with message: {e}")

    def change_vth_shift(self,shift):
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.sdb.setup.febs[0].petiroc.shift_10b_dac(shift)
        self.sdb.to_csv_files()
    def change_db(self,state_name,version):
        self.params["db_state"]=state_name
        self.params["db_version"]=version
        
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.sdb.to_csv_files()

    def configure(self):
        self.feb.load_config_from_csv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (self.params["db_state"],self.params["db_version"],self.params["feb_id"]))
        enableforces2=True
        if ("disable_force_s2" in self.params):
            enableforces2=not (self.params["disable_force_s2"]==1)
        self.fc7.configure_los_fsm(   
            s0_duration=self.params["orbit_fsm"]["s0"],
            s1_duration=self.params["orbit_fsm"]["s1"],
            s2_duration=self.params["orbit_fsm"]["s2"],
            s3_duration=self.params["orbit_fsm"]["s3"],
            s4_duration=self.params["orbit_fsm"]["s4"],
            enable_force_s2=enableforces2)
       
        self.fc7.reset_bc0_id()

        if ("trigger" in self.params):
            trg=self.params["trigger"]
            if "n_bc0" in trg:
                self.fc7.configure_nBC0_trigger(trg["n_bc0"])
            if "n_data" in trg:
                self.fc7.configure_ndata_trigger(trg["n_data"])
            if "external" in trg:
                self.fc7.configure_external_trigger(trg["external"])
            if "periodic" in trg:
                self.fc7.configure_periodic_trigger(trg["periodic"])

      
    def setWriter(self,fw):
        self.writer=fw
    def writeRunHeader(self):
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

    def start(self,run=0):
        if (self.writer==None):
            logging.fatal("no writer defined")
            return
        configFEBMinidaqLogger(logging.WARN)
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
        r={}
        if (self.writer!=None):
            r["state"]="running"
            r["event"]=self.writer.eventNumber()
        else:
            r["state"]="not_running"
            r["event"]=-1
        return r  
    def acquiring_data(self):
        nacq= 0
        ntrig = 0
        configFEBMinidaqLogger(loglevel=logging.WARN)

        self.feb.enable_tdc(True)
        fout=open("debug.out","w")
        while (self.running):
            self.writer.newEvent()
            #self.fc7.reset_bc0_id()
            self.fc7.configure_acquisition_and_start(buf_size=self.params["config"]["buf_size"],
                                           triggerless=(self.params["config"]["triggerless"]==1),
                                           single=(self.params["config"]["single"]==1),
                                           keep=(self.params["config"]["keep"]==1),
                                           external_window=(self.params["config"]["external_window"]==1))
            #self.fc7.configure_acquisition(buf_size=self.params["config"]["buf_size"],
            #                               triggerless=1,
            #                               single=0,
            #                               keep=(self.params["config"]["keep"]==1),
            #                               external_window=0)
            
            #self.fc7.start_acquisition()
            nbt=0
            datawait=0
            nb_frame32=0
            nb_last=0
            nwait=0
            while (nb_frame32==0 or nb_last!=nb_frame32) and self.running:
                nb_frame32 = self.fc7.fifos.get_counter('fifo_rx_tdc_ipbus')
                nb_last=nb_frame32
                if (nwait%1000==999):
                    print(nwait)
                nwait+=1
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
                nb_frame32_1 = self.fc7.fifos.get_counter('fifo_rx_tdc_ipbus')
                if (nb_frame32%8!=0 or nb_frame32_1< nb_frame32):
                    print("oops not x 8 %d et le second %d \n" % (nb_frame32,nb_frame32_1))
                    nb_frame32 = self.fc7.fifos.get_counter('fifo_rx_tdc_ipbus')
                    continue
                else:
                    nb_frame32=nb_frame32_1

                try:
                    nb_frame32 = min(4096+2048, nb_frame32)
                    #print("frames :%d %d\n"% (nb_frame32//8,nb_frame32))
                    #sys.stdout.flush()
                    datas=self.fc7.fifos.read_block_tdc(nb_frame32)
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
                nb_frame32 = self.fc7.fifos.get_counter('fifo_rx_tdc_ipbus')
            if nbt == 0:
                message= "problem at fifo readout number {}, event written {}.".format(nacq, self.writer.eventNumber())
                logging.error(message)
                time.sleep(0.01)
            else:
                nacq+=1
                if not self.dummy:
                    self.writer.writeEvent()
           
            if (nacq % 100 == 0):
                message= "Info : Event {} (acquisition number {}) have read {} words = {} potential TDC frames.".format(self.writer.eventNumber(), nacq, nbt/(self.params["trigger"]["n_bc0"]-1), nbt/(self.params["trigger"]["n_bc0"]-1)/8)
                logging.info(message)

                #with open("/data/trigger_count.txt", "w") as trig_output:
                #    trig_output.write(str(self.writer.eventNumber()))             
            self.fc7.stop_acquisition()
            #time.sleep(0.005)
        logging.info("Thread %d: finishing", self.run)
    def stop(self):
        self.running= False
        self.producer_thread.join()
        self.fc7.stop_acquisition()
        self.feb.enable_tdc(False)
        configFEBMinidaqLogger(logging.INFO)
        self.writer.endRun()
        logging.info("Daq is stopped")

    
