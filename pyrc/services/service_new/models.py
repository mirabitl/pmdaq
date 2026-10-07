import time

import MongoJob as mg
import rc_fast as daq


class DbAccess:
    """Small adapter around the existing MongoJob API."""

    def __init__(self):
        self._job = mg.instance()

    def configurations(self):
        return self._job.configurations(do_json=True)

    def parameters(self):
        return self._job.parameters(do_json=True)

    def parameter_info(self, name: str, version: int):
        return self._job.parametersInfo(name, version, do_json=True)

    def runs(self, experiment: str):
        return self._job.runs(experiment=experiment, do_json=True)

    def download_config(self, name: str, version: int):
        return self._job.downloadConfig(name, version, True)

    def download_parameters(self, name: str, version: int):
        return self._job.downloadParameters(name, version, True)


class Session:
    def __init__(self, name: str, version: str, db: DbAccess):
        self.name = name
        self.version = int(version)
        self.config = {}
        self.daq = None
        self._db = db

    def restart(self):
        if not self.daq:
            return {"status": "missing", "message": f"daq {self.name} v{self.version} is not configured"}

        self.daq.restart()
        return {"status": "restarted", "message": "remove and reconfigure needed"}

    def parse_settings(self, params: dict):
        params_file = params.get("params_file")
        params_set = params.get("params_set")
        params_dbname = params.get("params_dbname")
        params_dbversion = params.get("params_dbversion")
        file_access = params_file is not None
        db_access = params_dbname is not None and params_dbversion is not None

        if not (file_access or db_access) or not params_set:
            raise ValueError(
                "Missing 'params_file' or 'params_dbname' and 'params_dbversion' or 'params_set'"
            )

        if db_access:
            self._db.download_parameters(params_dbname, params_dbversion)
            params_file = f"/dev/shm/mgparams/{params_dbname}_{params_dbversion}.json"

        self.daq.set_parameters_access(params_file, params_set)
        return f"parameter '{params_file}' access set to '{params_set}'"

    def configure(self, params: dict):
        self.config.update(params)
        try:
            self._db.download_config(self.name, self.version)
        except Exception:
            return {"status": "failed"}

        self.conf_file = f"/dev/shm/mgjob/{self.name}_{self.version}.json"
        self.daq = daq.rc_fast(self.conf_file)

        if not (params.get("params_dbname") or params.get("params_file")):
            return {"status": "configured", "file": self.conf_file}

        settings = self.parse_settings(params)
        return {"status": "configured", "file": self.conf_file, "params": settings}

    def execute(self, command_type: str, params: dict):
        if not self.daq:
            raise ValueError(f"daq {self.name} v{self.version} is not configured")

        valid = ["pause", "resume", "set_parameter_access", "set_parameter_db", "status", "app_command", "set_comment"]
        if command_type not in valid:
            raise ValueError(f"Invalid command '{command_type}' for state '{self.daq.state}' (valid: {valid})")

        if command_type == "status":
            self.daq.update_status()
            time.sleep(0.1)
            return self.daq.config.to_dict()
        if command_type == "pause":
            self.daq.pause()
            return {"status": "paused"}
        if command_type == "resume":
            self.daq.resume()
            return {"status": "resumed"}
        if command_type in ("set_parameter_access", "set_parameter_db"):
            return {"status": self.parse_settings(params)}
        if command_type == "set_comment":
            self.daq.run_comment = params.get("comment")
            return {"status": self.daq.run_comment}
        if command_type == "app_command":
            app_name = params.get("app_name")
            cmd_name = params.get("cmd_name")
            cmd_params = params.get("cmd_params")
            if not app_name or not cmd_name or cmd_params is None:
                raise ValueError("Missing 'app_name', 'cmd_name' or 'cmd_params' in parameters")
            answer = self.daq.namedCommand(app_name, cmd_name, cmd_params)
            return {"status": f"command '{cmd_name}' sent to app '{app_name}': {answer}"}

        raise ValueError(f"Not handled command '{command_type}'")

    def transition(self, transition_type: str):
        if not self.daq:
            raise ValueError(f"daq {self.name} v{self.version} is not configured")

        valid = self.daq.daqfsm.get_triggers(self.daq.state)
        if transition_type not in valid:
            raise ValueError(f"Invalid transition '{transition_type}' for state '{self.daq.state}' (valid: {valid})")

        transitions = {
            "initialise": (self.daq.initialise, "initialised"),
            "start": (self.daq.start, "started"),
            "stop": (self.daq.stop, "stopped"),
            "destroy": (self.daq.destroy, "destroyed"),
        }
        if transition_type == "configure":
            if self.daq.daq_params_file == "UNKNOWN":
                raise ValueError(f"daq {self.name} v{self.version} has no valid config file for configure transition")
            self.daq.configure()
            result = "configured"
        elif transition_type in transitions:
            action, result = transitions[transition_type]
            action()
        else:
            raise ValueError(f"Not yet handled '{transition_type}'")

        return {"result": f"{self.name} v{self.version} {result}"}