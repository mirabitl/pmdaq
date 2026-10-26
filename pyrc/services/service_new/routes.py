from fastapi import APIRouter, Depends, HTTPException, Request

from .schemas import CommandRequest, ConfigureRequest, CreateAppRequest, TransitionRequest
from .service import AppService

router = APIRouter(prefix="/apps", tags=["apps"])


def get_app_service(request: Request) -> AppService:
    return request.app.state.app_service


def get_session(
    name: str,
    version: str,
    service: AppService = Depends(get_app_service),
):
    try:
        return service.get_app(name, version)
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")


@router.get("/", status_code=200)
def list_apps(service: AppService = Depends(get_app_service)):
    return {"apps": service.list_apps()}


@router.post("/", status_code=201)
def create_app(req: CreateAppRequest, service: AppService = Depends(get_app_service)):
    try:
        service.create_app(req.name, req.version)
    except ValueError as exc:
        raise HTTPException(status_code=409, detail=str(exc)) from exc
    return {"message": "App created"}


@router.post("/{name}/versions/{version}/configure")
def configure_app(name: str, version: str, req: ConfigureRequest, app=Depends(get_session)):
    try:
        return {"result": app.configure(req.params)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@router.post("/{name}/versions/{version}/restart")
def restart_app(name: str, version: str, app=Depends(get_session)):
    try:
        return {"result": app.restart()}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@router.post("/{name}/versions/{version}/commands")
def execute_command(name: str, version: str, req: CommandRequest, app=Depends(get_session)):
    try:
        return {"result": app.execute(req.cmd, req.params)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@router.post("/{name}/versions/{version}/transitions")
def execute_transition(name: str, version: str, req: TransitionRequest, app=Depends(get_session)):
    try:
        return {"result": app.transition(req.name)}
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@router.delete("/{name}/versions/{version}", status_code=204)
def delete_app(name: str, version: str, service: AppService = Depends(get_app_service)):
    try:
        service.delete_app(name, version)
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")