from fastapi import FastAPI
from routes import router as app_router
from db_routes import router as db_router
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(
    title="App Manager API",
    version="1.0.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # ou ton host Apache
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(app_router)
app.include_router(db_router)
