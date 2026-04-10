from typing import Dict, Any
import time
import MongoJob as mg
import json
import os
import logging
import rc_fast as daq

logging.basicConfig(level=logging.INFO)

class Session:
    def __init__(self, name: str, version: str):
        self.name = name
        self.version = int(version)
        self.config = {}
        self.daq=None

    def restart(self):
        if self.daq:
            self.daq.restart()
            return {"status": "restarted", "status":"remove and reconfigure needed"}
        else:
            return {"status": "missing", "status":f"daq {self.name} v{self.version} is not configured"}

    def parse_settings(self,params: dict):
        params_file=params.get("params_file")
        params_set=params.get("params_set")
        params_dbname=params.get("params_dbname")
        params_dbversion=params.get("params_dbversion")
        file_access= params_file!=None
        db_access =params_dbname!=None and params_dbversion!=None
        if not (file_access or db_access) or not params_set:
            raise ValueError("Missing 'params_file' or 'params_name' or 'params_version' or 'params_set' in parameters")

            
        if db_access:
            self._wdj.downloadParameters(params_dbname,params_dbversion,True)
            params_file=f"/dev/shm/mgparams/{params_dbname}_{params_dbversion}.json"

        self.daq.set_parameters_access(params_file, params_set)
        return f"parameter '{params_file}' access set to '{params_set}'"

    def configure(self, params: dict):
        self.config.update(params)
        self._wdj=mg.instance()
        try:
            self._wdj.downloadConfig(self.name,self.version,True)
        except:
            return {"status":"failed"}
        self.conf_file=f"/dev/shm/mgjob/{self.name}_{self.version}.json"
        self.daq=daq.rc_fast(self.conf_file)
        # parameters
        have_parameters=params.get("params_dbname") or params.get("params_file")
        if not have_parameters:
            return {"status": "configured", "file":self.conf_file}
        else:
            rc=self.parse_settings(params)
            return {"status": "configured", "file":self.conf_file,"params":rc}
        
    def execute(self, command_type: str, params: dict):
        # dispatcher simple
        if not self.daq:
            raise ValueError(f"daq {self.name} v{self.version} is not configured for transition")
        valid=["pause","resume","set_parameter_access","set_parameter_db","status","app_command","set_comment"]
        if not command_type in valid:
            raise ValueError(f"Invalid command '{command_type}' for state '{self.daq.state}' (valid: {valid})")


        if command_type == "status":
            self.daq.update_status()
            time.sleep(0.1)  # simuler un délai de traitement
            #return {f"status:{self.daq.config.to_dict()}"}
            return self.daq.config.to_dict()
        if command_type == "pause":
            self.daq.pause()
            return {"status": "paused"}
        if command_type == "resume":
            self.daq.resume()
            return {"status": "resumed"}
        if command_type == "set_parameter_access":
            rc=self.parse_settings(params)
            return {"status":rc}

        if command_type == "set_comment":
            rc=self.daq.run_comment=params.get("comment")
            return {"status":rc}

        if command_type == "app_command":
            app_name=params.get("app_name")
            cmd_name=params.get("cmd_name")
            cmd_params=params.get("cmd_params")
            if not app_name or not cmd_name or not cmd_params:
                raise ValueError("Missing 'app_name' or 'cmd_name' or 'cmd_params' in parameters")
            answer=self.daq.namedCommand(app_name, cmd_name, cmd_params)
            return {"status": f"command '{cmd_name}' sent to app '{app_name}': {answer}"}
            
        raise ValueError(f"Not handled command '{command_type}'")
    
    def transition(self, transition_type: str):
        # dispatcher simple
        if not self.daq:
            raise ValueError(f"daq {self.name} v{self.version} is not configured for transition")
        valid=self.daq.daqfsm.get_triggers(self.daq.state)
        if not transition_type in valid:
            raise ValueError(f"Invalid transition '{transition_type}' for state '{self.daq.state}' (valid: {valid})")

        if transition_type == "initialise":
            self.daq.initialise()
            return {"result": f"{self.name} v{self.version} initialised"}
        if transition_type == "configure":
            if self.daq.daq_params_file ==  "UNKNOWN":
                    raise ValueError(f"daq {self.name} v{self.version} has no valid config file for configure transition")
            self.daq.configure()  
            return {"result": f"{self.name} v{self.version} configured"}
        if transition_type == "start":
            self.daq.start()
            return {"result": f"{self.name} v{self.version} started"}
        if transition_type == "stop":
            self.daq.stop()
            return {"result": f"{self.name} v{self.version} stopped"}
        if transition_type == "destroy":
            self.daq.destroy()
            return {"result": f"{self.name} v{self.version} destroyed"}

        raise ValueError(f"Not yet handled'{transition_type}'")
