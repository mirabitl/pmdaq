from fastapi import APIRouter, HTTPException, status
from models import febv2_physic 
from pydantic import BaseModel
from typing import Dict, Any,Optional
import inspect
from typing import get_origin, get_args

class CreateAcqRequest(BaseModel):
    name: str
    version: int

class TransitionAcqRequest(BaseModel):
    name: str
    params: Optional[Dict[str,Any]]={}


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
