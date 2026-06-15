#!/usr/bin/env bash
# RED: This script will fail until the server is running via docker-compose.
# Confirms RED state before GREEN implementation.
set -euo pipefail

HOST="${SERVER_HOST:-localhost}"
PORT="${SERVER_PORT:-3000}"
URL="http://${HOST}:${PORT}/health"

echo "Testing GET ${URL} ..."
RESPONSE=$(curl -sf --max-time 5 "${URL}") || {
    echo "FAIL: Could not reach ${URL} (server not running or not healthy)"
    exit 1
}

STATUS=$(echo "${RESPONSE}" | grep -o '"status":"ok"' || true)
if [ -z "${STATUS}" ]; then
    echo "FAIL: Response did not contain {\"status\":\"ok\"}: ${RESPONSE}"
    exit 1
fi

echo "PASS: GET /health returned 200 with {\"status\":\"ok\"}"
