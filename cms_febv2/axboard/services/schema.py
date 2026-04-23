from pydantic import BaseModel
from typing import Dict, Optional
import json

class Config(BaseModel):
    mapping: str
    buf_size: int
    resync_delay: int

class OrbitFsm(BaseModel):
    s0: int
    s1: int
    s2: int
    s3: int
    s4: int

class Trigger(BaseModel):
    external: int
    n_bc0: int

class Writer(BaseModel):
    detector_id: int
    file_directory: Optional[str]=None
    shm_directory: Optional[str]=None
    source_id: int

class FebDaqParams(BaseModel):
    config: Config
    db_state: str
    db_version: int
    feb_id: int
    location: str
    logging: str
    orbit_fsm: OrbitFsm
    disable_force_s2: int
    trigger: Trigger
    vth_shift: int
    writer: Writer
    pa_ccomp: Optional[int]=None
    delay_reset_trigger: Optional[int]=None

class FebAcquisition(BaseModel):
    daq: FebDaqParams
    name: str
    version: Optional[int]=1
    def to_dict(self):
        """Retourne un dictionnaire représentant l'instance."""
        return self.dict()
