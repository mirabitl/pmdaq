import logging
import random
import time
import argparse
import json
import os.path
from transitions import Machine, State
import FebWriter as FW
from febv2_setup import *
import csv_register_access as cra
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("debug.log", mode='w')  # ,
        # logging.StreamHandler()
    ]
)


class febv2_fsm:
    def __init__(self, config_file=None):
        self.db = cra.instance()
        login=os.getenv("DAQSETUP","NONE")
        if (login != "NONE" and config_file==None):
            acq_name=login.split(":")[0]
            acq_vers=int(login.split(":")[1],0)
            self.db.download_configuration(acq_name,acq_vers,True)
            self.config_file=f"/dev/shm/config/{acq_name}_{acq_vers}.json"
        else:
            if (config_file!=None):
                self.config_file=config_file
            else:
                print("No config file")
                exit(0)
                # The DAQ FSM by itself
        self.comment=None
        self.running = False
        self.configured = False
        self.daqfsm = Machine(model=self, states=[
                              'CREATED', 'INITIALISED', 'CONFIGURED', 'RUNNING', 'CONFIGURED'], initial='CREATED')
        self.daqfsm.add_transition(
            'initialise', 'CREATED', 'INITIALISED', after='daq_initialising')
        self.daqfsm.add_transition(
            'configure', ['INITIALISED', 'CONFIGURED'], 'CONFIGURED', after='daq_configuring')
        self.daqfsm.add_transition('start', 'CONFIGURED',
                                   'RUNNING',     after='daq_starting', conditions='isConfigured')
        self.daqfsm.add_transition('stop', 'RUNNING',
                                   'CONFIGURED',  after='daq_stopping', conditions='isConfigured')
        self.daqfsm.add_transition(
            'destroy', ['INITIALISED', 'CONFIGURED'], 'CREATED',     after='daq_destroying')
        self.state = "CREATED"

        # data needed for non dummy modes
        self.run = 0
        # getting the configuration
        self.config = json.load(open(self.config_file))
        print(self.config)
        self.setup=febv2_setup(self.config["daq"])

        # debug printout
        self.EDAQ_debug = False
        self.dummy = False
        # Share memory tagging
        self.writer = None
        self.setAcquisitionWithFEB()
    def getEventNumber(self):
        return self.writer.eventNumber()

    def getRunNumber(self):
        return self.writer.runNumber()

    def isConfigured(self):
        return self.configured

    def setDaqMode(self, daqMode):
        self.config["daq"]["mode"] = daqMode
    def setRun(self, r):
        self.run=r
    def setComment(self, r):
        self.comment=r
    # Do nothing DAQ mode. This allows to test the FSM.
    def printState(self):
        print(self.state)

    # Talking to the FEB through FC7
    #
    def dispatch_config(self):
        if (self.setup!=None):
            self.setup.params["daq"]=self.config["daq"]
    def setOrbitMode(self):
        self.config["daq"]["mode"] = "Wait_orbit"
        self.dispatch_config()
        self.setAcquisitionWithFEB()

    def setTimeWindowMode(self):
        self.config["daq"]["mode"] = "Time_Window"
        self.dispatch_config()
        self.setAcquisitionWithFEB()

    def setTDCcountMode(self):
        self.config["daq"]["mode"] = "TDC_Count"
        self.dispatch_config()
        self.setAcquisitionWithFEB()

    def setExternalCircularBufferMode(self):
        self.config["daq"]["mode"] = "Circular_Buffer_External"
        self.dispatch_config()
        self.setAcquisitionWithFEB()

    def setInternalCircularBufferMode(self):
        self.config["daq"]["mode"] = "Circular_Buffer_Internal"
        self.dispatch_config()
        self.setAcquisitionWithFEB()

    def setAcquisitionWithFEB(self):
        self.daq_initialising = self.FEB_initialising
        self.daq_configuring = self.FEB_configuring
        self.daq_starting = self.FEB_starting
        self.daq_stopping = self.FEB_stopping
        self.daq_destroying = self.FEB_destroying
        #self.feb_running = self.FEB_acquiring_data

    def FEB_initialising(self):
        if self.EDAQ_debug:
            logging.getLogger().setLevel(logging.DEBUG)
        self.setup.init()
        logging.info("DAQ, FC7 and FEB are initialised")

    def FEB_configuring(self):
        self.setup.configure()
        # self.feb.reset()
        self.configured = True
        logging.info("Daq, FC7 and FEB are configured")


    def FEB_starting(self):
        print("STARTING ",self.config["daq"])
        if (self.writer == None):
            if "writer" in self.config["daq"]:
                s_w=self.config["daq"]["writer"]
                self.writer = FW.FebWriter(s_w["file_directory"], s_w["location"])
                self.writer.setIds(s_w["detector_id"], s_w["source_id"])
                if ("shm_directory" in s_w):
                    os.system("mkdir -p %s/closed" % s_w["shm_directory"])
                    self.writer.setShmDirectory(s_w["shm_directory"])
                self.setup.setWriter(self.writer)
            else:
                logging.fatal("No writer definition")
                return


        if (self.comment==None):
            loc=self.config["daq"]["location"]
            self.comment=f'Setup {loc}|'
        self.comment=self.comment+f"Acquisition from {self.config_file}"
        if (not self.writer.useShm()):
            newrun = self.db.getRun(self.config["daq"]["location"], self.comment)
            self.run=newrun["run"]
            os.system("mkdir -p %s" % (self.config["daq"]["writer"]["file_directory"]))
        else:
            self.run=999
        self.writer.newRun(self.run)
        logging.info("Data are written in file %s." % self.writer.file_name())
        self.setup.writeRunHeader()
        self.setup.start(run=self.run)

  
    def FEB_stopping(self):
        self.running = False
        self.setup.stop()
        self.writer.endRun()
        logging.info("Daq is stopped")

    def FEB_destroying(self):
        self.configured = False
        del self.setup
        self.setup=febv2_setup(self.config["daq"])

        # debug printout
        self.EDAQ_debug = False
        self.dummy = False
        # Share memory tagging
        self.writer = None
        self.setAcquisitionWithFEB()
        logging.info("Daq is destroyed")

    def changeDB(self, state, version):
        self.config["daq"]["db_state"] = state
        self.config["daq"]["db_version"] = version
        self.dispatch_config()
        self.setup.change_db(state,version)
    def change_vth_shift(self,shift):
        self.setup.change_vth_shift(shift)

    def veto(self):
        return False


if __name__ == '__main__':
    daqMode = "DoNothing"
    """
      fb=FEB_control_FSM(daqMode)
      if needStateConfig:
         #fb.config_statename="UN_TEST_FEBV2"
      fb.config_statename="FEB_TEST_DOME_C"
      fb.config_version=2
      fb.config_source="DB"
      fb.AlbanStylePrintout=True
      fb.initialise()
      time.sleep(2)
      fb.configure()
      time.sleep(2)
      fb.start()
      time.sleep(5)
      fb.stop()
      time.sleep(2)
      fb.destroy()
      """
