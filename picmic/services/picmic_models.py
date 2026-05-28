

import threading
import time
import logging
import time
#from turtle import setup
import numpy as np
from picmic_schema import *
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
    """Load a configuration from a json file and create a picmic_physic object with it
    Args:
        f_config (str): path to the json configuration file
    Returns:
        picmic_physic: a picmic_physic object with the configuration loaded
    """
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
        """ Execute a method of the class with the given parameters
        Args:
            nom (str): name of the method to execute
            params (dict): a dictionnary with the parameters to pass to the method

        Returns:
            The result of the method execution
        """
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
    def set_db_configuration(self,name:str,version:int) -> None:
        """ Download and set a new DB configuration for the setup
        Args:
            name (str): name of the configuration to set
            version (int): version of the configuration to set
        """
        self.sdb.download_configuration(name,version)
        c=json.loads(open(f"/dev/shm/config/{name}_{version}.json").read())
        print(c)
        self.set_configuration(c)        
    def set_configuration(self,c:Dict[str, object]) -> None:
        """Set the configuration of the setup
        Args:
            c (Dict[str, object]): a dictionnary with the configuration to set
        """
        self.config_dict=c
        self.config=PicConfig.model_validate(c)
        self.daq_conf=self.config.configuration_list[self.config.configuration]
    def store_configuration(self,name:str,version:int,comment="Comment is missing")-> None:
        """Store the configuration in the DB
        Args:
            name (str): name of the configuration to store
            version (int): version number of the configuration to store
            comment (str): a comment to be stored with the configuration
        """
        fname="/tmp/%s_%s.json" % (self.config.name,self.config.version)
        f=open(fname,"w+")
        f.write(json.dumps(self.conf, indent=2, sort_keys=True))
        f.close()
        self.sdb.upload_configuration(fname,comment)
    def daq_destroying(self) -> bool:
        """ Destroy the setup
            It closes the connection to the KC705 and the access to the DB
        """
        return True

    def daq_initialising(self) -> None:
        """ Now initialize the setup
            It creates the access to the DB, and open the connection to the KC705
        """
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

    def change_threshold(self,thr:int)->None:
        """ Change dac10b threshold
        Args:
            thr (int); The 10bit DAC threshold
        """
        self.daq_conf.threshold= thr

    def change_filtering(self,on:int)->None:
        """Change the filtering configuration
        Args:            on (int): 1 for filtering on, 0 for filtering off
        """
        self.daq_conf.filtering=(on!=0)

    def change_faling(self,on:int)->None:
        """Change the falling edge configuration
        Args:
            on (int): 1 for falling edge, 0 for rising edge
        """
        self.daq_conf.falling=(on!=0)
    def change_val_evt(self,on:int)->None:
        """Change the ValEvt value  
        Args:
            on (int): The ValEvt value to set (0 or 1)
        """      
        self.daq_conf.val_evt=on

    def change_dc_pa(self,on:int)->None:
        """Change the DC_PA value for all channels
        Args:
            on (int): The DC_PA value to set (0 to 15)
        """
        self.daq_conf.dc_pa=on
    def change_polarity(self,on:int)->None:
        """Change the polarity of the signal
        Args:
            on (int): 1 for positive, 0 for negative
        """
        self.daq_conf.pol_neg=on
        
    def daq_starting(self,location=None,comment=None,params={"type":"NORMAL"}) -> None:
        """ Now start the acquisition
        It starts the acquisition thread and store the data in the configured storage
        """
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
            self.scurve_run(location=location, comment=comment)
        else:
            raise ValueError(f"Unknown run type {self.daq_conf.type}")        
    def normal_loop(self):
        """ Normal acquisition loop, it runs until stop is called
        """
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
        """ Time loop acquisition, it runs a loop over a parameter (voltage for example) and acquire a fixed number of events for each step
        The parameters of the loop are defined in the configuration file
        """
        with self._lock:
            if self._thread and self._thread.is_alive():
                self.logger.warning("Acquisition déjà en cours please use stop before")
                return False
            self._running.set()
            self._thread = threading.Thread(target=self.time_loop, args=(params,), daemon=True)
            self._thread.start()
            return True
        
    def init_pulser(self,fname="/opt/pmdaq/picmic/etc/pulse_config.json"):
        """ Initialize the pulser with the given configuration file
        Args:
            fname (str): path to the pulser configuration file
        """
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
        """ Time loop acquisition, it runs a loop over a parameter (voltage for example) and acquire a fixed number of events for each step
        The parameters of the loop are defined in the configuration file
        """
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
        """ Check if the acquisition is running
        Returns:
            True if the acquisition is running, False otherwise
        """
        return self._running.is_set()

    def get_status(self):
        """ Get the status of the acquisition
        Returns:
            A dictionnary with the status of the acquisition
        """
        with self._lock:
            return dict(self.status, running=self.running())

    def setup_injection(self,vout,rise=1.0E-9,delay=120E-9,use_ctest=False):
        """ Setup the pulser for the injection
        Args:
            vout (float): voltage to set on the pulser (0 to 1.0 V if ctest is used, 0 to 6 V otherwise)
            rise (float): rise time of the pulse in seconds
            delay (float): delay of the pulse in seconds
            use_ctest (bool): if True, the pulse will be sent to the CTest pin, otherwise it will be sent to the regular injection pin
        """
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
        """ Acquire data from the KC705 and store it in the storage
        Args:
            n_acq (int): number of acquisitions to perform
            window_size (int): size of the acquisition window in ns
            dead_time (int): dead time between acquisitions in ns
            n_window (int): number of windows to acquire for each acquisition
        """
        for i in range(n_acq):
            words = self.kc705.acqPtdc(window=window_size, deadtime=dead_time, window_number=n_window)
            # File storage
            self.storage.writeEvent(words)
        return
    
    def daq_stopping(self, params=None):
        """ Now stop the acquisition
        It stops the acquisition thread and close the storage
        """
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

    # S-Curve methods

    def sweep_dac10b(self, start, stop, step):
        """Sweep the 10-bit DAC and measure counts for each value
        Args:
            start (int): starting DAC value
            stop (int): stopping DAC value
            step (int): step size for the sweep
        Returns:
            list: scurves - list of count arrays for each channel (64 channels)
        """
        scurves = [[] for _ in range(64)]
        for dac10b_val in range(start, stop, step):
            self.feb.liroc.set10bDac(dac10b_val)
            self.feb.liroc.stopScClock()
            counters = self.kc705.acqPtdcCounters(window=5, deadtime=500, window_number=1000)
            for ch, counter in enumerate(counters):
                scurves[ch].append(counter)
        return scurves

    def scurve_single_chan(self, lichan, start=450, stop=750, step=1, dac_loc=32):
        """Get S-curve for a single channel
        Args:
            lichan (int): LIROC channel index (0-63)
            start (int): starting threshold value
            stop (int): stopping threshold value
            step (int): step size
            dac_loc (int): local DAC value to set
        Returns:
            list: s-curve data for the selected channel
        """
        for ch in range(64):
            if dac_loc != 0:
                self.feb.liroc.setChannelDac(ch, dac_loc)
            self.feb.liroc.maskChannel(ch, ch != lichan)
        scurve = self.sweep_dac10b(start, stop, step)[daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[lichan]]
        return scurve

    def scurve_all_channels(self, start=450, stop=750, step=1, dac_loc=32):
        """Get S-curves for all channels
        Args:
            start (int): starting threshold value
            stop (int): stopping threshold value
            step (int): step size
            dac_loc (int): local DAC value to set
        Returns:
            list: scurves for all 64 channels
        """
        for ch in range(64):
            if dac_loc != 0:
                self.feb.liroc.setChannelDac(ch, dac_loc)
            if ch > -1:
                self.feb.liroc.maskChannel(ch, False)
            else:
                self.feb.liroc.maskChannel(ch, True)
        scurves = self.sweep_dac10b(start, stop, step)
        return scurves

    def scurve_loop_one(self, start=450, stop=750, step=1, dac_loc=32):
        """Get S-curves for all channels one at a time
        Args:
            start (int): starting threshold value
            stop (int): stopping threshold value
            step (int): step size
            dac_loc (int): local DAC value to set
        Returns:
            list: scurves for all channels
        """
        scurves = [[] for _ in range(64)]
        if not hasattr(self, 'status'):
            self.status = {}
        self.status["scurve"] = scurves
        for ch in range(64):
            if hasattr(self, '_running') and (not self._running.is_set()):
                break
            if ch not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN:
                continue
            scurves[ch] = self.scurve_single_chan(ch, start, stop, step, dac_loc)
            self.logger.info(f"Channel {ch} scurve data: {scurves[ch]}")
        return scurves

    def scurve_one_channel(self, index, thmin, thmax, thstep=1, dac_loc=64):
        """Get an optimized S-curve for one channel with adaptive stepping
        Args:
            index (int): channel index (0-63)
            thmin (int): minimum threshold value
            thmax (int): maximum threshold value
            thstep (int): threshold step size
            dac_loc (int): local DAC value
        Returns:
            int: optimized threshold value (turn-on point)
        """
        # First rough scan with larger steps
        scurve2 = self.scurve_single_chan(index, start=thmin, stop=thmax, step=thstep, dac_loc=dac_loc)
        scurve2_th = [i for i in range(thmin, thmax, thstep)]

        # Find the transition point
        to_0 = thmin
        cmax1 = max(scurve2)
        cmax_index = scurve2.index(max(scurve2))
        
        # Find 70% point
        to_1i = to_0
        for t in range(cmax_index, len(scurve2)):
            if scurve2[t] < 0.7 * cmax1:
                to_1i = scurve2_th[t]
                break
        
        # Find 30% point
        to_1a = to_1i
        for t in range(cmax_index, len(scurve2)):
            if scurve2[t] < 0.3 * cmax1:
                to_1a = scurve2_th[t]
                break
        
        to_1 = (to_1a + to_1i) // 2
        self.logger.info(f"Channel {index}: Seuil raw {to_0}, Seuil fin {to_1}")
        return to_1

    def calib_dac_local(self, thi, tha):
        """Calibrate the local DAC values for all channels
        Args:
            thi (int): minimum threshold to scan
            tha (int): maximum threshold to scan
        Returns:
            tuple: (target_threshold, list of optimized DAC values for each channel)
        """
        turn_on = [0 for i in range(64)]
        v6 = [0 for i in range(64)]
        
        for idx in range(64):
            v6[idx] = self.feb.liroc.getParam(f'DAC_local_ch{idx}')
            if idx not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN:
                continue
            to = self.scurve_one_channel(idx, thi, tha, dac_local=0)
            turn_on[idx] = to

        self.logger.info(f"Turn ON values: {turn_on}")
        nto = np.array(turn_on)
        target = round(np.median(nto))
        self.logger.info(f"Median target: {target}")

        # Check minimal gain value
        too_low = False
        too_high = False
        vexp = [v6[i] for i in range(64)]
        
        for idx in range(64):
            gc = v6[idx]
            if turn_on[idx] != 0:
                gc = v6[idx] + round((target - turn_on[idx]) / 2.0)
                vexp[idx] = max(1, min(127, gc))
            else:
                continue
            too_low = too_low or (gc < 3)
            too_high = too_high or (gc > 1020)

        if too_low:
            target = target + 10
        if too_high:
            target = target - 10

        self.logger.info(f"Final target: {target}, DAC values: {vexp}")
        return target, vexp

    def calib_iterative_dac_local(self, thi, tha):
        """Iteratively calibrate the local DAC values for all channels
        Uses two-step refinement: coarse scan (step=5) then fine scan (step=1)
        Args:
            thi (int): minimum threshold to scan
            tha (int): maximum threshold to scan
        Returns:
            tuple: (target_threshold, list of optimized DAC values for each channel)
        """
        turn_on = [0 for i in range(64)]
        v6 = [0 for i in range(64)]
        self.status["raw_turnon"] = [0 for i in range(64)]

        # First pass: coarse scan, second pass: fine scan
        for idx in range(64):
            if hasattr(self, '_running') and (not self._running.is_set()):
                break
            
            v6[idx] = self.feb.liroc.getParam(f'DAC_local_ch{idx}')
            if idx not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN:
                continue
            
            # Coarse scan with step=5
            to_0 = self.scurve_one_channel(idx, thi, tha, thstep=5, dac_loc=0)
            ti0 = round(max(to_0 - 15, thi))
            ta0 = round(min(to_0 + 15, tha))
            
            # Fine scan with step=1
            to_1 = self.scurve_one_channel(idx, ti0, ta0, thstep=1, dac_loc=0)
            turn_on[idx] = to_1
            self.status["raw_turnon"][idx] = to_1

        self.logger.info(f"Turn ON values: {turn_on}")
        nto = np.array(turn_on)
        target = round(np.median(nto))
        self.logger.info(f"Median target: {target}")
        self.status["target"] = target

        # Check minimal gain value
        too_low = False
        too_high = False
        vexp = [v6[i] for i in range(64)]

        for idx in range(64):
            if hasattr(self, '_running') and (not self._running.is_set()):
                break
            gc = v6[idx]
            if turn_on[idx] != 0:
                gc = v6[idx] + round((target - turn_on[idx]) / 2.0)
                vexp[idx] = max(1, min(127, gc))
            else:
                continue
            too_low = too_low or (gc < 3)
            too_high = too_high or (gc > 127)

        if too_low:
            target = target + 10
        if too_high:
            target = target - 10

        self.logger.info(f"Final target: {target}, v6: {v6}, vexp: {vexp}")
        self.status["target"] = target
        self.status["dac_local"] = [0 for i in range(64)]

        # Fine-tune each channel's DAC value
        for idx in range(64):
            if hasattr(self, '_running') and (not self._running.is_set()):
                break
            if idx not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN:
                continue
            
            dac_map = {}
            gg = vexp[idx]
            
            # Iterative refinement up to 10 iterations
            for _ in range(10):
                self.feb.liroc.setParam(f'DAC_local_ch{idx}', gg)
                to_fine = self.scurve_one_channel(idx, target - 35, target + 35, thstep=1, dac_loc=0)
                self.logger.info(f"DAC {gg} channel {idx} TO {to_fine} diff {target - to_fine}")
                dac_map[gg] = abs(target - to_fine)
                
                ngg = gg + round((target - to_fine) / 2.0)
                if ngg < 3:
                    ngg = 3
                if ngg > 125:
                    ngg = 125
                if abs(ngg - gg) < 1:
                    break
                gg = ngg

            self.logger.info(f"Channel {idx} DAC scan results: {dac_map}")
            op_dac = min(dac_map, key=dac_map.get)
            self.logger.info(f"Channel {idx} optimized DAC: {op_dac}")
            self.status["dac_local"][idx] = op_dac
            vexp[idx] = op_dac

        return target, vexp

    # S-Curve loop methods

    def align(self, thmin, thmax, location=None, comment=None):
        """Perform threshold alignment calibration
        Calibrates the DAC local values and global threshold using iterative s-curve measurements
        Args:
            thmin (int): Minimum threshold value for scan
            thmax (int): Maximum threshold value for scan
            location (str): Database location (optional)
            comment (str): Comment for database upload (optional)
        Returns:
            tuple: (target_threshold, list of DAC values) or (None, None) if failed
        """
        self.status["method"] = "aligning"
        self.logger.info(f"Starting alignment calibration: thmin={thmin}, thmax={thmax}")
        
        try:
            # Perform iterative DAC local calibration
            target, v_dac_local = self.calib_iterative_dac_local(thmin, thmax)
            
            self.logger.info(f"Alignment target: {target}")
            self.logger.info(f"DAC local values: {v_dac_local}")
            
            # Update the database with calibrated values
            for ich in range(len(v_dac_local)):
                self.sdb.setup.boards[0].picmic.set("DAC_local_ch", v_dac_local[ich], ich)
                self.logger.info(f"DAC_local_ch{ich} = {v_dac_local[ich]}")
            
            # Set the global threshold
            tlsb = target & 0xFF
            tmsb = (target >> 8) & 0xFF
            self.sdb.setup.boards[0].picmic.set("dac_threshold_lsb", tlsb)
            self.sdb.setup.boards[0].picmic.set("dac_threshold_msb", tmsb)
            self.logger.info(f"Threshold set to {target} (LSB={tlsb}, MSB={tmsb})")
            
            # Check if alignment was stopped
            if hasattr(self, '_running') and not self._running.is_set():
                self.logger.info("Alignment was stopped before the end")
                return None, None
            
            # Upload changes to database or save to CSV
            if location and comment:
                try:
                    self.sdb.setup.version = self.daq_conf.db.version
                    self.sdb.upload_changes(comment)
                    self.logger.info(f"Alignment changes uploaded to DB with comment: {comment}")
                except Exception as e:
                    self.logger.warning(f"Could not upload alignment to DB: {e}")
                    self.sdb.setup.version = 999
                    self.sdb.to_csv_files()
            else:
                self.sdb.setup.version = 999
                self.sdb.to_csv_files()
                self.logger.info("Alignment results saved to CSV files")
            
            return target, v_dac_local
            
        except Exception as e:
            self.logger.error(f"Error during alignment: {e}")
            import traceback
            traceback.print_exc()
            return None, None

    def start_scurves(self, params={"analysis": "SCURVE_A", "plot_fig": None, "thmin": None, "thmax": None, "thstep": 1, "location": None, "comment": None, "do_align": False}):
        """Start S-curve measurement in a separate thread
        Args:
            params (dict): Parameters for the S-curve measurement
                - analysis (str): Type of analysis "SCURVE_A" (all channels) or "SCURVE_1" (one by one)
                - plot_fig (matplotlib.figure.Figure): Matplotlib figure for plotting (optional)
                - thmin (int): Minimum threshold value
                - thmax (int): Maximum threshold value
                - thstep (int): Threshold step size
                - location (str): Database location
                - comment (str): Comment for the database
                - do_align (bool): Whether to perform alignment calibration before s-curves
        Returns:
            bool: True if started successfully, False if already running
        """
        if self._running.is_set():
            self.logger.warning("S-curve measurement already in progress")
            return False
        
        if self._thread:
            self._thread.join(timeout=10)
        if self._thread and self._thread.is_alive():
            self.logger.warning("S-curve measurement déjà en cours")
            return False
        
        self._running.set()
        self.status = {}
        self._thread = threading.Thread(target=self.scurve_loop, args=(params,), daemon=True)
        self._thread.start()
        return True

    def scurve_loop(self, params=None):
        """Execute S-curve measurement with analysis and storage
        Args:
            params (dict): Parameters for the S-curve measurement
                - analysis (str): Type of analysis "SCURVE_A" or "SCURVE_1"
                - plot_fig (matplotlib.figure.Figure): Matplotlib figure for plotting
                - thmin (int): Minimum threshold
                - thmax (int): Maximum threshold
                - thstep (int): Step size
                - location (str): Database location
                - comment (str): Database comment
                - do_align (bool): Whether to perform alignment calibration before s-curves
        """
        if params is None:
            params = {}
        
        analysis = params.get("analysis", "SCURVE_A")
        plot_fig = params.get("plot_fig", None)
        thmin = params.get("thmin", self.daq_conf.threshold - 100 if self.daq_conf.threshold else 700)
        thmax = params.get("thmax", self.daq_conf.threshold + 100 if self.daq_conf.threshold else 900)
        thstep = params.get("thstep", 1)
        location = params.get("location", None)
        comment = params.get("comment", None)
        do_align = params.get("do_align", False)
        
        self.logger.info(f"Starting S-curve measurement: analysis={analysis}, thmin={thmin}, thmax={thmax}, thstep={thstep}, do_align={do_align}")
        
        # Initialize results dictionary
        res = {}
        res["state"] = self.daq_conf.db.state
        res["version"] = self.daq_conf.db.version
        res["feb"] = self.daq_conf.db.board
        res["thmin"] = thmin
        res["thmax"] = thmax
        res["thstep"] = thstep
        res["asic"] = "LIROC"
        res["ctime"] = time.time()
        res["analysis"] = analysis
        res["channels"] = []
        
        if location:
            res["location"] = location
        
        self.status["method"] = analysis
        scurves = None
        ax = None
        
        try:
            # Perform alignment if requested
            if do_align:
                self.logger.info("Performing alignment calibration before S-curves")
                align_thmin = params.get("align_thmin", thmin)
                align_thmax = params.get("align_thmax", thmax)
                target, v_dac_local = self.align(align_thmin, align_thmax, location, comment)
                if target is None:
                    self.logger.warning("Alignment failed, but continuing with S-curves")
                time.sleep(1.0)
            
            # Check the analysis type
            if analysis == "SCURVE_A":
                self.logger.info(f"Running SCURVE_A: start={thmin}, stop={thmax}, step={thstep}")
                scurves = self.scurve_all_channels(start=thmin, stop=thmax, step=thstep, dac_loc=0)
            elif analysis == "SCURVE_1":
                self.logger.info(f"Running SCURVE_1: start={thmin}, stop={thmax}, step={thstep}")
                scurves = self.scurve_loop_one(start=thmin, stop=thmax, step=thstep, dac_loc=0)
            else:
                self.logger.error(f"Unknown analysis type: {analysis}")
                self._running.clear()
                return False
            
            time.sleep(1.0)
            
            # Plot results if figure provided
            if plot_fig is not None:
                plot_fig.clear()
                ax = plot_fig.add_subplot(111)
            
            # Process results for each channel
            for liroc_chan in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN.keys():
                if not self._running.is_set():
                    self.logger.info("S-curve measurement was stopped before completion")
                    break
                
                ptdc_chan = daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[liroc_chan]
                rc = {}
                rc["prc"] = liroc_chan
                rc["tdc"] = ptdc_chan
                
                if len(scurves[ptdc_chan]) == 0:
                    continue
                
                rc["scurve"] = scurves[ptdc_chan]
                res["channels"].append(rc)
                
                # Plot the S-curve
                th_range = list(range(thmin, thmax, thstep))
                if plot_fig is None:
                    plt.plot(th_range, scurves[ptdc_chan], '+-', label=f"ch{liroc_chan}")
                else:
                    ax.plot(th_range, scurves[ptdc_chan], '+-', label=f"ch{liroc_chan}")
            
            if plot_fig is None:
                plt.grid()
                plt.legend(loc="upper right")
                plt.xlabel("Threshold (DAC value)")
                plt.ylabel("Counts")
                plt.title(f"S-curves {analysis}")
                plt.show()
            else:
                ax.grid()
                ax.legend(loc="upper right")
                ax.set_xlabel("Threshold (DAC value)")
                ax.set_ylabel("Counts")
                ax.set_title(f"S-curves {analysis}")
                self.logger.info(f"Plot figure updated: {plot_fig}")
            
            if not self._running.is_set():
                self.logger.info("S-curve measurement was stopped before the end")
                return True
            
            # Get run ID from database or user input
            runid = None
            if location and comment:
                try:
                    runobj = self.sdb.getRun(location, comment)
                    runid = runobj['run']
                except Exception as e:
                    self.logger.warning(f"Could not get run ID from DB: {e}")
            
            if runid is None:
                runid = int(input("Enter a run number: "))
            
            res["runid"] = runid
            
            # Store results in JSON
            res_dir = f'/tmp/results/{self.daq_conf.db.state}_{self.daq_conf.db.version}_f_{self.daq_conf.db.board}'
            os.system(f"mkdir -p {res_dir}")
            
            fout = open(f"{res_dir}/scurves_{analysis}_{runid}.json", "w")
            fout.write(json.dumps(res, indent=2))
            fout.close()
            
            self.logger.info(f"S-curve results saved to {res_dir}/scurves_{analysis}_{runid}.json")
            
            # Upload results to database if location and comment provided
            if location and comment:
                try:
                    self.sdb.upload_results(
                        runid,
                        location,
                        res["state"],
                        res["version"],
                        res["feb"],
                        res["analysis"],
                        res,
                        comment
                    )
                    self.logger.info(f"S-curve results uploaded to DB with run ID {runid}")
                except Exception as e:
                    self.logger.warning(f"Could not upload results to DB: {e}")
            
            return True
            
        except Exception as e:
            self.logger.error(f"Error during S-curve measurement: {e}")
            import traceback
            traceback.print_exc()
            return False
        finally:
            self._running.clear()

    def scurve_run(self, location=None, comment=None):
        """Execute S-curve run with parameter validation from configuration
        This method is called from daq_starting() for ScurveRunConfig
        It validates all necessary parameters are available in self.daq_conf before starting
        
        Args:
            location (str): Database location (optional)
            comment (str): Database comment (optional)
        Returns:
            bool: True if started successfully, False otherwise
        """
        # Verify that daq_conf is ScurveRunConfig
        if not isinstance(self.daq_conf, ScurveRunConfig):
            self.logger.error(f"daq_conf is not ScurveRunConfig, it's {type(self.daq_conf)}")
            return False
        
        # Extract parameters from configuration with validation
        try:
            # Required parameters
            thmin = self.daq_conf.thmin
            thmax = self.daq_conf.thmax
            thstep = self.daq_conf.thstep
            calibration = self.daq_conf.calibration
            
            # Optional parameters with defaults
            dc_pa = self.daq_conf.dc_pa if self.daq_conf.dc_pa else 0
            
            # Validate threshold values
            if thmin is None or thmax is None or thstep is None:
                self.logger.error(f"Invalid threshold parameters: thmin={thmin}, thmax={thmax}, thstep={thstep}")
                return False
            
            if thmin >= thmax:
                self.logger.error(f"Invalid threshold range: thmin ({thmin}) >= thmax ({thmax})")
                return False
            
            if thstep <= 0:
                self.logger.error(f"Invalid threshold step: thstep ({thstep}) must be > 0")
                return False
            
            self.logger.info(f"S-curve parameters: thmin={thmin}, thmax={thmax}, thstep={thstep}, calibration={calibration}")
            
            # Build parameters dictionary for start_scurves()
            scurve_params = {
                "thmin": thmin,
                "thmax": thmax,
                "thstep": thstep,
                "location": location,
                "comment": comment,
                "plot_fig": None,
                "do_align": False,
                "analysis": "SCURVE_A"  # default
            }
            
            # Determine analysis type based on calibration mode
            if calibration == "ALIGN":
                self.logger.info("Calibration mode: ALIGN")
                # For ALIGN mode, we do the alignment only
                if location and comment:
                    target, v_dac_local = self.align(thmin, thmax, location, comment)
                else:
                    target, v_dac_local = self.align(thmin, thmax)
                if target is None:
                    self.logger.error("Alignment failed")
                    return False
                self.logger.info(f"Alignment completed successfully: target={target}")
                return True
            
            elif calibration == "SCURVE_A":
                self.logger.info("Calibration mode: SCURVE_A (all channels)")
                scurve_params["analysis"] = "SCURVE_A"
            
            elif calibration == "SCURVE_1":
                self.logger.info("Calibration mode: SCURVE_1 (one by one)")
                scurve_params["analysis"] = "SCURVE_1"
            
            else:
                self.logger.error(f"Unknown calibration mode: {calibration}")
                return False
            
            # Start the S-curve measurement
            self.logger.info(f"Starting S-curve measurement with parameters: {scurve_params}")
            success = self.start_scurves(scurve_params)
            
            if not success:
                self.logger.error("Failed to start S-curve measurement")
                return False
            
            self.logger.info("S-curve measurement started successfully")
            return True
            
        except AttributeError as e:
            self.logger.error(f"Missing configuration parameter: {e}")
            return False
        except Exception as e:
            self.logger.error(f"Error in scurve_run: {e}")
            import traceback
            traceback.print_exc()
            return False

    
