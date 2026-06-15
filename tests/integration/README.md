# tests/integration/

Shell-based integration tests that validate the running stack. Currently in RED state — tests are written before the compose stack exists (TASK-001 TDD).

## Files

- `test_health.sh` — curls `GET /health` and asserts `{"status":"ok"}` is returned; uses `SERVER_HOST`/`SERVER_PORT` env vars (defaults: localhost:3000)
- `test_compose_smoke.sh` — validates `docker-compose.yml` syntax via `docker compose config` and checks that all four required services (`postgres`, `redis`, `nginx`, `server`) are declared
