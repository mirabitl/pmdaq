from typing import Dict

from .models import DbAccess, Session


class AppService:
    def __init__(self, db: DbAccess):
        self._db = db
        self.apps: Dict[str, Dict[str, Session]] = {}

    def create_app(self, name: str, version: str):
        versions = self.apps.setdefault(name, {})
        if version in versions:
            raise ValueError("App version already exists")
        versions[version] = Session(name, version, self._db)

    def get_app(self, name: str, version: str) -> Session:
        try:
            return self.apps[name][version]
        except KeyError as exc:
            raise KeyError("App not found") from exc

    def list_apps(self):
        return [
            {"name": name, "versions": list(versions)}
            for name, versions in self.apps.items()
        ]

    def delete_app(self, name: str, version: str):
        versions = self.apps.get(name)
        if versions is None or version not in versions:
            raise KeyError("App not found")

        del versions[version]
        if not versions:
            del self.apps[name]