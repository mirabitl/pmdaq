from pydantic import BaseModel
from typing import Dict, Any


class RegisterRequest(BaseModel):
    name: str


class ConfigureRequest(BaseModel):
    params: Dict[str, Any]


class AppResponse(BaseModel):
    name: str
    config: Dict[str, Any]

class CreateAppRequest(BaseModel):
    name: str
    version: str


class CommandRequest(BaseModel):
    cmd: str
    params: Dict[str, Any]


# -------- Schema --------
class SetupRequest(BaseModel):
    name: str
    version: int

class RunsRequest(BaseModel):
    experiment: str

