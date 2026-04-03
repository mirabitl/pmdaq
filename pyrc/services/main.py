from fastapi import FastAPI
from routes import router

app = FastAPI(
    title="App Manager API",
    version="1.0.0"
)

app.include_router(router)
