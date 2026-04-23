

import threading
import time
import logging
import time
import numpy as np
from schema import Config,OrbitFsm,Trigger,Writer,FebDaqParams,FebAcquisition
import ax_storage as ps
import cms_irpc_feb_lightdaq as lightdaq # pyright: ignore[reportMissingImports]
import csv_register_access as cra # pyright: ignore[reportMissingImports]
import os
import json
import inspect
import threading
from transitions import Machine, State # pyright: ignore[reportMissingImports]
from transitions.core import MachineError
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/febv2debug.log", mode='w')  # ,
        # logging.StreamHandler()
    ]
)

def load_from_file(f_config:str) -> febv2_physic:
    c=json.loads(open(f_config).read())
    print(c)
    p=febv2_physic()
    p.set_configuration(c)
    return p

class febv2_physic:
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
        lightdaq.configLogger(logging.WARN)

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
        self.febwriter=None        
        self.params=None
        self.configured=False
        self.dummy=False
        self.sdb=cra.instance()
        self.methodes = [attr for attr in dir(self) if callable(getattr(self, attr))]
    def transition(self,name:str) -> dict:
        """ Execute a transition of the FSM

        Args:
            name (str): name of the transition to execute
        Returns:
            A dictionnary with the new state and status of the acquisition
        """
       
        if not hasattr(self, name):
            raise ValueError(f"Transition {name} not found")
       
        transition_method = getattr(self, name)
        if not callable(transition_method):
            raise ValueError(f"Transition {name} not callable")
        try:
            transition_method()
        except MachineError as e:
            raise ValueError(f"Transition {name} forbidden {e}")
        return self.get_status()
    def execute(self,nom:str,params:dict):
        # Vérifier si la méthode existe
        if not hasattr(self, nom):
            raise AttributeError(f"La méthode '{nom}' n'existe pas.")

        methode = getattr(self, nom)

        # Vérifier que c'est bien une méthode
        if not callable(methode):
            raise TypeError(f"'{nom}' n'est pas une méthode.")

        # Récupérer la signature de la méthode
        signature = inspect.signature(methode)
        parametres = signature.parameters

        # Vérifier que les clés du dictionnaire correspondent aux paramètres
        for cle in params.keys():
            if cle not in parametres:
                raise ValueError(f"Le paramètre '{cle}' n'existe pas pour la méthode '{nom}'.")

        # Appeler la méthode avec les paramètres
        return methode(**params)
            
    def set_configuration(self,c:str) -> None:
        self.conf=c
        self.params=FebAcquisition(**c)
        self.daq_conf=self.params.daq
        self.buf_size=self.daq_conf.config.buf_size
    def set_db_configuration(self,name:str,version:int) -> None:
        self.sdb.download_configuration(name,version)
        c=json.loads(open(f"/dev/shm/config/{name}_{version}.json").read())
        print(c)
        self.set_configuration(c)
    def store_configuration(self,name:str,version:int,comment="Comment is missing")-> None:
        self.conf["name"]=name
        self.conf["version"]=version
        fname="/tmp/%s_%s.json" % (name,version)
        f=open(fname,"w+")
        f.write(json.dumps(self.conf, indent=2, sort_keys=True))
        f.close()
        self.sdb.upload_configuration(fname,comment)
    def daq_destroying(self) -> bool:
        return True

    def daq_initialising(self) -> None:
        try:
            self.ax7325b = lightdaq.AX7325BBoard()
            self.feb0 = lightdaq.FebV2Board(self.ax7325b, febid='FEB0', fpga_fw_ver='4.8')
            self.ax7325b.init(feb0=True, feb1=False,mapping_mode=self.daq_conf.config.mapping)
            ### Test
            self.feb0.init()
        except NameError as e:
            print(f"Test failed with message: {e}")

    def daq_configuring(self,board_id:int=0,dbstate:str=None,dbversion:int=0) -> None:
        """ Now Configure the setup
        It creates the access to the DB,
        It configures the FEB and prepare the FC7 for a run setting the orbit and trigger definition
        """
        
        self.sdb.download_setup(self.daq_conf.db_state,self.daq_conf.db_version)
        self.sdb.setup.febs[0].fpga_version='4.8'
        # Handle possible changes of vth, ccomp or delay reset
        if self.daq_conf.vth_shift:
            self.sdb.setup.febs[0].petiroc.shift_10b_dac(self.daq_conf.vth_shift)
        if self.daq_conf.pa_ccomp:
            self.sdb.setup.febs[0].petiroc.set_parameter("pa_ccomp",self.daq_conf.pa_ccomp&0XF,asic=None)
        if self.daq_conf.delay_reset_trigger:
            self.sdb.setup.febs[0].petiroc.set_parameter("delay_reset_trigger",self.daq_conf.delay_reset_trigger&0XF,asic=None)
        self.sdb.setup.version=999
        self.sdb.to_csv_files()
        lightdaq.configLogger(logging.WARN)
        
        self.feb0.loadConfigFromCsv(folder='/dev/shm/feb_csv', base_name='%s_%d_f_%d_config' % (self.daq_conf.db_state,999,self.daq_conf.feb_id))
        enableforces2=True
        if (self.daq_conf.disable_force_s2!=None):
            enableforces2=not (self.daq_conf.disable_force_s2==1)
        for fpga in lightdaq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('standard')
            self.feb0.fpga[fpga].tdcEnable(False)       

        
        self.ax7325b.fastbitFsmConfigure(
            s0_duration=self.daq_conf.orbit_fsm.s0,
            s1_duration=self.daq_conf.orbit_fsm.s1,
            s2_duration=self.daq_conf.orbit_fsm.s2,
            s3_duration=self.daq_conf.orbit_fsm.s3,
            s4_duration=self.daq_conf.orbit_fsm.s4,
            enable_force_s2=enableforces2)

        self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=False, delay=self.daq_conf.config.resync_delay)
        #self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=True, delay=100)
    
        self.ax7325b.fastbitResetBc0Id()
        #self.fc7.configure_resync_external(100)
        #self.fc7.reset_bc0_id()

        if (self.daq_conf.trigger):
            trg=self.daq_conf.trigger
            if trg.n_bc0:
                self.ax7325b.triggerBc0Configure(int(trg.n_bc0) != 0 , trg.n_bc0) 
            if trg.external:
                self.ax7325b.triggerExternalConfigure(int(trg.external) != 0 , trg.external)                

        self.storage=None
        self.febwriter=None
        if self.params!=None:
            if self.daq_conf.writer.file_directory:
                self.storage=ps.storage_manager(self.daq_conf.writer.file_directory)
            elif self.daq_conf.writer.shm_directory:
                self.febwriter=ps.PyFebWriter(self.daq_conf.writer.shm_directory)
        #self.storage.open("unessai")
        self.runid=None
        self.configured=True

    def isConfigured(self) -> bool:
        """ Check the configuration
        Returns:
            True is configured
        """
        return self.configured
    def change_vth_shift(self,shift:int) -> None:
        """ Change the PETIROC VTH_TIME threshold
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            shift (int): Shift to add to VTH_TIME DAC10 bits taken in the DB
        """
        self.daq_conf.vth_shift=shift
    def change_paccomp(self,value:int) -> None:
        """ Change the PETIROC PACCOMP
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            value (int): PACCOMP to all asics
        """
        self.daq_conf.paccomp=value

    def change_delay_reset_trigger(self,value:int) -> None:
        """ Change the PETIROC delay_reset_trigger value
            The PETIROC parameters to be used are modified but not load (configure needed)
        Args:
            value (int): DELAY_RESET_TRIGGER VALUE
        """
        self.daq_conf.delay_reset_trigger=value
    def change_db(self,state_name:str,version:int) -> None:
        """ Download and store a new DB version
            The FPGA/PETIROC parameters to be used are stored but not load (configure needed)
        Args:
            state_name (str): name of the state
            version (int)" version number
        """
        self.daq_conf.db_state=state_name
        self.daq_conf.db_version=version
        
        self.sdb.download_setup(self.daq_conf.db_state,self.daq_conf.db_version)
        self.sdb.to_csv_files()

    def daq_starting(self,location=None,comment=None,params={"type":"NORMAL"}) -> None:
        if self.params!=None and location ==None:
            location=self.daq_conf.location
            self.logger.info(f"DAQ parameter {self.params}")
            #exit(0)
            comment=self.daq_conf.comment if "comment" in self.daq_conf else "No comment set"
                         
        runobj = self.sdb.getRun(location,comment)
        self.runid = runobj["run"]
        if self.runid == None:
            self.runid = int(input("Enter a run number: "))

        self.run_type=1
        if self.storage: 
            # Store results in json
            self.storage.open(f'{self.daq_conf.location}_{self.runid}_DB{self.daq_conf.db_state}_{self.daq_conf.db_version}_{self.daq_conf.feb_id}_VTH{self.daq_conf.vth_shift}')

            
            if self.params==None:
                rh=np.array([self.run_type,self.daq_conf.vth_shift],dtype='int64')
                self.storage.writeRunHeader(self.runid,rh)
            else:
                self.storage.writeRunHeaderDict(self.runid,self.params.to_dict())
                self.logger.info("Run header writen")
        print(f"Now we start with \n {params}")
        
        self.logger.info("Normal run")
        self.normal_run()
        
    def normal_run(self,params=None) -> bool:
        if self._thread and self._thread.is_alive():
            self.logger.warning("Acquisition déjà en cours")
            return False
        self._running.set()
        self._thread = threading.Thread(target=self.normal_loop, args=(params,), daemon=True)
        self._thread.start()
        return True
    
    def normal_loop(self, params=None) -> None:
        self.logger.info("NORMAL Acquisition thread démarré")
        while self._running.is_set():
            # simulate acquisition tick
            
            self.status=self.acq_status()
            self.acquiring_data()
            if self.storage:
                if self.storage.event%100 == 0: 
                    self.logger.info(f"Acquisition {self.storage.run} {self.storage.event}")
            elif self.febwriter:
                if self.febwriter._event%100 == 0: 
                    self.logger.info(f"Acquisition {self.runid} {self.febwriter._event}")
            time.sleep(0.001)
        self.logger.info("Acquisition thread arrêté")


    def acq_status(self) -> dict:
        """ returns the status of the acquisition
        Returns:
            A dictionnary object with status and event number
        """
        r={}
        r["state"]=self.state
        if self.params!=None:
            r["configuration"]=self.daq_conf.dict()
        if (self._running.is_set()):
            if self.storage:
                r["run"] = self.storage.run
                r["event"]=self.storage.event
            elif self.febwriter:
                r["run"] = self.runid
                r["event"]=self.febwriter._event
            
            
        else:
            r["run"] =-1
            r["event"]=-1
        return r
    def start_acquisition(self) -> None:
        acq_ctrl =  lightdaq.BitField(self.ax7325b.ipbRead('ACQ_CTRL'))
        acq_ctrl[30] = 1
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = self.buf_size
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
        self.logger.debug(f"start_acquisition")
    def stop_acquisition(self) -> None:
        acq_ctrl =  lightdaq.BitField(self.ax7325b.ipbRead('ACQ_CTRL'))
        acq_ctrl[30] = 0
        acq_ctrl[29] = 1
        acq_ctrl[15,0] = 0
        self.ax7325b.ipbWrite('ACQ_CTRL', acq_ctrl)
        self.logger.debug(f"stop_acquisition")

    def hasTrigger(self) -> bool:
        acq_status = lightdaq.BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
        return (acq_status[31] == 0)
    
    def getNFrames(self) -> int:
        acq_status = lightdaq.BitField(self.ax7325b.ipbRead('ACQ_STATUS'))
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
    def readFrames(self,n) -> list:
        self.logger.info(f"Reading {n+3} TDC frames")
        try:
            rawdata = self.ax7325b.ipbReadBlock('FEBS_TDC_DATA_WORDS', (n+3)*8) # 3 tdcframes are stuck between ringbuffer and ipbreadout
        except:
            self.logger.error("cannot read block {(n+3)*8} ")

        return rawdata

    def acquiring_data(self) -> None:
        """ Acquisition thread

        While running, it loops continously and spy data in the FC7 readout fifo
        It writes data to disk or shared memory until running is false and the run stopped. 
        """
        nacq= 0
        nbt = 0
        self.logger.setLevel(logging.INFO)
        #self.feb.enable_tdc(True)
        for fpga in lightdaq.FPGA_ID:
            self.feb0.fpga[fpga].tdcSetInjectionMode('standard')
            self.feb0.fpga[fpga].tdcEnable(True)
            self.feb0.fpga[fpga].tdcEnableChannel()       

        evt=0
        while (self._running.is_set()):
            self.ax7325b.fastbitResyncConfigure(external=True, after_bc0=False, delay=2)
            #print(f"Running {self.running()}")
            self.ax7325b.fastbitResetBc0Id()
            if self.febwriter:
                self.febwriter.newEvent()
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
                    if self.storage:
                        self.storage.writeEvent([words])
                        evt=self.storage.event
                    elif self.febwriter:
                        self.febwriter.appendEventData(words)
                        self.febwriter.writeEvent()
                        evt=self.febwriter._event
        
            if (nacq % 1 == 0):
                message= "Info : Event {} (acquisition number {}) have read {} words = {} potential TDC frames.".format(evt, nacq, nbt, nbt/8)
                self.logger.info(message)
                if not self._running.is_set():
                    self.logger.info("running lock is cleared")
                    self.stop_acquisition()
                    self.ax7325b.flushDataflow()
                    break
            self.stop_acquisition()
            self.ax7325b.flushDataflow()
            time.sleep(0.005)
            
        for fpga in lightdaq.FPGA_ID:
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
            if self.storage:
                self.storage.close()
            self._running.clear()
            return False
        self._running.clear()
        # join optionnel court
        self._thread.join(timeout=10)
        if self.storage:
            self.storage.close()
        return True
        
    def running(self):
        return self._running.is_set()

    def get_status(self) -> dict:
        self.status=self.acq_status()
        return dict(self.status, running=self.running())
