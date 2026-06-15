# Auth — OAuth2 + JWT

## Description

Players authenticate via Google or Apple OAuth2. The server validates the provider ID token, creates an account on first login, and issues its own RS256 JWT used for all subsequent requests and WebSocket connections.

## Acceptance Criteria

1. **Given** a client sends a valid Google ID token to `POST /auth/google`, **When** the server validates it against Google's JWKS endpoint, **Then** the server returns a server-issued JWT with 24h expiry and HTTP 200.

2. **Given** a client sends a valid Apple ID token to `POST /auth/apple`, **When** the server validates it against Apple's JWKS endpoint, **Then** the server returns a server-issued JWT with 24h expiry and HTTP 200.

3. **Given** a valid ID token for a user not yet in the database, **When** the server processes `/auth/google` or `/auth/apple`, **Then** a new user record is created with `display_name` from the provider profile and `points_balance` of 1000.

4. **Given** a valid ID token for an existing user, **When** the server processes the auth request, **Then** no new user record is created and the existing account is returned.

5. **Given** a client sends an expired or tampered ID token, **When** the server attempts JWKS validation, **Then** the server returns HTTP 401 and no JWT is issued.

6. **Given** a client sends a missing or malformed request body, **When** the server receives the auth request, **Then** the server returns HTTP 400.

7. **Given** a client includes a valid JWT in `Authorization: Bearer` header, **When** any protected REST route is called, **Then** the request proceeds to the handler.

8. **Given** a client includes an expired JWT, **When** any protected REST route is called, **Then** the server returns HTTP 401.

9. **Given** a client includes a tampered JWT (signature mismatch), **When** any protected REST route is called, **Then** the server returns HTTP 401.

10. **Given** a client includes a valid JWT as the `Authorization` header on the WebSocket upgrade request, **When** the connection is established, **Then** the WebSocket connection is accepted and associated with the authenticated player.

11. **Given** a client sends a missing or invalid JWT on the WebSocket upgrade, **When** the server processes the upgrade, **Then** the connection is rejected with HTTP 401.

12. **Given** a JWT is issued, **When** inspected, **Then** it contains `sub` (user UUID), `display_name`, `iat`, `exp`, and `jti` claims.

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| Google JWKS endpoint is unreachable | Return HTTP 503; do not issue JWT |
| Apple JWKS endpoint is unreachable | Return HTTP 503; do not issue JWT |
| Provider returns same `sub` for two different providers | Treated as different accounts (keyed by `(provider, sub)`) |
| First-login race condition (two simultaneous requests for same new user) | Only one user record created; second request returns same account |
| JWT with valid signature but `exp` in the past | Rejected with 401 |
| WebSocket upgrade with no Authorization header | Rejected with 401 |

## Test Requirements

- AC1–2: Integration test — mock Google/Apple JWKS, send valid token, assert JWT returned with correct shape
- AC3: Integration test — assert new user row created with balance=1000 on first login
- AC4: Integration test — assert no duplicate user row on repeat login
- AC5: Unit test — invalid/expired provider token returns 401
- AC6: Unit test — malformed request body returns 400
- AC7–9: Middleware unit test — valid/expired/tampered JWT against each protected route
- AC10–11: WebSocket integration test — upgrade accepted/rejected based on JWT validity
- AC12: Unit test — JWT claims structure matches spec

## Dependencies

None — this is the foundation all other specs depend on.

## Out of Scope

- Token refresh / silent re-auth
- JWT revocation / blocklist
- Admin authentication
- Facebook or other OAuth providers (v2)

## Open Questions

- None.
