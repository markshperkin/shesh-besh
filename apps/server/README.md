# apps/server/

Drogon C++ application entry point. Bootstraps Postgres and Redis connections from environment variables, registers the `/health` endpoint, and starts the HTTP server on port 3000.

## Files

- `main.cpp` — application entry point: wires up DB/Redis clients, registers `/health`, runs the Drogon event loop
- `CMakeLists.txt` — builds the `server` executable against Drogon, requires C++20
