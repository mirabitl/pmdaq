from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from service import DbService
from schemas import SetupRequest



# -------- Router --------
router = APIRouter(prefix="/db", tags=["db"])

service = DbService()


@router.get("/configurations", status_code=200)
def list_configurations():
    return {"configurations": service.list_configurations()}


@router.get("/parameters", status_code=200)
def list_parameters():
    return {"parameters": service.list_parameters()}


@router.post("/setups", status_code=200)
def list_setups(req: SetupRequest):
    try:
        params = {
            "name": req.name,
            "version": req.version
        }
        setups = service.list_setups(params)
        return {"setups": setups}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
