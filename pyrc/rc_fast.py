import time
import MongoJob as mg
import json
import os
import logging
import threading
from tabulate import tabulate
from termcolor import colored
from typing import Dict,List, Optional,Any
from pydantic import BaseModel
import requests
from transitions import Machine, State
import mqtt_interface
def print_dict(d, mode="row", tablefmt="grid"):
    """
    Affiche un dictionnaire avec tabulate.

    mode="row"    → affiche le dict comme une seule ligne (colonnes = clés)
    mode="kv"     → affiche le dict en deux colonnes clé/valeur
    """
    if mode == "row":
        print(tabulate([d], headers="keys", tablefmt=tablefmt))
    elif mode == "kv":
        print(tabulate(d.items(), headers=["Clé", "Valeur"], tablefmt=tablefmt))
    else:
        raise ValueError("mode doit être 'row' ou 'kv'")

def pmrTransitionWorker(o,app,transition,res):
    """!thread pmr Transition worker function"""
    s = json.loads(o.sendTransition(app,transition, {}))
    res = s
    logging.info(f'ending {app.name} {transition} with result {res}')
    return
logging.basicConfig(level=logging.INFO,
                    format='(%(threadName)-10s) %(message)s',
                    )


class App(BaseModel):
    host: str
    instance: int
    name: str
    port: int
    params: Dict[str, Any]
    info: Optional[Dict[str, Any]] = None  # Champ optionnel avec typage
    commands:Optional[ List[str]]=None
    allowed:Optional[ List[str]]=None
    transitions:Optional[ List[str]]=None
    
    status: Optional[Dict[str, Any]] = None  # Champ optionnel avec typage
    state: Optional[str] = None  # Champ optionnel avec typage
    model_config = {"extra": "allow"}
    def print(self):
        print(f"App {self.name} instance {self.instance} at {self.host}:{self.port}")
        print("Params:")
        print_dict(self.params)
        if self.info:
            print("Info:")
            print_dict(self.info)
        if self.status:
            print("Status:")
            print_dict(self.status)
        if self.state:
            print(f"State: {self.state}")

class AppConfig(BaseModel):
    apps: List[App]
    pns: str
    session: str
    type: str
    version: int
    mqtt_broker: Optional[str] = None
    state: Optional[str] = "UNKNOWN"  # Champ optionnel avec typage
    model_config = {"extra": "allow"}
    def print(self):
        print(f"Session {self.session} version {self.version} state {self.state}")
        for x in self.apps:
            x.print()

class meta_Transition(BaseModel):
    fsm: List[str]
    commands: List[Any] = []

class meta_AppConfig(BaseModel):
    threaded: int
    transitions: Dict[str, meta_Transition]


class meta_RootConfig(BaseModel):
    apps: Dict[str, meta_AppConfig]  # Modèle précédent pour les apps
    sequences: Dict[str, List[str]]

    
class rc_fast:
    def __init__(self, config):
        """! Inherits from a pmdaqControl
        It additionnally adds an acces to the MongoDB handling the runs collection
        @param config The configuration file
        """

        ##reset time
        self.reset = 0
        ## Comment for a run
        self.comment = "Not yet set"
        ## Setup name
        self.experiment = "UNKNOWN"
        ## MDCC plugin name
        self.md_name = "lyon_mdcc"
        ## MongoDB MongoJob instance
        self.db = mg.instance()

        # Configure MQTT
        self.mqtt=None
        self.broker = os.getenv("MQTT_BROKER", "localhost")
        ### Build the AppConfig
        self.config=None
        self.parse_config(config)
        ## Meta donnees
        with open("/usr/local/pmdaq/etc/rc_meta.json", "r") as f:
            meta_data = json.load(f)

        self.meta_config=meta_RootConfig(**meta_data)


        self.daq_params_file = os.getenv("DAQ_PARAMS_FILE", "UNKNOWN")
        self.daq_params_set = os.getenv("DAQ_PARAMS_SET", "UNKNOWN:UNKNOWN")
        if (self.daq_params_file == "UNKNOWN" or self.daq_params_set == "UNKNOWN:UNKNOWN"):
            print("Warning: DAQ_PARAMS_FILE or DAQ_PARAMS_SET environment variable not set, set_parameters will not work")
            exit(1)

        
       
        # RC state machine
        # Wait MQTT status 
        time.sleep(1)
        # To do xcheck services
        running_daq=self.check_services()
        #it should set self.config.state to UNKNOWN if seervices are missing and trigger a restart
        if not running_daq:
            self.restart()
            print("Wait 10 s ... restarting pmdaq ")
            time.sleep(10)
            self.config.state="UNKNOWN"
        # Configure state machine
        self.configure_state_machine()

    def check_services(self):
        for x in self.config.apps:
            s_rep=executeRequest(f"http://{x.host}:{x.port}/SERVICES")
            if s_rep=='null':
                return False
            o_rep=json.loads(s_rep)
            if not type(o_rep) is list:
                return False
            s_info=f"/{self.config.session}/{x.name}/{x.instance}/INFO"
            if not s_info in o_rep:
                return False
        return True
    
    def configure_state_machine(self):
         ## DAQ Finite State Machine (transitions.Machine)
        self.daqfsm = Machine(model=self, states=[
                              'CREATED', 'INITIALISED', 'CONFIGURED', 'RUNNING', 'CONFIGURED'], initial='CREATED')
        self.daqfsm.add_transition(
            'initialise', 'CREATED', 'INITIALISED', after='daq_initialising', conditions='isConfigured')
        self.daqfsm.add_transition('configure', [
                                   'INITIALISED', 'CONFIGURED'], 'CONFIGURED', after='daq_configuring', conditions='isConfigured')
        self.daqfsm.add_transition(
            'start', 'CONFIGURED', 'RUNNING', after='daq_starting', conditions='isConfigured')
        self.daqfsm.add_transition(
            'stop', 'RUNNING', 'CONFIGURED', after='daq_stopping', conditions='isConfigured')
        self.daqfsm.add_transition(
            'destroy', 'CONFIGURED', 'CREATED', after='daq_destroying', conditions='isConfigured')
        
        # Pour l'instant juste la gestion d'une prise en main a distance
        
        if self.config.state == "UNKNOWN":
            # Creer la session et la publier sur MQTT
            self.create_session()
            
        else:
            self.daqfsm.set_state(self.config.state, model=self)

    def publish_state(self):
        """! Publish the current state of the RC on MQTT
        """
        if self.mqtt:
            self.mqtt.publish(f"pmdaq/{self.config.session}/rc/state", self.state,retain=True)
    def create_session(self):
        """! Create the session on MQTT with the session name and the state CREATED
        """
        self.daqfsm.set_state('CREATED', model=self)
        for x in self.config.apps:
            print(x)
            par={}
            par["session"]=self.config.session
            par["name"]=x.name
            par["instance"]=x.instance
            par["params"]=""
            if (x.params!=None):
                par["params"]=x.params
            executeCMD(x.host,x.port,"/REGISTER",par)
            r=self.sendRequest(x,"INFO")
            print(r)
        self.publish_state() 
    # daq
    def parse_config(self,file_name,debug=False):
        # Charger le JSON
        with open(file_name, "r") as f:
            data = json.load(f)

        # Valider et parser avec Pydantic
        self.config = AppConfig(**data)
        if self.config.mqtt_broker:
            for x in self.config.apps:
                x.params["mqtt_broker"]=self.config.mqtt_broker
            self.broker=self.config.mqtt_broker
        if debug:
            print(f"Session: {self.config.session} version: {self.config.version}")
            for x in self.config.apps:
                print(x.host,x.port,x.instance,x.name,x.params)
            print(self.config.model_dump(mode='json'))
        self.mqtt=mqtt_interface.MQTTInterface(host=self.broker,root_topic=f"pmdaq/{self.config.session}/#")
        self.mqtt.start(self.config)

    def sendRequest(self,app: App,name: str,params=None)->str:
        """
        Access to a command or a transition of a pmdaq service
        
        @param host: Host name
        @param port: Application port
        @param path: The complete PATH of the service session/pluggin/instance/command
        @param params: CGI additional parameters
        @return: url answer as text
        """
        path="/".join([self.config.session,app.name,str(app.instance),name])
        if (params!=None ):            
            lq={}
            for x,y in params.items():
                if (type(y) is dict):
                    y=json.dumps(y).replace(" ","").encode("utf8")
                    #print("STRING ",y)
                lq[x]=y
            try:
                r = requests.get(f"http://{app.host}:{app.port}/{path}", params=lq)
                #print(f"http://{app.host}:{app.port}/{path} with {lq}")
                #print(r.text)
                return r.text
            except requests.exceptions.RequestException as e:
                print(e)
                p_rep={}
                p_rep["STATE"]="DEAD"
                p_rep["http_error"] = e.code
                return json.dumps(p_rep,sort_keys=True)
            return r.text
        else:
            try:
                r = requests.get(f"http://{app.host}:{app.port}/{path}")
                #print(f"http://{app.host}:{app.port}/{path} with no params")
                #print("In sendRequest ",r.text)

                return r.text
            except requests.exceptions.RequestException as e:
                print(e)
                p_rep={}
                p_rep["STATE"]="DEAD"
                p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
    def update_access_info(self,app: App):
        """!
        Update the access information of an app (state, info, params) by sending an INFO command to the plugin service
        @param app The app to update
        """
        self.sendRequest(app,"INFO",None)
    def sendCommand(self, app: App, name: str, content)->str:
        """!
        Send a command to the plugin service
        @param name Command(service) Name
        @param content CGI parameters of the command
        @return The string answer 
        """
        self.update_access_info(app)
        #self.sendRequest(app,"INFO",None)
        isValid = name in app.commands
        if (not isValid):
            return '{"answer":"invalid command ","status":"FAILED"}'
        rep=self.sendRequest(app,name,content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")
        return rep
    def sendTransition(self,app: App, name, content)->str:
        """!
        Send a transition to the plugin service. The transition is checked to be in the ALLOWED list before beeing sent
        @param name Transition(service) Name
        @param content CGI parameters of the command
        @return The string answer  or a FAILED message
        """
        self.update_access_info(app)
        #print "Send Transition",self.procInfos
        isValid = False
        isValid = name in app.allowed
        if (not isValid):
            return '{"answer":"invalid transition","status":"FAILED"}'

        rep=self.sendRequest(app,name,content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")

        # update state (published is asynchronous)
        #time.sleep(10000/1000000.0)
        self.update_access_info(app)
        #print("New State is ",self.state)
        return rep

    # daq
    def process_transition(self,transition_name):
        rep={}
        if not transition_name in self.meta_config.sequences.keys():
            print(f"{transition_name} is not in possibles kest {self.meta_config.sequences.keys()}")
            return rep
        app_list=self.meta_config.sequences[transition_name]
        for app_name in app_list:
            #  The plugin is in the daq
            if not any(d.name == app_name for d in self.config.apps):
                continue
            # the pllugin is defined in meta data
            if not app_name in self.meta_config.apps.keys():
                continue
            #print(app_name)
            #print(self.metadata["apps"][app_name])
            threaded=self.meta_config.apps[app_name].threaded==1
            fsm_list=self.meta_config.apps[app_name].transitions[transition_name].fsm
            cmd_list=self.meta_config.apps[app_name].transitions[transition_name].commands
            # Loop on specific transitions for this transition
            for t in fsm_list:
                list_of_threads=None
                msg=self.build_message(app_name,t)
                print(app_name,t,threaded,f" Message {msg}")
                if not threaded:
                    for a in self.config.apps:
                        if a.name==app_name:
                            s=json.loads(self.sendTransition(a,t,msg))
                            rep[f"{app_name}_{a.instance}"]=s
                    continue
                else:
                    list_of_threads=list()
                for x in self.config.apps:
                    if x.name== app_name:                     
                        rep[f"{app_name}_{x.instance}"]={}
                        thr = threading.Thread(target=pmrTransitionWorker, args=(self,x,t,rep[f"{app_name}_{x.instance}"],))
                        thr.start()
                        list_of_threads.append(thr)

                logging.info(f'{t} Waiting for worker threads')
                alive=True
                while (alive):
                    nalive=False
                    for thr in list_of_threads:
                        nalive=nalive or thr.is_alive()
                        logging.debug("%s %d " % (thr.getName() ,thr.is_alive()))
                        thr.join(1)
                    alive=nalive
            # Now process command list
            for c in cmd_list:
                msg={}
                for x in self.config.apps:
                    if x.name==app_name:                     
                        s = json.loads(self.sendCommand(x,c,msg))
                        rep[f"{app_name}_{x.instance}_{c}"]=s

        self.daq_answer = json.dumps(rep)
        self.publish_state()

    def build_message(self,app_name,transition_name):
        """
        Build a message with run number uniquely for evb_builder 
        """
        m={}
        if (app_name == "evb_builder" and transition_name == "START"):
            if (self.experiment == "UNKNOWN"):
                self.experiment = os.getenv("DAQSETUP", "UNKNOWN")

            jnrun = self.db.getRun(self.experiment, self.comment)
            m['run'] = jnrun['run']
        return m
    def set_parameters(self):
        rep={}
        
        j_params=json.loads(open(self.daq_params_file).read())
        pset=self.daq_params_set.split(":")
        if (not pset[0] in j_params["setups"].keys()):
            print(f"Missing experiment {pset[0]} in file {self.daq_params_file} ({j_params['setups'].keys()})")
            return rep
        if (not pset[1] in j_params["setups"][pset[0]].keys()):
            print(f"Missing parameters set {pset[1]} in {pset[0]} experiment in the file {self.daq_params_file} ({j_params['setups'].keys()})")
            return rep
        p_apps=j_params["setups"][pset[0]][pset[1]]["apps"]
        for x in p_apps:
            if not any(d.name == x["name"] for d in self.config.apps):
                continue

            for a in self.config.apps:
                if a.name== x["name"]:
                    par={}  
                    par["params"]=x["params"]
                    s = json.loads(self.sendCommand(a,"SETPARAMS",par))
                    rep[f"{x['name']}_{a.instance}"]=s
        return rep
        

    # Initialising implementation

    def daq_initialising(self):
       self.process_transition("INITIALISE")
    def daq_configuring(self):
       self.set_parameters()
       self.process_transition("CONFIGURE")
    def daq_stopping(self):
       self.process_transition("STOP")
    def daq_destroying(self):
       self.process_transition("DESTROY")
    def daq_starting(self):
       self.process_transition("START")

    def isConfigured(self):
        return True    

    def update_status(self):
        """! Update the status of the RC by sending a STATUS command to all plugins and updating the state of the RC with the answer
        """
        rep={}
        for x in self.config.apps:
            s = json.loads(self.sendCommand(x,"STATUS",None))
            rep[f"{x.name}_{x.instance}"]=s
        self.daq_answer = json.dumps(rep)

    def print(self):
        """! Print the status of the RC by sending a STATUS command to all plugins and 
        printing the configuration updated by mqtt
        """
        self.update_status()
        self.config.print()

    # RESTART
    def restart(self,url=None):
        """!
        Send an EXIT command to all pmdaq daemon
        @param url If not None send EXIT only to this url
        @warning It is a real restarting of the whole DAQ
        """
        exit_done=[]
        for x in self.config.apps:
            url=f"http://{x.host}:{x.port}/EXIT"
            if not url in exit_done:
                executeRequest(url)
            exit_done.append(url)
            
def executeRequest(url):
    """
    Access to an url
    
   @param surl: The url
   @return: url answer as a text
   """
    try:
        r = requests.get(url)
    except requests.exceptions.RequestException as e:
        print(e)
        p_rep={}
        p_rep["STATE"]="DEAD"
        p_rep["http_error"] = e.code
        return json.dumps(p_rep,sort_keys=True)
    return r.text
    
def executeCMD(host,port,path,params):
    """
        Access to a command or a transition of a pmdaq service
        
        @param host: Host name
        @param port: Application port
        @param path: The complete PATH of the service session/pluggin/instance/command
        @param params: CGI additional parameters
        @return: url answer as text
    """

    if (params!=None ):
        myurl = "http://"+host+ ":%d" % (port)

        lq={}
        for x,y in params.items():
            if (type(y) is dict):
                y=json.dumps(y).replace(" ","").encode("utf8")
                #print("STRING ",y)
            lq[x]=y
        try:
            r = requests.get(myurl+path, params=lq)
        except requests.exceptions.RequestException as e:
            print(e)
            p_rep={}
            p_rep["STATE"]="DEAD"
            p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
        return r.text
    else:
        myurl = "http://"+host+ ":%d%s" % (port,path)
        #print(myurl)
        try:
            r = requests.get(myurl)
        except requests.exceptions.RequestException as e:
            print(e)
            p_rep={}
            p_rep["STATE"]="DEAD"
            p_rep["http_error"] = e.code
            return json.dumps(p_rep,sort_keys=True)
        return r.text
