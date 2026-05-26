
import json

from enum import Enum
from typing import Annotated, Dict, List, Literal, Optional, Union

from pydantic import BaseModel, Field, ConfigDict


# =========================================================
# ENUMS
# =========================================================

class StorageType(str, Enum):
    DISK = "disk"
    SHM = "shm"

class RunMode(str, Enum):
    FINE = "fine"
    COARSE = "coarse"

class CalibrationMode(str, Enum):
    ALIGN = "ALIGN"
    SCURVE_1 = "SCURVE_1"
    SCURVE_A= "SCURVE_A"

# =========================================================
# COMMON OBJECTS
# =========================================================

class StorageConfig(BaseModel):
    model_config = ConfigDict(extra="forbid")

    directory: str
    type: StorageType


class DatabaseConfig(BaseModel):
    model_config = ConfigDict(extra="forbid")

    state: str
    version: int
    board: int


# =========================================================
# BASE RUN CONFIG
# =========================================================

class BaseRunConfig(BaseModel):
    model_config = ConfigDict(extra="forbid")

    comment: Optional[str] = None

    db: DatabaseConfig

    mode: RunMode

    threshold: Optional[int] = 800
    channel_list: Optional[List[int]] = None

    ctest_list: Optional[List[int]] = None
    filtering: Optional[bool] = True
    val_evt: Optional[int] = 0
    falling: Optional[int] = 0
    pol_neg: Optional[int] = 0
    nacq: Optional[int] = 40
    
# =========================================================
# NORMAL CONFIG
# =========================================================

class NormalRunConfig(BaseRunConfig):
    type:  Literal["NORMAL"] = "NORMAL"


# =========================================================
# TIMELOOP CONFIG
# =========================================================

class TimeLoopRunConfig(BaseRunConfig):
    type: Literal["TIMELOOP"] = "TIMELOOP"
    
    use_pulser: str

    rise: float = Field(gt=0)

    delay: int

    vmin: float

    vmax: float

    nstep: int = Field(gt=0)

    ctest: Optional[int] = 0

# =========================================================
# Configuration SCURVE
# =========================================================

class ScurveRunConfig(BaseRunConfig):
    type: Literal["SCURVE"] = "SCURVE"

    calibration: CalibrationMode
    
    thmin: int

    thmax: int

    thstep: int

    dc_pa: Optional[int] = 0
    
    window: Optional[int] = 5
    
    deadtime: Optional[int] = 500
    
    number_of_windows: Optional[int] = 1000
# =========================================================
# DISCRIMINATED UNION
# =========================================================

RunConfiguration = Annotated[
    Union[
        NormalRunConfig,
        TimeLoopRunConfig,
        ScurveRunConfig
    ],
    Field(discriminator="type")
]


# =========================================================
# ROOT CONFIG
# =========================================================

class PicConfig(BaseModel):
    model_config = ConfigDict(extra="forbid")

    name: str

    version: int

    location: str

    storage: StorageConfig

    configuration: str

    configuration_list: Dict[str, RunConfiguration]
    
    def to_dict(self):
        """Retourne un dictionnaire représentant l'instance."""
        return self.dict()

