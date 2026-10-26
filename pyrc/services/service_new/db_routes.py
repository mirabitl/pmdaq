from fastapi import APIRouter, Depends, HTTPException, Request

from .models import DbAccess
from .schemas import RunsRequest, SetupRequest

router = APIRouter(prefix="/db", tags=["db"])


def get_db_access(request: Request) -> DbAccess:
    return request.app.state.db_access


@router.get("/configurations", status_code=200)
def list_configurations(db: DbAccess = Depends(get_db_access)):
    return db.configurations()


@router.get("/parameters", status_code=200)
def list_parameters(db: DbAccess = Depends(get_db_access)):
    return db.parameters()


@router.post("/setups", status_code=200)
def list_setups(req: SetupRequest, db: DbAccess = Depends(get_db_access)):
    try:
        return {"setup_list": db.parameter_info(req.name, req.version)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@router.post("/runs", status_code=200)
def list_runs(req: RunsRequest, db: DbAccess = Depends(get_db_access)):
    try:
        return {"run_list": db.runs(req.experiment)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc