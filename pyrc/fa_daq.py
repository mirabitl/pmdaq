from fastapi import FastAPI, HTTPException, status
from pydantic import BaseModel
from typing import Dict, Any
import socket
import json
from copy import deepcopy
import base64
import time
import MongoAsic as mga
import MongoJob as mgj
import rc_fast as daq

app = FastAPI()

# Dictionnaire pour stocker les daq_sessioncations
mes_daq_sessions: Dict[str, Any] = {}

class Daq_Session:
    def __init__(self,session):
        vn=session.split(":")
        self.session=session
        self.session_name=vn[0]
        self.session_version=int(vn[1])
        self.daq=None
    """Classe de base pour une daq_session."""
    def configure(self, params: Dict) -> Dict:
        """Méthode de configuration de l'daq_session."""
        self._wdj=mgj.instance()
        try:
            self._wdj.downloadConfig(self.session_name,self.session_version,True)
        except:
            return {"status":"failed"}
        self.conf_file=f"/dev/shm/mgjob/{self.session_name}_{self.session_version}.json"
        self.daq=daq.rc_fast(self.conf_file)
        return {"status": "configured", "file":self.conf_file}

    def restart(self):
        if self.daq:
            self.daq.restart()
            return {"status": "restarted", "status":"remove and reconfigure needed"}
        else:
            return {"status": "missing", "status":"daq {self.session} is not configured"}
        
    def execute_command(self, cmd_name: str, params: Dict) -> Dict:
        """Méthode pour exécuter une commande sur l'daq_session."""
        newres={"aa":100,"bb":200}
        return {"status": "done", "command_name": cmd_name, "params": params, "result":newres}
    def execute_transition(self, trs_name: str, params: Dict) -> Dict:
        """Méthode pour exécuter une commande sur l'daq_session."""
        valid=[]
        return {"status": "Transition executed", "transition_name": trs_name, "params": params}

class RegisterRequest(BaseModel):
    session: str

class RestartRequest(BaseModel):
    session: str

class ConfigureRequest(BaseModel):
    session: str
    params: Dict

class RemoveRequest(BaseModel):
    session: str

class CommandRequest(BaseModel):
    cmd_name: str
    session: str
    params: Dict

class TranitionRequest(BaseModel):
    trs_name: str
    session: str
    params: Dict

@app.get("/list")
async def list_daq_sessions():
    """Retourne la liste des clés des daq_sessions enregistrées."""
    return {"status_code": status.HTTP_200_OK, "daq_sessions": list(mes_daq_sessions.keys())}

@app.post("/register")
async def register_daq_session(request: RegisterRequest):
    """Enregistre une nouvelle daq_session."""
    if request.session in mes_daq_sessions:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Daq_Session already exists")
    # Access 
    mes_daq_sessions[request.session] = Daq_Session(request.session)
    return {"status_code": status.HTTP_201_CREATED, "message": f"Daq_Session {request.session} registered {mes_daq_sessions[request.session].session_name}  {mes_daq_sessions[request.session].session_version}"}

@app.post("/restart")
async def restart_daq_session(request: RestartRequest):
    """Enregistre une nouvelle daq_session."""
    if not request.session in mes_daq_sessions:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Daq_Session does not exist")
    # Access 
    msg=mes_daq_sessions[request.session].restart()
    return {"status_code": status.HTTP_201_CREATED, "message": msg}

@app.post("/configure")
async def configure_daq_session(request: ConfigureRequest):
    """Configure une daq_session existante."""
    if request.session not in mes_daq_sessions:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Daq_Session not found")
    result = mes_daq_sessions[request.session].configure(request.params)
    return {"status_code": status.HTTP_200_OK, "result": result}

@app.post("/remove")
async def remove_daq_session(request: RemoveRequest):
    """Supprime une daq_session existante."""
    if request.session not in mes_daq_sessions:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Daq_Session not found")
    del mes_daq_sessions[request.session]
    return {"status_code": status.HTTP_200_OK, "message": f"Daq_Session {request.session} removed"}

@app.post("/command")
async def execute_command(request: CommandRequest):
    """Exécute une commande sur une daq_session existante."""
    if request.session not in mes_daq_sessions:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Daq_Session not found")
    result = mes_daq_sessions[request.session].execute_command(request.cmd_name, request.params)
    return {"status_code": status.HTTP_200_OK, "infos": result}

@app.post("/transition")
async def execute_transition(request: CommandRequest):
    """Exécute une commande sur une daq_session existante."""
    if request.session not in mes_daq_sessions:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Daq_Session not found")
    result = mes_daq_sessions[request.session].execute_transition(request.trs_name, request.params)
    return {"status_code": status.HTTP_200_OK, "infos": result}

# Pour tester, exécutez avec: uvicorn main:app --reload
