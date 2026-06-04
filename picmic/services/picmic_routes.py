from fastapi import APIRouter, HTTPException, status, UploadFile, File
from picmic_models import picmic_physic 
from pydantic import BaseModel
from typing import Dict, Any, Optional, List
import inspect
from typing import get_origin, get_args
import json
import os
from pathlib import Path
import time
from collections import deque

class CreateAcqRequest(BaseModel):
    name: str
    version: int

class TransitionAcqRequest(BaseModel):
    name: str
    params: Optional[Dict[str,Any]]={}

class CalibrationRequest(BaseModel):
    thmin: int
    thmax: int
    thstep: int
    dc_pa: int = 0
    mode: Optional[str] = None

class AdvancedParamsRequest(BaseModel):
    threshold: int
    filtering: bool
    falling: bool
    val_evt: bool
    pol_neg: bool
    dc_pa: int

router = APIRouter(prefix="/picmic", tags=["picmic"])

acq = picmic_physic()

# Data history for monitoring
status_history = deque(maxlen=100)
config_storage_dir = Path("/tmp/picmic_configs")


@router.get("/", status_code=200)
def get_status():
    return {"status": acq.get_status()}

@router.post("/create", status_code=201)
def create_app(req: CreateAcqRequest):
    try:
        acq.set_db_configuration(req.name, req.version)
        return {"message": "App created"}
    except ValueError as e:
        raise HTTPException(status_code=409, detail=str(e))



def format_type(annotation):
    if annotation == inspect.Parameter.empty:
        return None

    origin = get_origin(annotation)
    args = get_args(annotation)

    if origin:
        return f"{origin.__name__}[{', '.join(format_type(a) or 'Any' for a in args)}]"

    if hasattr(annotation, "__name__"):
        return annotation.__name__

    return str(annotation).replace("typing.", "")


@router.get("/methods", status_code=200)
def list_methods():
    try:
        methods = []
        cls = acq.__class__

        for name, member in inspect.getmembers(acq, predicate=callable):
            # ✅ uniquement public
            if name.startswith("_"):
                continue

            # optionnel : uniquement définies dans la classe
            if not hasattr(cls, name):
                continue

            sig = inspect.signature(member)

            methods.append({
                "name": name,
                "parameters": [
                    {
                        "name": p_name,
                        "type": format_type(p.annotation),
                        "default": None if p.default == inspect.Parameter.empty else p.default
                    }
                    for p_name, p in sig.parameters.items()
                ]
                ,
                "return": format_type(sig.return_annotation)
            })

        return {"methods": methods}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/commands")
def execute_command(req: TransitionAcqRequest):
    try:
        result = acq.execute(req.name, req.params)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except TypeError as e:
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/transitions")
def execute_transition(req: TransitionAcqRequest):
    try:
        result = acq.transition(req.name)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.delete("/", status_code=204)
def delete_app(name: str, version: str):
    try:
        del acq
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")


# ============================================================================
# NEW ENDPOINTS FOR GRAPHICS & ADVANCED FEATURES
# ============================================================================

@router.get("/status-history", status_code=200)
def get_status_history():
    """Get historical status data for monitoring charts"""
    try:
        history = list(status_history)
        return {
            "history": history,
            "timestamp": time.time(),
            "count": len(history)
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/update-status", status_code=200)
def update_status():
    """Update status with timestamp for monitoring"""
    try:
        status = acq.get_status()
        status["timestamp"] = time.time()
        status_history.append(status)
        return {"status": status}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/calibration-data", status_code=200)
def get_calibration_data():
    """Get S-curve calibration data for visualization"""
    try:
        if hasattr(acq, 'scurve_data') and acq.scurve_data:
            return {
                "data": acq.scurve_data,
                "timestamp": time.time()
            }
        return {
            "data": None,
            "message": "No calibration data available"
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/calibration", status_code=200)
def start_calibration(req: CalibrationRequest):
    """Start S-curve calibration with parameters"""
    try:
        params = {
            "db": {
                "board": getattr(req, 'board', 0),
                "state": getattr(req, 'state', 'default'),
                "version": getattr(req, 'version', 0)
            },
            "thmin": req.thmin,
            "thmax": req.thmax,
            "thstep": req.thstep,
            "dc_pa": req.dc_pa
        }
        if req.mode:
            params["mode"] = req.mode
        
        result = acq.execute("run_calibration", params)
        return {"result": result, "message": "Calibration started"}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.get("/config/list", status_code=200)
def list_configurations():
    """List all saved configurations"""
    try:
        config_storage_dir.mkdir(parents=True, exist_ok=True)
        configs = []
        for f in config_storage_dir.glob("*.json"):
            configs.append({
                "name": f.stem,
                "size": f.stat().st_size,
                "modified": f.stat().st_mtime
            })
        return {"configurations": sorted(configs, key=lambda x: x['modified'], reverse=True)}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/config/load/{config_name}", status_code=200)
def load_configuration(config_name: str):
    """Load a specific configuration"""
    try:
        config_file = config_storage_dir / f"{config_name}.json"
        if not config_file.exists():
            raise HTTPException(status_code=404, detail="Configuration not found")
        
        with open(config_file, 'r') as f:
            config_data = json.load(f)
        
        acq.set_configuration(config_data)
        return {"message": "Configuration loaded", "config": config_data}
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="Configuration file not found")
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.post("/config/save", status_code=201)
def save_configuration(name: str, description: str = ""):
    """Save current configuration"""
    try:
        config_storage_dir.mkdir(parents=True, exist_ok=True)
        config_file = config_storage_dir / f"{name}.json"
        
        config_data = {
            "name": name,
            "description": description,
            "timestamp": time.time(),
            "configuration": acq.config_dict if hasattr(acq, 'config_dict') else {}
        }
        
        with open(config_file, 'w') as f:
            json.dump(config_data, f, indent=2)
        
        return {"message": "Configuration saved", "filename": str(config_file)}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.delete("/config/{config_name}", status_code=204)
def delete_configuration(config_name: str):
    """Delete a configuration file"""
    try:
        config_file = config_storage_dir / f"{config_name}.json"
        if not config_file.exists():
            raise HTTPException(status_code=404, detail="Configuration not found")
        
        config_file.unlink()
        return {"message": "Configuration deleted"}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.post("/advanced-params", status_code=200)
def set_advanced_parameters(req: AdvancedParamsRequest):
    """Set advanced DAQ parameters"""
    try:
        params = {
            "threshold": req.threshold,
            "filtering": req.filtering,
            "falling": req.falling,
            "val_evt": req.val_evt,
            "pol_neg": req.pol_neg,
            "dc_pa": req.dc_pa
        }
        
        # Store in config
        if hasattr(acq, 'daq_conf'):
            for key, value in params.items():
                if hasattr(acq.daq_conf, key):
                    setattr(acq.daq_conf, key, value)
        
        return {
            "message": "Advanced parameters updated",
            "parameters": params
        }
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.get("/advanced-params", status_code=200)
def get_advanced_parameters():
    """Get current advanced parameters"""
    try:
        if hasattr(acq, 'daq_conf'):
            params = {
                "threshold": getattr(acq.daq_conf, 'threshold', 800),
                "filtering": getattr(acq.daq_conf, 'filtering', False),
                "falling": getattr(acq.daq_conf, 'falling', False),
                "val_evt": getattr(acq.daq_conf, 'val_evt', False),
                "pol_neg": getattr(acq.daq_conf, 'pol_neg', False),
                "dc_pa": getattr(acq.daq_conf, 'dc_pa', 0)
            }
            return params
        return {}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# MONGODB CONFIGURATION MANAGEMENT
# ============================================================================

@router.get("/mongo/configs", status_code=200)
def list_mongo_configurations():
    """List all configurations from MongoDB"""
    try:
        import picmic_register_access as cra
        sdb = cra.instance()
        config_list = sdb.configurations()
        
        if not config_list:
            return {"configurations": []}
        
        configs = [
            {
                "name": config[0],
                "version": config[1],
                "label": f"{config[0]}:{config[1]}"
            }
            for config in config_list
        ]
        
        return {"configurations": configs}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"MongoDB error: {str(e)}")


@router.post("/mongo/config/download", status_code=200)
def download_mongo_configuration(name: str, version: int):
    """Download a configuration from MongoDB and set it"""
    try:
        import picmic_register_access as cra
        sdb = cra.instance()
        
        # Download from MongoDB
        sdb.download_configuration(name, version)
        
        # Load the JSON file
        config_file = f"/dev/shm/config/{name}_{version}.json"
        with open(config_file, 'r') as f:
            config_data = json.load(f)
        
        # Set the configuration in the acquisition object
        acq.set_configuration(config_data)
        
        return {
            "message": f"Configuration {name}:{version} downloaded and loaded",
            "config": config_data
        }
    except FileNotFoundError as e:
        raise HTTPException(status_code=404, detail=f"Configuration file not found: {str(e)}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error: {str(e)}")


@router.get("/config/current", status_code=200)
def get_current_config():
    """Get the current loaded configuration as JSON"""
    try:
        if hasattr(acq, 'config_dict') and acq.config_dict:
            return {"config": acq.config_dict}
        return {"config": None, "message": "No configuration loaded"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/config/update", status_code=200)
def update_current_config(config_data: Dict[str, Any]):
    """Update the current configuration with new data"""
    try:
        acq.set_configuration(config_data)
        return {
            "message": "Configuration updated",
            "config": acq.config_dict if hasattr(acq, 'config_dict') else {}
        }
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.post("/mongo/config/save", status_code=201)
def save_mongo_configuration(name: str, comment: str = ""):
    """Save current configuration to MongoDB"""
    try:
        import picmic_register_access as cra
        
        if not hasattr(acq, 'config_dict') or not acq.config_dict:
            raise HTTPException(status_code=400, detail="No configuration loaded to save")

        config_to_save = dict(acq.config_dict)
        config_to_save.pop("comment", None)
        config_to_save.pop("description", None)
        config_to_save.pop("timestamp", None)
        
        # Save to temporary file
        temp_file = f"/tmp/{name}_save.json"
        with open(temp_file, 'w') as f:
            json.dump(config_to_save, f, indent=2)
        
        # Upload to MongoDB
        sdb = cra.instance()
        sdb.upload_configuration(temp_file, comment=comment)
        
        return {"message": f"Configuration {name} saved to MongoDB"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error saving to MongoDB: {str(e)}")
