from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from service import DbService
from schemas import SetupRequest,RunsRequest



# -------- Router --------
router = APIRouter(prefix="/db", tags=["db"])

service = DbService()


@router.get("/configurations", status_code=200)
def list_configurations():
    return  service.list_configurations() #{"configurations": service.list_configurations()}


@router.get("/parameters", status_code=200)
def list_parameters():
    return service.list_parameters() #{"parameters": service.list_parameters()}


@router.post("/setups", status_code=200)
def list_setups(req: SetupRequest):
    try:
        params = {
            "name": req.name,
            "version": req.version
        }
        setups = service.list_setups(params)
        return {"setup_list": setups}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/runs", status_code=200)
def list_runs(req: RunsRequest):
    try:
        params = {
            "experiment": req.experiment
        }
        runs = service.list_runs(params)
        return {"run_list": runs}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
