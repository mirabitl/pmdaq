from fastapi import APIRouter, HTTPException, status
from schemas import CreateAppRequest, CommandRequest, ConfigureRequest, RegisterRequest
from models import Session  
from service import AppService

router = APIRouter(prefix="/apps", tags=["apps"])

service = AppService()


@router.get("/", status_code=200)
def list_apps():
    return {"apps": service.list_apps()}

@router.post("/", status_code=201)
def create_app(req: CreateAppRequest):
    try:
        service.create_app(req.name, req.version)
        return {"message": "App created"}
    except ValueError as e:
        raise HTTPException(status_code=409, detail=str(e))

@router.post("/{name}/versions/{version}/configure")
def configure_app(name: str, version: str, req: ConfigureRequest):
    try:
        app = service.get_app(name, version)
        result = app.configure(req.params)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/{name}/versions/{version}/restart")
def restart_app(name: str, version: str):
    try:
        app = service.get_app(name, version)
        result = app.restart()
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/{name}/versions/{version}/commands")
def execute_command(name: str, version: str, req: CommandRequest):
    try:
        app = service.get_app(name, version)
        result = app.execute(req.cmd, req.params)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/{name}/versions/{version}/transitions")
def execute_command(name: str, version: str, req: RegisterRequest):
    try:
        app = service.get_app(name, version)
        result = app.transition(req.cmd)
        return {"result": result}
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.delete("/{name}/versions/{version}", status_code=204)
def delete_app(name: str, version: str):
    try:
        service.delete_app(name, version)
    except KeyError:
        raise HTTPException(status_code=404, detail="App not found")