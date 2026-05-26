

import threading
import time
import logging
import time
import numpy as np
from picmic.services.picmic_schema import *
import picmic_storage as ps
import liroc_ptdc_daq as daq
import picmic_register_access as cra
import agilent81160 as agp
import os
import json
import inspect
from transitions import Machine, State # pyright: ignore[reportMissingImports]
from transitions.core import MachineError
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/picdebug.log", mode='w')  # ,
        # logging.StreamHandler()
    ]
)

def load_from_file(f_config:str):
    c=json.loads(open(f_config).read())
    print(c)
    p=picmic_physic()
    p.set_configuration(c)
    return p

class picmic_physic:
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
        self.daqwriter=None        
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
            
    def set_configuration(self,c:Dict[str, object]) -> None:
        self.config_dict=c
        self.config=PicConfig.model_validate(c)
        self.daq_conf=self.config.configuration_list[self.config.configuration]
    def store_configuration(self,name:str,version:int,comment="Comment is missing")-> None:
        fname="/tmp/%s_%s.json" % (self.config.name,self.config.version)
        f=open(fname,"w+")
        f.write(json.dumps(self.conf, indent=2, sort_keys=True))
        f.close()
        self.sdb.upload_configuration(fname,comment)
    def daq_destroying(self) -> bool:
        return True

    def daq_initialising(self) -> None:
        
        try:
            # Open KC705
            self.kc705 = daq.KC705Board()
            self.kc705.init()

            self.feb = daq.FebBoard(self.kc705)
            self.feb.init()

        except NameError as e:
            print(f"Test failed with message: {e}")

    def daq_configuring(self,board_id:int=0,dbstate:str=None,dbversion:int=0) -> None:
        """ Now Configure the setup
        It creates the access to the DB,
        It configures the board and prepare the KC705 for a run 
        """
       
        self.sdb.download_setup(self.daq_conf.db.state, self.daq_conf.db.version)
        self.sdb.to_csv_files()

        # Default threshold set at 800
        target = self.daq_conf.threshold if self.daq_conf.threshold else 800
        tlsb = target & 0xFF
        tmsb = (target >> 8) & 0xFF
        self.sdb.setup.boards[0].picmic.set("dac_threshold_lsb", tlsb)
        self.sdb.setup.boards[0].picmic.set("dac_threshold_msb", tmsb)
        
        # Maximal filtering
        if self.daq_conf.filtering:
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
        self.sdb.setup.boards[0].picmic.set("falling_en", self.daq_conf.falling)
        # ValEvt
        self.sdb.setup.boards[0].picmic.set("Forced_ValEvt", self.daq_conf.val_evt)
        #self.sdb.setup.boards[0].picmic.set("EN-CLPS", 1)
        #self.sdb.setup.boards[0].picmic.set("EN-pE", 15)
        #self.sdb.setup.boards[0].picmic.set("PA_gain", 15)
        # Polarity
        self.sdb.setup.boards[0].picmic.set("Polarity", self.daq_conf.pol_neg)
        # DC_PA
        for ch in range(64):
            if self.daq_conf.dc_pa != 0:
                self.sdb.setup.boards[0].picmic.set("DC_PA_ch", self.daq_conf.dc_pa, ch)
        self.sdb.setup.version = 888
        self.sdb.to_csv_files()

        self.logger.info(f"Version {self.sdb.setup.boards[0].picmic_version}")


        self.feb.loadConfigFromCsv(
            folder="/dev/shm/board_csv",
            config_name="%s_%d_f_%d_config_picmic.csv" % (self.daq_conf.db.state, 888, self.daq_conf.db.board)
        )
        self.feb.fpga.enableDownlinkFastControl()
        # disable all liroc channels
        for ch in range(64):
            self.feb.liroc.maskChannel(ch)
        if self.daq_conf.mode != None:
            self.feb.ptdc.setResMode(self.daq_conf.mode)

        self.feb.ptdc.powerup()

        self.kc705.fastbitConfigure(mode='normal',dig2_edge='rising', dig2_delay=1)


    #def daq_configuring(self,threshold=0,channel_list=[i for i in range(64)],ctest_list=[]):
        #print(channel_list)
        #input()
        if self.daq_conf.channel_list != None:
            channel_list=self.daq_conf.channel_list
        else:
            channel_list=[i for i in range(64)]
        if self.daq_conf.ctest_list != None:
            ctest_list=self.daq_conf.ctest_list
        else:
            ctest_list=[]
        for ch in range(64):
            if ch in channel_list:
                self.logger.info(f"{ch} is unmasked")
                self.feb.liroc.maskChannel(ch, False)
            else:
                self.feb.liroc.maskChannel(ch, True)
            if ch in ctest_list:
                self.logger.info(f"{ch} is CTest")
                self.feb.liroc.setParam(f"Ctest_ch{ch}",True)
            else:
                self.feb.liroc.setParam(f"Ctest_ch{ch}",False)
                #self.feb.liroc.maskChannel(ch, True)
        # Set the threshold
        self.feb.liroc.set10bDac(self.daq_conf.threshold if self.daq_conf.threshold else 800)
        self.feb.liroc.stopScClock()
        #input("Hit return to continue..")
        # generate a few windows to flush out the agilient patterns
        self.kc705.acqSetWindow(1, 1)
        for _ in range(5): 
            self.kc705.ipbWrite('ACQ_CTRL.window_start', 1)
            self.kc705.ipbWrite('ACQ_CTRL.window_start', 0)
        # files
        if self.run_params!=None:
            self.storage=ps.storage_manager(self.daq_conf.storage.directory)
        #self.storage.open("unessai")
        self.runid=None



        self.configured=True
            

    def isConfigured(self) -> bool:
        """ Check the configuration
        Returns:
            True is configured
        """
        return self.configured

    def change_db(self,state_name:str,version:int) -> None:
        """ Download and store a new DB version
            The FPGA/PETIROC parameters to be used are stored but not load (configure needed)
        Args:
            state_name (str): name of the state
            version (int)" version number
        """
        self.daq_conf.db.state=state_name
        self.daq_conf.db.version=version
        
        self.sdb.download_setup(self.daq_conf.db.state,self.daq_conf.db.version)
        self.sdb.to_csv_files()

    def daq_starting(self,location=None,comment=None,params={"type":"NORMAL"}) -> None:
        if self.params!=None and location ==None:
            location=self.daq_config.location
            self.logger.info(f"DAQ parameter {self.daq_conf}")
            #exit(0)
            comment=self.daq_conf.comment if "comment" in self.daq_conf else "No comment set"
                         
        runobj = self.sdb.getRun(location,comment)
        self.runid = runobj["run"]
        if self.runid == None:
            self.runid = int(input("Enter a run number: "))

        self.run_type=1
        if self.storage: 
            # Store results in json
            self.storage.open(f'{self.config.location}_{self.runid}_DB{self.daq_conf.db.state}_{self.daq_conf.db.version}_{self.daq_conf.db.board}_VTH{self.daq_conf.threshold if self.daq_conf.threshold else 800}')

            
            if self.params==None:
                rh=np.array([self.run_type,self.daq_conf.threshold] ,dtype='int64')
                self.storage.writeRunHeader(self.runid,rh)
            else:
                self.storage.writeRunHeaderDict(self.runid,self.daq_conf.to_dict())
                self.logger.info("Run header writen")
        print(f"Now we start with \n {params}")
        if isinstance(self.daq_conf, NormalRunConfig):
            self.logger.info("Normal run")
            self.normal_run()
        elif isinstance(self.daq_conf, TimeLoopRunConfig):
            self.logger.info("Time loop run")
            self.time_loop_run()
        elif isinstance(self.daq_conf, ScurveRunConfig):
            self.logger.info("Scurve run")
        else:
            raise ValueError(f"Unknown run type {self.daq_conf.type}")        
    def normal_loop(self):
        self.logger.info("NORMAL Acquisition thread démarré")
        while self._running.is_set():
            # simulate acquisition tick
            with self._lock:
                self.status["run"] = self.storage.run
                self.status["event"] = self.storage.event
                self.acquire_and_store(self.daq_conf.nacq if self.daq_conf.nacq else 40)
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
            #input()
        else:
            self.logger.error("No agilent pulser")
            return
    def time_loop(self, params=None):
        self.logger.info("TIMELOOP Acquisition thread démarré")
        # Check pulser
        if self.pulser==None:
            self.logger.error("Timeloop thread needs pulser to be defined (call init_pulser before) exiting")
            return
        # Check parameters
        vmin=self.daq_conf.vmin
        
        vmax=self.daq_conf.vmax
        rise=self.daq_conf.rise*1.0E-9
        delay=self.daq_conf.delay*1.0E-9
        
        
        nstep=self.daq_conf.nstep
        
        use_ctest=self.daq_conf.ctest==1 if self.daq_conf.ctest else False
       
        nacq=self.daq_conf.nacq if self.daq_conf.nacq else 40
        
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
                    if not self._running.is_set():
                        break
                    self.logger.info(f"Step {vset}  acquistion of {nacq} events")
                    self.setup_injection(vset,rise,delay,use_ctest)
                    self.acquire_and_store(nacq)
                    self.status["run"] = self.storage.run
                    self.status["event"] = self.storage.event
                self.pulser.setOFF(1)
                self.pulser.setOFF(2)
                break
        self.logger.info("TimeLoop Acquisition thread arrêté")

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
    
    def daq_stopping(self, params=None):
        with self._lock:
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
  
    def acq_status(self) -> dict:
        """ returns the status of the acquisition
        Returns:
            A dictionnary object with status and event number
        """
        r={}
        r["state"]=self.state
        r["configuration"]=self.daq_conf.dict()
        if (self._running.is_set()):
            if self.storage:
                r["run"] = self.storage.run
                r["event"]=self.storage.event
            elif self.daqwriter:
                r["run"] = self.runid
                r["event"]=self.daqwriter._event
            
            
        else:
            r["run"] =-1
            r["event"]=-1
        return r

    def running(self):
        return self._running.is_set()

    def get_status(self) -> dict:
        self.status=self.acq_status()
        return dict(self.status, running=self.running())
