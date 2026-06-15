# docker/nginx/

Nginx reverse proxy config: redirects HTTP to HTTPS and forwards traffic to the Drogon server with WebSocket support.

## Files

- `nginx.conf` — listens on 80 (redirect) and 443 (TLS termination); proxies to `server:3000` with upgrade headers for WebSocket and 1-hour read/send timeouts
