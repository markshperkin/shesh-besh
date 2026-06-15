# ADR-004: JWT RS256 for Session Auth after OAuth2

## Status
Accepted

## Context
Players authenticate via Google or Apple (OAuth2/OIDC). After the initial token exchange the server needs a session mechanism that works across REST and WebSocket without a DB lookup on every request.

## Decision
Server validates the provider's ID token (against Google/Apple JWKS), then issues its own **JWT signed with RS256** (asymmetric — private key signs, public key verifies).

JWT payload: `{ sub: user_uuid, display_name, iat, exp (24h), jti }`

## Reasons
- Stateless validation: middleware verifies signature with public key — no DB or Redis hit per request
- RS256 (asymmetric): private key stays only on server; public key can be distributed to future microservices without exposing signing capability
- 24h expiry balances UX (not re-logging in constantly) and security (limited window if token leaked)
- Standard approach for mobile + web clients (Authorization header on REST, header on WS upgrade)

## Rejected Alternatives
- **HS256 (symmetric HMAC)**: simpler but same key used to sign and verify — can't safely distribute to other services
- **Opaque session tokens in Redis**: requires a DB lookup on every request; adds Redis as a hard dependency for auth (currently optional for leaderboard)
- **Reusing provider tokens directly**: Google/Apple tokens are short-lived and not meant for repeated use; we'd hit their JWKS endpoint constantly

## Token Revocation
No revocation for MVP. A leaked token is valid until expiry (24h max). If needed in v2: maintain a `jti` blocklist in Redis with matching TTL.

## Consequences
- Server generates and stores an RS256 keypair (env var or mounted secret)
- Drogon JWT middleware reads `Authorization: Bearer <token>` header
- WS auth: token passed as `Authorization` header on the HTTP upgrade request
- Key rotation requires a brief dual-validation window (old + new public key) — deferred to v2
