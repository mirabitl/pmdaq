from typing import Dict
from models import Session,DbAccess

class DbService:
    def __init__(self):
        self.db=DbAccess()
    def list_configurations(self):
        return self.db.configurations()
    def list_parameters(self):
        return self.db.parameters_set()
    def list_setups(self,params: dict):
         set_name=params.get("name")
         set_version=params.get("version")
         if not set_name:
             raise ValueError("Missing 'name' or 'version' in parameters")
         return self.db.parameters_set_info(set_name,set_version)
    def list_runs(self,params: dict):
         experiment=params.get("experiment")
         if not experiment:
             raise ValueError("Missing 'experiment' in parameters")
         return self.db.runs(experiment)
         
class AppService:
    def __init__(self):
        self.apps: Dict[str, Dict[str, Session]] = {}

    def create_app(self, name: str, version: str):
        if name not in self.apps:
            self.apps[name] = {}

        if version in self.apps[name]:
            raise ValueError("App version already exists")

        self.apps[name][version] = Session(name, version)

    def get_app(self, name: str, version: str) -> Session:
        try:
            return self.apps[name][version]
        except KeyError:
            raise KeyError("App not found")

    def list_apps(self):
        return [
            {"name": name, "versions": list(versions.keys())}
            for name, versions in self.apps.items()
        ]

    def delete_app(self, name: str, version: str):
        if name not in self.apps or version not in self.apps[name]:
            raise KeyError("App not found")

        del self.apps[name][version]

        if not self.apps[name]:  # nettoyage
            del self.apps[name]

