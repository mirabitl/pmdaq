

import threading
import time
import logging
import time
import numpy as np
import ax_storage as ps

import cms_irpc_feb_lightdaq as daq # pyright: ignore[reportMissingImports]

import csv_register_access as cra # pyright: ignore[reportMissingImports]
import os
import json

import threading
from transitions import Machine, State # pyright: ignore[reportMissingImports]

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/febv2debug.log", mode='w')  # ,
        # logging.StreamHandler()
    ]
)

def load_from_file(f_config):
    c=json.loads(open(f_config).read())
    print(c)
    p=febv2_light()
    p.set_configuration(c)
    return p

class febv2_light:
    """The Light FEBV2 setup interface class.
    It hides access to MINIDAQ driver classes and implements basics function as
    init,configure,start,stop ....
    """
    def __init__(self,verbose=False):
        """
        Object creation

        Args:
            config: a python object conatining the daq configuration
            verbose (bool): False by default, tune to true for debug printout
        """
        daq.configLogger(logging.WARN)

        self._thread = None
        self._running = threading.Event()
        self._lock = threading.Lock()
        self.status = {"run": 0, "event": 0}
        self.logger=logging.getLogger(__name__)
        self.conf=None
        self.daqfsm = Machine(model=self, states=['CREATED', 'INITIALISED', 'CONFIGURED', 'RUNNING','CONFIGURED'], initial='CREATED')
        self.daqfsm.add_transition(
            'initialise', 'CREATED', 'INITIALISED', after='daq_initialising')
        self.daqfsm.add_transition(
            'configure', ['INITIALISED', 'CONFIGURED'], 'CONFIGURED', after='daq_configuring')
        self.daqfsm.add_transition(
            'configure', ['CONFIGURED', 'CONFIGURED'], 'CONFIGURED', after='daq_configuring')
        self.daqfsm.add_transition('start', 'CONFIGURED',
                                   'RUNNING',     after='daq_starting', conditions='isConfigured')
        self.daqfsm.add_transition('stop', 'RUNNING',
                                   'CONFIGURED',  after='daq_stopping', conditions='isConfigured')
        self.daqfsm.add_transition(
            'destroy', ['INITIALISED', 'CONFIGURED'], 'CREATED',after='daq_destroying')

        self.storage=None
        self.last_paccomp=None
        self.last_delay_reset_trigger=None
        
        self.params=None
        self.configured=False
        self.dummy=False

    def set_configuration(self,c):
        self.conf=c
        self.params=c["daq"]
        self.buf_size=self.params["config"]["buf_size"]
    def set_db_configuration(self,name,version):
        self.sdb.download_configuration(name,version)
        c=json.loads(open(f"/dev/shm/config/{name}_{version}.json").read())
        print(c)
        self.set_configuration(c)
    def store_configuration(self,name,version,comment="Comment is missing"):
        self.conf["name"]=name
        self.conf["version"]=version
        fname="/tmp/%s_%s.json" % (name,version)
        f=open(fname,"w+")
        f.write(json.dumps(self.conf, indent=2, sort_keys=True))
        f.close()
        self.sdb.upload_configuration(fname,comment)
    def daq_destroying(self):
        return True

    def daq_initialising(self):
        try:
            self.ax7325b = daq.AX7325BBoard()
            self.feb0 = daq.FebV2Board(self.ax7325b, febid='FEB0', fpga_fw_ver='4.8')
            self.ax7325b.init(feb0=True, feb1=False,mapping_mode=self.params["config"]["mapping"])
            ### Test
            #self.sdb.setup.febs[0].fpga_version='4.8'
            self.feb0.init()
        except NameError as e:
            print(f"Test failed with message: {e}")

    def daq_configuring(self,board_id=0,dbstate=None,dbversion=0):
        """ Now Configure the setup
        It creates the access to the DB,
        It configures the FEB and prepare the FC7 for a run setting the orbit and trigger definition
        """
        self.sdb=cra.instance()
        self.sdb.download_setup(self.params["db_state"],self.params["db_version"])
        self.sdb.setup.febs[0].fpga_version='4.8'
        # Handle possible changes of vth, ccomp or delay reset
        if "vth_shift" in self.params:
            self.sdb.setup.febs[0].petiroc.shift_10b_dac(self.params["vth_shift"])
        if "pa_ccomp" in self.params:
            self.sdb.setup.febs[0].petiroc.set_parameter("pa_ccomp",self.params["pa_ccomp"]&0XF,asic=None)
        if "delay_reset_trigger" in self.params:
            self.sdb.setup.febs[0].petiroc.set_parameter("delay_reset_trigger",self.params["delay_reset_trigger"]&0XF,asic=None)

        self.sdb.to_csv_files()
        daq.configLogger(logging.WARN)
        
        self.feb0.loadConfigFromCsv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (self.params["db_state"],self.params["db_version"],self.params["feb_id"]))
        #enableforces2=True
        if ("disable_force_s2" in self.params):
            enableforces2=not (self.params["disable_force_s2"]==1)
        for fpga in daq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('standard')
            self.feb0.fpga[fpga].tdcEnable(False)       

        
        self.ax7325b.fastbitFsmConfigure(
            s0_duration=self.params["orbit_fsm"]["s0"],
            s1_duration=self.params["orbit_fsm"]["s1"],
            s2_duration=self.params["orbit_fsm"]["s2"],
            s3_duration=self.params["orbit_fsm"]["s3"],
            s4_duration=self.params["orbit_fsm"]["s4"],
            enable_force_s2=enableforces2)

        self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=False, delay=self.params["config"]["resync_delay"])
        #self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=True, delay=100)
    
        self.ax7325b.fastbitResetBc0Id()
        #self.fc7.configure_resync_external(100)
        #self.fc7.reset_bc0_id()

        if ("trigger" in self.params):
            trg=self.params["trigger"]
            if "n_bc0" in trg:
                #self.fc7.configure_nBC0_trigger(trg["n_bc0"])
                self.ax7325b.triggerBc0Configure(int(trg["n_bc0"]) != 0 , trg["n_bc0"]) 

            if "external" in trg:
                #self.fc7.configure_external_trigger(trg["external"])
                self.ax7325b.triggerExternalConfigure(int(trg["external"]) != 0 , trg["external"])                
            #if "periodic" in trg:
            #    self.fc7.configure_periodic_trigger(trg["periodic"])
        if self.params!=None:
            self.storage=ps.storage_manager(self.params["writer"]["file_directory"])
        #self.storage.open("unessai")
        self.runid=None
        self.configured=True

    def isConfigured(self):
        """ Check the configuration
        Returns:
            True is configured
        """
        return self.configured
    def change_vth_shift(self,shift):
        """ Change the PETIROC VTH_TIME threshold
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            shift (int): Shift to add to VTH_TIME DAC10 bits taken in the DB
        """
        #self.sdb.setup.febs[0].petiroc.shift_10b_dac(shift)
        #self.sdb.to_csv_files()
        self.params["vth_shift"]=shift
    def change_paccomp(self,value):
        """ Change the PETIROC PACCOMP
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            value (int): PACCOMP to all asics
        """
        #self.sdb.setup.febs[0].petiroc.set_parameter("pa_ccomp",value&0XF,asic=None)
        #self.sdb.to_csv_files()
        self.params["paccomp"]=value

    def change_delay_reset_trigger(self,value):
        """ Change the PETIROC delay_reset_trigger value
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            value (int): DELAY_RESET_TRIGGER VALUE
        """
        #self.sdb.setup.febs[0].petiroc.set_parameter("delay_reset_trigger",value&0XF,asic=None)
        #self.sdb.to_csv_files()
        self.params["delay_reset_trigger"]=value
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

    def daq_starting(self,location=None,comment=None,params={"type":"NORMAL"}):
        if self.params!=None and location ==None:
            location=self.params["location"]
            self.logger.info(f"DAQ parameter {self.params}")
            #exit(0)
            comment=params["comment"] if "comment" in params else "No comment set"
                         
        runobj = self.sdb.getRun(location,comment)
        self.runid = runobj["run"]
        if self.runid == None:
            self.runid = int(input("Enter a run number: "))

        # Store results in json
        self.storage.open(f'run_{self.runid}_{self.params["db_state"]}_{self.params["db_version"]}_{self.params["feb_id"]}_{self.params["vth_shift"]}')

        self.run_type=1
        if self.params==None:
            rh=np.array([self.run_type,self.params["vth_shift"]],dtype='int64')
            self.storage.writeRunHeader(self.runid,rh)
        else:
            self.storage.writeRunHeaderDict(self.runid,self.params)
            self.logger.info("Run header writen")
        print(f"Now we start with \n {params}")
        
        self.logger.info("Normal run")
        self.normal_run()
        
    def normal_run(self,params=None):
        if self._thread and self._thread.is_alive():
            self.logger.warning("Acquisition déjà en cours")
            return False
        self._running.set()
        self._thread = threading.Thread(target=self.normal_loop, args=(params,), daemon=True)
        self._thread.start()
        return True
    
    def normal_loop(self, params=None):
        self.logger.info("NORMAL Acquisition thread démarré")
        while self._running.is_set():
            # simulate acquisition tick
            
            self.status=self.acq_status()
            self.acquiring_data()
            if self.storage.event%100 == 0: 
                self.logger.info(f"Acquisition {self.storage.run} {self.storage.event}")
            time.sleep(0.001)
        self.logger.info("Acquisition thread arrêté")


    def acq_status(self):
        """ returns the status of the acquisition
        Returns:
            A dictionnary object with status and event number
        """
        r={}
        if (self._running.is_set()):
            r["run"] = self.storage.run
            r["state"]=self.state
            r["event"]=self.storage.event
        else:
            r["run"] =-1
            r["state"]=self.state
            r["event"]=-1
        return r
    def start_acquisition(self):
        acq_ctrl =  daq.BitField(self.ax7325b.ipbRead('ACQ_CTRL'))
        acq_ctrl[30] = 1
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = self.buf_size
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
        self.logger.debug(f"start_acquisition")
    def stop_acquisition(self):
        acq_ctrl =  daq.BitField(self.ax7325b.ipbRead('ACQ_CTRL'))
        acq_ctrl[30] = 0
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
        self.logger.debug(f"stop_acquisition")

    def hasTrigger(self):
        acq_status = daq.BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
        return (acq_status[31] == 0)
    
    def getNFrames(self):
        acq_status = daq.BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
        istat=self.ax7325b.ipbRead('ACQ_STATUS')
        self.logger.debug(f"Status {acq_status:08b} {bin(istat)[2:6]}")
        try:
            assert acq_status[31] == 0, "Not trigged => no data!"
        except:
            self.logger.warning("Bit 31 set")
            return 0
        n = acq_status[15,0]
        #self.logger.info(f"Reading {n}+3 TDC frames")
        return n
    def readFrames(self,n):
        self.logger.info(f"Reading {n+3} TDC frames")
        try:
            rawdata = self.ax7325b.ipbReadBlock('FEBS_TDC_DATA_WORDS', (n+3)*8) # 3 tdcframes are stuck between ringbuffer and ipbreadout
        except:
            self.logger.error("cannot read block {(n+3)*8} ")

        return rawdata

    def acquiring_data(self):
        """ Acquisition thread

        While running, it loops continously and spy data in the FC7 readout fifo
        It writes data to disk or shared memory until running is false and the run stopped. 
        """
        nacq= 0
        nbt = 0
        self.logger.setLevel(logging.INFO)
        #self.feb.enable_tdc(True)
        for fpga in daq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('standard')
            self.feb0.fpga[fpga].tdcEnable(True)
            self.feb0.fpga[fpga].tdcEnableChannel()       


        while (self._running.is_set()):
            self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=False, delay=2)
            #print(f"Running {self.running()}")
            self.ax7325b.fastbitResetBc0Id()
            words=[]
            self.start_acquisition()
            nb_frames=0
            while (nb_frames==0 and self._running.is_set()):
                if not self.hasTrigger():
                    time.sleep(0.005)
                    continue
                try:
                    nb_frames = self.getNFrames()
                except: 
                    self.logger.info(f"Cannot read nframes")
                    nb_frames=0  
                self.logger.info(f"Read  getNFrames {nb_frames}")
            if not self._running.is_set():
                break
            self.logger.info(f"Found {nb_frames}")
            while nb_frames!=0:
                try:
                    message= "Nb frames to be block read {}".format(nb_frames)
                    self.logger.info(message)
                    words+=self.readFrames(nb_frames)
                except:
                    self.logger.error("error in readFrames")
                nbt+=nb_frames
                nb_frames = self.getNFrames()
                    
            if nbt == 0:
                message= "problem at frames readout number {}, event written {}.".format(nacq, self.writer.eventNumber())
                self.logger.error(message)
                time.sleep(0.01)
            else:
                nacq+=1
                if not self.dummy:
                    #self.logger.info(f'Data {words}')
                    self.storage.writeEvent([words])
        
            if (nacq % 1 == 0):
                message= "Info : Event {} (acquisition number {}) have read {} words = {} potential TDC frames.".format(self.storage.event, nacq, nbt, nbt/8)
                self.logger.info(message)
                if not self._running.is_set():
                    self.logger.info("running lock is cleared")
                    self.stop_acquisition()
                    self.ax7325b.flushDataflow()
                    break
            self.stop_acquisition()
            self.ax7325b.flushDataflow()
            time.sleep(0.005)
            
        for fpga in daq.FPGA_ID:
            self.feb0.fpga[fpga].tdcEnable(False) 
        self.stop_acquisition()
        nb_frames = self.getNFrames()
        if not nb_frames==0:
            self.ax7325b.flushDataflow();
        time.sleep(0.005)
        self.logger.info("Thread %d: finishing", self.storage.run)

    def daq_stopping(self, params=None):
       
        if not (self._thread and self._thread.is_alive()):
            self.logger.warning("Acquisition non démarrée")
            self.storage.close()
            self._running.clear()
            return False
        self._running.clear()
        # join optionnel court
        self._thread.join(timeout=10)
        self.storage.close()
        return True
        
    def running(self):
        return self._running.is_set()

    def get_status(self):
        self.status=self.acq_status()
        return dict(self.status, running=self.running())
    
