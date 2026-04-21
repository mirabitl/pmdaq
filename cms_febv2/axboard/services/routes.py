from fastapi import APIRouter, HTTPException, status
from schema import CreateAcqRequest
from models import febv2_physic 

router = APIRouter(prefix="/febv2", tags=["febv2"])

acq = febv2_physic()


@router.get("/", status_code=200)
def get_status():
    return {"status": acq.acq_status()}

@router.post("/create", status_code=201)
def create_app(req: CreateAcqRequest):
    try:
        acq.set_db_configuration(req.name, req.version)
        return {"message": "App created"}
    except ValueError as e:
        raise HTTPException(status_code=409, detail=str(e))

@router.post("/commands")
def execute_command(req: CommandRequest):
    try:
        result = acq.execute(req.cmd, req.params)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/transitions")
def execute_transition(req: RegisterRequest):
    try:
        result = acq.transition(req.name)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.delete("/{name}/versions/{version}", status_code=204)
def delete_app(name: str, version: str):
    try:
        del acq
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
