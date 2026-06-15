# Implementation Plan

## Overview

- Project: Shesh-Besh — Real-Time Multiplayer Backgammon Server
- Generated: 2026-06-15
- Total tasks: 20
- Estimated effort: 8–14 weeks (solo developer)
- Waves: 10

---

## Gap Analysis

- **Engine prerequisites**: Tasks T016–T018 (gameplay) depend on GitHub issues #1–#4 (immutable GameState, event log, move history, max dice enforcement) being resolved in the engine library first. These are tracked separately and must be completed before Wave 8.
- **PlayerRegistry + strand**: GitHub issues #5 and #6 are prerequisites for T011 and T012 respectively. Noted in task dependencies.
- **Apple Sign-In quirk**: Apple's ID token requires server-side JWT validation (not just JWKS public key lookup) — the `kid` lookup and nonce handling add complexity. Flagged as HITL.
- **No gaps in specs**: All ACs are testable, scope is bounded, dependencies are named. No conflicting requirements found across specs.

---

## Tasks by Wave

---

### Wave 1 — Foundation

---

### TASK-001: Drogon server scaffold + Docker Compose

- Spec: docs/architecture.md
- Slice: A `GET /health` endpoint returns 200; server boots in Docker with Postgres and Redis connected and reachable
- Mode: HITL
- Files: create `apps/server/main.cpp`, create `apps/server/CMakeLists.txt`, modify `CMakeLists.txt`, create `docker-compose.yml`, create `docker/nginx/nginx.conf`, create `docker/postgres/init.sql` (empty placeholder), create `.env.example`
- Depends on: none
- Complexity: M
- AC:
  - `GET /health` returns HTTP 200 with `{ "status": "ok" }`
  - Server boots via `docker-compose up` with Postgres and Redis services reachable
  - Drogon connects to Postgres connection pool on startup
  - Drogon connects to Redis on startup
  - Nginx proxies HTTPS (443) to Drogon (3000) internally
  - GitHub Actions CI builds the Docker image successfully

---

### TASK-002: Database schema migrations

- Spec: docs/architecture.md
- Slice: All tables exist in Postgres with correct columns, constraints, and indexes after running migrations
- Mode: AFK
- Files: create `db/migrations/001_initial_schema.sql`
- Depends on: TASK-001
- Complexity: S
- AC:
  - Tables created: `users`, `game_modes`, `matches`, `match_events`, `point_transactions`, `friendships`
  - `users.points_balance` has `CHECK (points_balance >= 0)` constraint
  - `friendships` has `CHECK (requester_id <> addressee_id)` constraint
  - `point_transactions` has no UPDATE or DELETE permitted (append-only enforced by policy or convention)
  - Indexes created: `(users.oauth_provider, users.oauth_subject)`, `(users.points_balance DESC)`, `(match_events.match_id, match_events.seq)`, `(point_transactions.user_id, point_transactions.created_at DESC)`
  - Migration runs idempotently (re-running does not error)
  - Migration applied automatically on server start via startup hook

---

### Wave 2 — Auth Providers

---

### TASK-003: Google OAuth2 token exchange + JWT issuance + first-login account creation

- Spec: specs/auth.md
- Slice: A client can POST a Google ID token and receive a server JWT; a new account is created with 1000 points on first login
- Mode: HITL
- Files: create `apps/server/services/auth_service.hpp`, create `apps/server/services/auth_service.cpp`, create `apps/server/controllers/auth_controller.hpp`, create `apps/server/controllers/auth_controller.cpp`, create `apps/server/util/jwt.hpp`, create `apps/server/util/jwt.cpp`
- Depends on: TASK-002
- Complexity: M
- AC:
  - `POST /auth/google` with valid Google ID token returns HTTP 200 with server JWT
  - JWT contains `sub` (user UUID), `display_name`, `iat`, `exp` (24h), `jti`
  - JWT signed with RS256 using private key from env var
  - New user row created on first login with `points_balance=1000` and `display_name` from Google profile
  - Existing user returned on repeat login — no duplicate row created
  - `point_transactions` row inserted with `reason=signup_bonus`, `delta=1000` on first login only
  - Concurrent first-login requests for same Google sub produce exactly one user row
  - `POST /auth/google` with invalid/expired Google ID token returns HTTP 401
  - `POST /auth/google` with malformed body returns HTTP 400
  - Google JWKS endpoint unreachable returns HTTP 503

---

### TASK-004: Apple OAuth2 token exchange

- Spec: specs/auth.md
- Slice: A client can POST an Apple ID token and receive a server JWT with the same behavior as Google login
- Mode: HITL
- Files: modify `apps/server/services/auth_service.hpp`, modify `apps/server/services/auth_service.cpp`, modify `apps/server/controllers/auth_controller.cpp`
- Depends on: TASK-002
- Complexity: M
- AC:
  - `POST /auth/apple` with valid Apple ID token returns HTTP 200 with server JWT
  - Apple ID token validated against Apple's JWKS endpoint (handles Apple-specific `kid` lookup)
  - New user row created on first Apple login with `display_name` from Apple profile
  - Same user is returned on repeat login — no duplicate row
  - Google and Apple accounts with the same email are treated as separate accounts (keyed by `(provider, sub)`)
  - Apple JWKS endpoint unreachable returns HTTP 503
  - Invalid Apple token returns HTTP 401

---

### Wave 3 — Auth Middleware + Game Mode Seeding

---

### TASK-005: JWT validation middleware for REST + WebSocket upgrade

- Spec: specs/auth.md
- Slice: All protected REST routes return 401 without a valid JWT; WebSocket upgrade is rejected without a valid JWT
- Mode: AFK
- Files: create `apps/server/middleware/jwt_middleware.hpp`, create `apps/server/middleware/jwt_middleware.cpp`, modify `apps/server/controllers/auth_controller.cpp` (register middleware on all non-auth routes)
- Depends on: TASK-003
- Complexity: S
- AC:
  - Valid JWT in `Authorization: Bearer` header allows request to proceed to handler
  - Missing JWT returns HTTP 401
  - Expired JWT returns HTTP 401
  - Tampered JWT (signature mismatch) returns HTTP 401
  - Valid JWT on WebSocket upgrade (`/ws`) allows connection; connection is associated with the authenticated player UUID
  - Invalid or missing JWT on WebSocket upgrade returns HTTP 401 and closes connection
  - `/auth/google` and `/auth/apple` are exempt from JWT middleware

---

### TASK-008: Game mode config seeding + GET /game-modes

- Spec: specs/game-modes.md
- Slice: Active game modes from `game_modes.json` are visible in `GET /game-modes` after server start
- Mode: AFK
- Files: create `apps/server/config/game_modes.json`, create `apps/server/services/game_mode_service.hpp`, create `apps/server/services/game_mode_service.cpp`, create `apps/server/controllers/game_mode_controller.hpp`, create `apps/server/controllers/game_mode_controller.cpp`
- Depends on: TASK-002
- Complexity: S
- AC:
  - Server reads `game_modes.json` at startup and upserts each entry into `game_modes` table
  - Server refuses to start if `game_modes.json` is missing or malformed (fail fast, log error)
  - `GET /game-modes` returns all rows where `active=true` with fields: `id`, `name`, `rules_engine`, `stake`, `turn_timer_secs`, `grace_timer_secs`
  - Modes with `active=false` are excluded from the response
  - `GET /game-modes` without valid JWT returns HTTP 401
  - Game mode with `stake=0` in JSON is rejected at startup

---

### Wave 4 — Player APIs + WebSocket Infrastructure

---

### TASK-006: Player profile — GET /players/me

- Spec: specs/player-account.md
- Slice: An authenticated player can retrieve their profile and current balance
- Mode: AFK
- Files: create `apps/server/controllers/player_controller.hpp`, create `apps/server/controllers/player_controller.cpp`, create `apps/server/services/player_service.hpp`, create `apps/server/services/player_service.cpp`
- Depends on: TASK-005
- Complexity: S
- AC:
  - `GET /players/me` returns HTTP 200 with `id`, `display_name`, `points_balance`, `created_at`
  - Response reflects current `points_balance` from Postgres (not cached)
  - Unauthenticated request returns HTTP 401

---

### TASK-007: Daily bonus — POST /players/me/daily-bonus

- Spec: specs/player-account.md
- Slice: An authenticated player can claim 200 points once per 24 hours; duplicate claims within the window are rejected
- Mode: AFK
- Files: modify `apps/server/controllers/player_controller.cpp`, modify `apps/server/services/player_service.cpp`
- Depends on: TASK-005
- Complexity: S
- AC:
  - `POST /players/me/daily-bonus` on first claim (or after 24h) returns HTTP 200 with updated balance
  - `users.points_balance` increased by 200 atomically with `point_transactions` row (`reason=daily_bonus`, `delta=200`)
  - Claim within 24h window returns HTTP 429 with `retry_after_seconds` field; balance unchanged
  - Two simultaneous bonus claims produce exactly one credit (idempotent)
  - Unauthenticated request returns HTTP 401

---

### TASK-009: Friend request — POST /friends/request + WS notification

- Spec: specs/friends.md
- Slice: A player can send a friend request and the recipient receives it as a WebSocket message if online
- Mode: AFK
- Files: create `apps/server/controllers/friend_controller.hpp`, create `apps/server/controllers/friend_controller.cpp`, create `apps/server/services/friend_service.hpp`, create `apps/server/services/friend_service.cpp`
- Depends on: TASK-005
- Complexity: S
- AC:
  - `POST /friends/request { target_id }` inserts `friendships` row with `status=pending`
  - If target player has active WS connection, they receive `friend_request { from_player }` message
  - Self-request returns HTTP 400
  - Request to non-existent player returns HTTP 404
  - Duplicate request to same pending target returns HTTP 409
  - Request to already-accepted friend returns HTTP 409
  - Unauthenticated request returns HTTP 401

---

### TASK-011: WebSocket connection handler + PlayerRegistry

- Spec: specs/matchmaking.md, docs/architecture.md (ADR-005)
- Slice: Players connect to `/ws` with a valid JWT; their connection is registered and retrievable by player ID; disconnect is handled cleanly
- Mode: HITL
- Files: create `apps/server/ws/ws_handler.hpp`, create `apps/server/ws/ws_handler.cpp`, create `apps/server/ws/player_registry.hpp`, create `apps/server/ws/player_registry.cpp`
- Depends on: TASK-005
- Complexity: M
- AC:
  - WS connection on `/ws` with valid JWT registers player in `PlayerRegistry` (player UUID → connection)
  - WS connection with invalid JWT is rejected (HTTP 401 on upgrade)
  - On disconnect, player is removed from `PlayerRegistry` automatically
  - `PlayerRegistry::find(playerId)` returns nullptr if player not connected
  - `PlayerRegistry` reads protected by `std::shared_mutex`; writes by exclusive lock
  - One player may only have one active WS connection; second connection replaces the first
  - Incoming WS messages are routed by `type` field to the appropriate handler

---

### Wave 5 — Social Features

---

### TASK-010: Friend management — POST /friends/accept, GET /friends, DELETE /friends/:id

- Spec: specs/friends.md
- Slice: Players can accept friend requests, view their friend list, and remove friends
- Mode: AFK
- Files: modify `apps/server/controllers/friend_controller.cpp`, modify `apps/server/services/friend_service.cpp`
- Depends on: TASK-009
- Complexity: S
- AC:
  - `POST /friends/accept { requester_id }` updates pending row to `status=accepted`
  - Accept on non-existent or misdirected request returns HTTP 404
  - `GET /friends` returns all accepted friendships for calling player with `friend_id`, `display_name`, `points_balance`
  - `DELETE /friends/:id` removes the friendship row regardless of which side requested originally
  - Unauthenticated requests return HTTP 401

---

### TASK-013: Private match invite via WebSocket

- Spec: specs/matchmaking.md
- Slice: A player can send a match invite to an accepted friend over WebSocket; the friend can accept or reject within 60 seconds
- Mode: AFK
- Files: create `apps/server/ws/handlers/invite_handler.hpp`, create `apps/server/ws/handlers/invite_handler.cpp`
- Depends on: TASK-010, TASK-011
- Complexity: S
- AC:
  - `invite_friend { friend_id, game_mode_id }` stores invite in Redis with 60s TTL and sends `friend_invite` WS message to recipient if online
  - Invite to a non-accepted friend returns an error message to sender
  - `accept_invite { invite_id }` creates match, sends `match_found` to both, removes Redis key
  - `reject_invite { invite_id }` removes Redis key silently
  - Accept after TTL expiry returns error "invite expired"
  - Inviter with insufficient balance for the game mode is rejected

---

### Wave 6 — Public Matchmaking

---

### TASK-012: Public matchmaking queue — join/leave/match

- Spec: specs/matchmaking.md
- Slice: Two players can join a queue for the same game mode and be matched; both receive match_found over WebSocket
- Mode: HITL
- Files: create `apps/server/services/match_maker_service.hpp`, create `apps/server/services/match_maker_service.cpp`, create `apps/server/ws/handlers/queue_handler.hpp`, create `apps/server/ws/handlers/queue_handler.cpp`
- Depends on: TASK-008, TASK-010, TASK-011
- Complexity: M
- AC:
  - `join_queue { game_mode_id }` adds player to tail of `queue:{game_mode_id}` Redis list
  - Duplicate `join_queue` for same mode is idempotent — player not added twice
  - `leave_queue { game_mode_id }` removes player from Redis list
  - Player disconnecting while in queue is removed automatically
  - When two players are in queue, matchmaker atomically dequeues both (Lua script) and creates match
  - Both players receive `match_found { match_id, opponent, game_mode, board_state, side_to_move }`
  - `matches` row created with `status=in_progress`
  - Player with insufficient balance for game mode is rejected on `join_queue`
  - Concurrent matchmaking attempts cannot double-match the same player

---

### Wave 7 — Points Economy

---

### TASK-014: Match escrow — atomic stake deduction on match creation

- Spec: specs/points-economy.md
- Slice: When a match is created, both players' balances are atomically debited and the transaction is recorded; insufficient balance prevents match creation
- Mode: HITL
- Files: create `apps/server/services/escrow_service.hpp`, create `apps/server/services/escrow_service.cpp`
- Depends on: TASK-012
- Complexity: M
- AC:
  - Match creation atomically debits both players' `points_balance` by `stake` in one Postgres transaction
  - Two `point_transactions` rows inserted per match: one per player with `reason=match_escrow`, `delta=-stake`
  - If either player's balance would go below 0, the entire transaction rolls back and match is not created
  - Partial failure (one debit succeeds, second fails) rolls back both
  - Postgres `CHECK >= 0` constraint enforced — balance can never go negative regardless of race
  - Concurrent double-join attempts for same player produce at most one successful escrow

---

### TASK-015: Winner transfer + disconnect resolution

- Spec: specs/points-economy.md
- Slice: When a game ends, the winner's balance is credited; single disconnect awards points to opponent; double disconnect refunds both
- Mode: AFK
- Files: modify `apps/server/services/escrow_service.cpp`, create `apps/server/services/match_resolver.hpp`, create `apps/server/services/match_resolver.cpp`
- Depends on: TASK-014
- Complexity: M
- AC:
  - On `game_over`, winner's balance credited with `stake * 2` atomically; `point_transactions` row inserted with `reason=match_win`
  - Match row updated to `status=completed`, `winner_id` set, `ended_at` set
  - Single player disconnect: remaining connected player immediately declared winner; same credit flow as normal win
  - Double disconnect: both players refunded `stake` each atomically; `point_transactions` rows with `reason=match_refund`; match marked `status=forfeited`, `forfeit_reason=double_disconnect`
  - `game_over` WS message sent to both connected players with `winner_id`, `points_delta`, `final_board`
  - Duplicate resolution attempt on same match is idempotent (check match status before executing)

---

### Wave 8 — Gameplay Core

*Prerequisites: GitHub issues #1 (immutable GameState), #2 (event log), #3 (move history) must be resolved in the engine library before starting this wave.*

---

### TASK-016: Move validation + broadcast — core game loop

- Spec: specs/gameplay.md
- Slice: A player can submit a move; the server validates it, applies it, and broadcasts the new board state to both players in real time
- Mode: HITL
- Files: create `apps/server/ws/handlers/move_handler.hpp`, create `apps/server/ws/handlers/move_handler.cpp`, create `apps/server/game/game_session_manager.hpp`, create `apps/server/game/game_session_manager.cpp`, modify `src/game/match_session.cpp` (wire strand, PlayerRegistry)
- Depends on: TASK-015
- Complexity: L
- AC:
  - `make_move { match_id, step }` validated against current `GameState` by `VanillaRulesEngine`
  - Legal move: `GameState` updated (immutable — returns new state per issue #1), `move_applied` broadcast to both players
  - Illegal move: `illegal_move { reason }` sent only to mover; state unchanged
  - Out-of-turn move rejected with `illegal_move`
  - Move on non-`in_progress` match rejected
  - `match_events` row appended per move with correct `seq` and `payload`
  - Each `MatchSession` runs on its own trantor strand (issue #6 prerequisite)
  - Concurrent moves from two players to same session serialized by strand — no state corruption

---

### TASK-017: Turn management — turn_start signal, forced pass, opening roll

- Spec: specs/gameplay.md
- Slice: Turn transitions are handled automatically: turn_start sent at game start and after each move sequence; turns pass automatically when no legal moves remain
- Mode: AFK
- Files: modify `apps/server/ws/handlers/move_handler.cpp`, modify `apps/server/game/game_session_manager.cpp`
- Depends on: TASK-016
- Complexity: M
- AC:
  - `turn_start { match_id, side, turn_timer_secs, grace_remaining_secs }` sent to both players at game start and after each turn ends
  - Opening roll determines first mover; re-rolled if equal
  - Doubles produce 4 dice of same value
  - Player must use maximum number of dice legally possible (issue #4 enforcement)
  - When no legal moves remain for remaining dice, server automatically passes turn and sends `turn_start` for opponent
  - Premature turn-end attempt while legal moves exist is rejected

---

### Wave 9 — Timers + Forfeit

*Prerequisite: GitHub issue #4 (max dice enforcement) must be resolved before TASK-017 AC enforcement.*

---

### TASK-018: Per-player turn timer + grace timer + forfeit

- Spec: specs/gameplay.md
- Slice: Turn and grace timers run server-side; a player who exhausts their grace timer is forfeited automatically
- Mode: HITL
- Files: create `apps/server/game/timer_manager.hpp`, create `apps/server/game/timer_manager.cpp`, modify `apps/server/game/game_session_manager.cpp`
- Depends on: TASK-017
- Complexity: M
- AC:
  - Turn timer starts when `turn_start` is sent; duration = game mode's `turn_timer_secs`
  - On turn timer expiry, `timer_warning { match_id, side, grace_remaining_secs }` sent to both players
  - Grace timer per player decrements from `grace_timer_secs` each time turn timer overruns
  - Grace timer persists across turns (e.g. 11s used on turn 1 → 49s remaining on turn 2 of 60s total)
  - Grace timer reaching 0: match forfeited, opponent declared winner, `game_over` sent, `forfeit_reason=grace_timer`
  - Timer cancelled immediately when player submits a move (no spurious expiry)
  - Timer state runs on match's trantor strand — no race between timer callback and move handler
  - Timer config (both values) sourced from match's copied game mode values, not live config

---

### Wave 10 — Leaderboard

---

### TASK-019: Global leaderboard — GET /leaderboard

- Spec: specs/leaderboard.md
- Slice: An authenticated player can retrieve the global top 50 players by balance with their own rank always included
- Mode: AFK
- Files: create `apps/server/controllers/leaderboard_controller.hpp`, create `apps/server/controllers/leaderboard_controller.cpp`, create `apps/server/services/leaderboard_service.hpp`, create `apps/server/services/leaderboard_service.cpp`
- Depends on: TASK-006
- Complexity: S
- AC:
  - `GET /leaderboard` returns top 50 players ranked by `points_balance DESC` with fields: `rank`, `player_id`, `display_name`, `points_balance`
  - Default `limit=50`; `offset` defaults to 0
  - `?offset=50&limit=50` returns players 51–100
  - `limit > 100` clamped to 100
  - `limit=0` or negative `offset` returns HTTP 400
  - Response includes `my_rank { rank, points_balance }` for the requesting player regardless of page
  - Tiebreaker on equal balance: `created_at ASC` (earlier account ranks higher) — deterministic
  - Unauthenticated request returns HTTP 401

---

### TASK-020: Friends leaderboard — GET /leaderboard/friends

- Spec: specs/leaderboard.md
- Slice: A player can view their accepted friends ranked by balance
- Mode: AFK
- Files: modify `apps/server/controllers/leaderboard_controller.cpp`, modify `apps/server/services/leaderboard_service.cpp`
- Depends on: TASK-019, TASK-010
- Complexity: S
- AC:
  - `GET /leaderboard/friends` returns only accepted friends of calling player ranked by `points_balance DESC`
  - Same field structure as global leaderboard (`rank`, `player_id`, `display_name`, `points_balance`)
  - Player with no accepted friends receives empty list with HTTP 200
  - Supports same `offset` and `limit` params as global leaderboard
  - Unauthenticated request returns HTTP 401

---

## Dependency Graph

```
TASK-001
TASK-002 → TASK-001
TASK-003 → TASK-002
TASK-004 → TASK-002
TASK-005 → TASK-003
TASK-006 → TASK-005
TASK-007 → TASK-005
TASK-008 → TASK-002
TASK-009 → TASK-005
TASK-010 → TASK-009
TASK-011 → TASK-005
TASK-012 → TASK-008, TASK-010, TASK-011
TASK-013 → TASK-010, TASK-011
TASK-014 → TASK-012
TASK-015 → TASK-014
TASK-016 → TASK-015
TASK-017 → TASK-016
TASK-018 → TASK-017
TASK-019 → TASK-006
TASK-020 → TASK-019, TASK-010
```

---

## Critical Path

TASK-001 → TASK-002 → TASK-003 → TASK-005 → TASK-011 → TASK-012 → TASK-014 → TASK-015 → TASK-016 → TASK-017 → TASK-018

Estimated: 10–15 weeks solo (M+S+M+S+M+M+M+M+L+M+M)

---

## Risk Areas

- **TASK-016 (gameplay core)**: Largest task. Wires the C++ engine library into the Drogon async model. Requires issues #1–#3 resolved first. Strand integration makes testing harder. Budget extra time.
- **TASK-018 (timers)**: Timer callbacks firing on a strand while moves are also arriving on the same strand — timing-sensitive. Must be tested under load.
- **TASK-014 (escrow)**: ACID transaction with two debits. Any mistake here corrupts user balances permanently. Requires thorough concurrency testing.
- **TASK-003/004 (OAuth)**: Depends on external JWKS endpoints. Test with mocked JWKS. Apple Sign-In has documented quirks with `kid` rotation — verify against latest Apple docs at implementation time.
- **TASK-012 (matchmaking)**: Redis Lua script for atomic dequeue must be correct. A bug here double-matches players or drops them from queues silently.
- **Engine issues #1–#4**: TASK-016 and TASK-017 are blocked until these are done. If they slip, Wave 8–9 slips entirely.
