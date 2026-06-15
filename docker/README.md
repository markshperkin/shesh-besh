# docker/

Per-service Docker configuration files (configs, init scripts). The root `docker-compose.yml` references these directories for build context and volume mounts.

## Subdirectories

- `nginx/` — Nginx reverse proxy config ([README](./nginx/README.md))
- `postgres/` — Postgres initialization scripts ([README](./postgres/README.md))
