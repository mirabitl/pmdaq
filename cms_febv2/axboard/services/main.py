from fastapi import FastAPI
from routes import router as app_router
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(
    title="FEBV2 AX7325 Manager",
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
