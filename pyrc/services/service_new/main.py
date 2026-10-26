from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .db_routes import router as db_router
from .models import DbAccess
from .routes import router as app_router
from .service import AppService


def create_app() -> FastAPI:
    app = FastAPI(title="App Manager API", version="1.0.0")
    app.add_middleware(
        CORSMiddleware,
        allow_origins=["*"],
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

    db_access = DbAccess()
    app.state.db_access = db_access
    app.state.app_service = AppService(db_access)
    app.include_router(app_router)
    app.include_router(db_router)
    return app


app = create_app()