# ADR-001: Drogon as WebSocket + HTTP Server Framework

## Status
Accepted

## Context
The server needs to handle both REST (auth, lobby, social) and persistent WebSocket connections (real-time gameplay). The game engine is C++20. Options evaluated: Crow, Boost.Beast, Drogon.

## Decision
Use **Drogon** as the server framework.

## Reasons
- First-class WebSocket support with per-connection handlers
- Async, coroutine-based I/O via `trantor` event loops — enables strand-per-match model (ADR-005)
- Built-in HTTP routing, middleware, and JWT filter hooks
- Production-grade, actively maintained, used in real C++ backends
- Docker-friendly, no system dependencies beyond libpq and hiredis

## Rejected Alternatives
- **Crow**: thin WebSocket support, not suitable for production-grade WS server
- **Boost.Beast**: correct choice for maximum control, but requires building all abstractions (routing, middleware, connection management) from scratch — unnecessary overhead for this project

## Consequences
- Drogon added as a CMake dependency (via FetchContent or system package)
- Game logic (engine library) remains independent of Drogon — only the server layer depends on it
- `trantor::EventLoop` strands available for match isolation (see ADR-005)
