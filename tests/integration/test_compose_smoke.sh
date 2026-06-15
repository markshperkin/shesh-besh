#!/usr/bin/env bash
# Smoke test: verify docker-compose.yml is valid YAML and has required services.
# RED: Fails until docker-compose.yml exists with required services.
set -euo pipefail

COMPOSE_FILE="${1:-docker-compose.yml}"

if [ ! -f "${COMPOSE_FILE}" ]; then
    echo "FAIL: ${COMPOSE_FILE} not found"
    exit 1
fi

# Validate YAML syntax via docker compose config
docker compose -f "${COMPOSE_FILE}" config --quiet 2>/dev/null || {
    echo "FAIL: docker-compose.yml is invalid YAML or failed compose config"
    exit 1
}

# Check required services are present
for SERVICE in postgres redis nginx server; do
    if ! grep -q "^  ${SERVICE}:" "${COMPOSE_FILE}"; then
        echo "FAIL: service '${SERVICE}' not found in ${COMPOSE_FILE}"
        exit 1
    fi
done

echo "PASS: docker-compose.yml is valid and contains all required services"
