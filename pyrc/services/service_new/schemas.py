from typing import Any

from pydantic import BaseModel, Field


class CreateAppRequest(BaseModel):
    name: str
    version: str


class ConfigureRequest(BaseModel):
    params: dict[str, Any] = Field(default_factory=dict)


class CommandRequest(BaseModel):
    cmd: str
    params: dict[str, Any] = Field(default_factory=dict)


class TransitionRequest(BaseModel):
    name: str


class SetupRequest(BaseModel):
    name: str
    version: int


class RunsRequest(BaseModel):
    experiment: str