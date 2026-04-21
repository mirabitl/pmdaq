from fastapi import APIRouter, HTTPException, status
from models import febv2_physic 
from pydantic import BaseModel
from typing import Dict, Any,Optional
import inspect

class CreateAcqRequest(BaseModel):
    name: str
    version: int

class TransitionAcqRequest(BaseModel):
    name: str
    params: Optional[Dict[Any]]={}


router = APIRouter(prefix="/febv2", tags=["febv2"])

acq = febv2_physic()


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

@router.get("/methods")
def list_methods(self):
    # Lister les méthodes appelables de self
    methodes = [attr for attr in dir(acq) if callable(getattr(acq, attr))]
    #print("Méthodes appelables de self :")
    vm=[]
    for methode in methodes:
        signature=inspect.signature(acq.methode)
        d={}
        d["method"]=methode
        d["parameters"]=[]
    # Afficher les paramètres d'une méthode spécifique
        for nom, param in signature.parameters.items():
            annotation = param.annotation if param.annotation != inspect.Parameter.empty else "Non spécifié"
            d["parameters"].append({"nom":nom,"type":annotation})
        vm.append(d)
    return {"methods":vm}

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
