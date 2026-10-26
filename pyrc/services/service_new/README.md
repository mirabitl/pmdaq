# Refactored run-control API

This directory is an alternative to `services/`; the existing service is unchanged.
It keeps the current `/apps` and `/db` API paths and response shapes while
removing the pass-through `DbService` layer. FastAPI owns one `DbAccess` and one
in-memory `AppService` per process, and routes retrieve them from `app.state`.

Run from the `pyrc` repository root:

```sh
uvicorn services.service_new.main:app --host 0.0.0.0 --port 8000
```

The deployment must provide the same external `MongoJob` module and Python
dependencies as the existing service. Application sessions remain in memory:
use one worker unless session state is moved to a shared runtime store.