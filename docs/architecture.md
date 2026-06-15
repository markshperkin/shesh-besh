# Architecture: Shesh-Besh

## System Overview

Shesh-Besh is a real-time multiplayer backgammon game server written in C++20. It serves cross-platform clients (Android, iOS, Web) over WebSocket for gameplay and REST for lobby/social features. The primary quality attributes are: **low-latency move delivery** (sub-100ms push to opponent), **correctness** (server-authoritative game state, no client trust), **points economy integrity** (ACID balance operations, no negative balances), and **concurrent game scalability** (thousands of simultaneous matches on a single VPS). Authentication is handled via OAuth2 (Google/Apple) with the server issuing its own JWTs.

---

## Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        Clients                              │
│          Android  ·  iOS  ·  Web                            │
└──────────────┬─────────────────┬───────────────────────────┘
               │ HTTPS (REST)    │ WSS (WebSocket)
               ▼                 ▼
┌──────────────────────────────────────────────────────────────┐
│                  Nginx (TLS termination)                      │
└──────────────┬─────────────────┬────────────────────────────┘
               │                 │
               ▼                 ▼
┌──────────────────────────────────────────────────────────────┐
│                   Drogon App Server                           │
│                                                              │
│  ┌─────────────────┐   ┌──────────────────────────────────┐ │
│  │   REST Router   │   │       WebSocket Handler           │ │
│  │                 │   │                                   │ │
│  │ /auth/*         │   │  on_connect  → auth JWT           │ │
│  │ /players/*      │   │  on_message  → route by type      │ │
│  │ /game-modes     │   │  on_close    → disconnect logic   │ │
│  │ /leaderboard    │   └────────────┬─────────────────────┘ │
│  │ /friends        │                │                        │
│  └────────┬────────┘                │                        │
│           │                         │                        │
│           └──────────┬──────────────┘                        │
│                      │                                        │
│           ┌──────────▼──────────────┐                        │
│           │      JWT Middleware      │                        │
│           └──────────┬──────────────┘                        │
│                      │                                        │
│        ┌─────────────▼──────────────────────┐                │
│        │           Service Layer             │                │
│        │                                    │                │
│        │  AuthService      PlayerService    │                │
│        │  MatchMaker       EscrowService    │                │
│        │  LeaderboardSvc   FriendService    │                │
│        └──────┬──────────────────┬──────────┘                │
│               │                  │                            │
│        ┌──────▼──────┐   ┌───────▼──────────────────────┐   │
│        │  GameSession│   │       PlayerRegistry          │   │
│        │  Manager    │   │  (playerId → WsConnection)    │   │
│        └──────┬──────┘   └───────────────────────────────┘   │
│               │                                               │
│        ┌──────▼──────────────────────────────────────┐       │
│        │  MatchSession (one per active game)          │       │
│        │  · owns a trantor strand                     │       │
│        │  · VanillaRulesEngine (or future engines)    │       │
│        │  · GameState, EventLog, TurnHistory          │       │
│        │  · Turn timer + Grace timer (per player)     │       │
│        └─────────────────────────────────────────────┘        │
└──────────────────────────────────────────────────────────────┘
               │                  │
               ▼                  ▼
┌──────────────────┐   ┌────────────────────────────────────┐
│   PostgreSQL     │   │              Redis                  │
│                  │   │                                     │
│ users            │   │ queue:{game_mode_id}  (list)        │
│ matches          │   │ leaderboard           (sorted set)  │
│ match_events     │   │ invite:{invite_id}    (hash, TTL)   │
│ point_tx         │   └────────────────────────────────────┘
│ friendships      │
│ game_modes       │
└──────────────────┘
```

---

## Data Model

**Storage choice:** PostgreSQL for all persistent data. The points economy requires ACID transactions (escrow deduction + winner credit must be atomic). SQLite won't survive concurrent writes at scale; Redis alone can't give us relational queries (friends-of-friends, leaderboard with friend filter). See ADR-002.

Redis as a cache + real-time layer: matchmaking queues (list), leaderboard sorted set (O(log N) rank), and ephemeral match invites (TTL hash). See ADR-003.

### PostgreSQL Tables

**users**
```
id                UUID        PK
oauth_provider    VARCHAR     ('google' | 'apple')
oauth_subject     VARCHAR     UNIQUE (provider sub claim)
display_name      VARCHAR
points_balance    BIGINT      DEFAULT 1000, CHECK >= 0
last_daily_bonus  TIMESTAMPTZ nullable
created_at        TIMESTAMPTZ DEFAULT now()

INDEX: (oauth_provider, oauth_subject)  -- fast login lookup
INDEX: (points_balance DESC)            -- leaderboard fallback
```

**game_modes** (config-driven, seeded at startup)
```
id                VARCHAR     PK  (e.g. 'vanilla_100')
name              VARCHAR
rules_engine      VARCHAR     ('vanilla' | future engines)
stake             INT         CHECK > 0
turn_timer_secs   INT
grace_timer_secs  INT
active            BOOLEAN     DEFAULT true
```

**matches**
```
id                UUID        PK
game_mode_id      VARCHAR     FK → game_modes
white_player_id   UUID        FK → users
black_player_id   UUID        FK → users
status            ENUM        (queued, in_progress, completed, forfeited, cancelled)
winner_id         UUID        FK → users, nullable
forfeit_reason    ENUM        (grace_timer, single_disconnect, double_disconnect) nullable
started_at        TIMESTAMPTZ
ended_at          TIMESTAMPTZ nullable
```

**match_events** (append-only event log)
```
id                BIGSERIAL   PK
match_id          UUID        FK → matches
seq               INT         (sequence within match)
event_type        ENUM        (move, turn_start, turn_end, game_over, forfeit)
payload           JSONB       (Step, Side, board snapshot, etc.)
created_at        TIMESTAMPTZ DEFAULT now()

INDEX: (match_id, seq)
```

**point_transactions** (immutable audit log — never update, only insert)
```
id                BIGSERIAL   PK
user_id           UUID        FK → users
delta             BIGINT      (positive = credit, negative = debit)
reason            ENUM        (signup_bonus, daily_bonus, match_escrow,
                               match_win, match_refund)
match_id          UUID        FK → matches, nullable
created_at        TIMESTAMPTZ DEFAULT now()

INDEX: (user_id, created_at DESC)
```

**friendships**
```
requester_id      UUID        FK → users
addressee_id      UUID        FK → users
status            ENUM        (pending, accepted)
created_at        TIMESTAMPTZ
PK: (requester_id, addressee_id)
CONSTRAINT: requester_id <> addressee_id
```

### Redis Keys

| Key pattern | Type | TTL | Purpose |
|---|---|---|---|
| `queue:{game_mode_id}` | List | none | Matchmaking FIFO per mode |
| `invite:{uuid}` | Hash | 60s | Pending friend match invites |

**Leaderboard** is served directly from Postgres (`points_balance DESC` index). Redis is not used for ranking — user base is not expected to reach the scale where a sorted set would outperform an indexed Postgres query.

---

## API Boundaries

### REST (HTTPS)

| Method | Path | Responsibility | Consumer |
|---|---|---|---|
| POST | `/auth/google` | Exchange Google ID token → server JWT | Client on login |
| POST | `/auth/apple` | Exchange Apple ID token → server JWT | Client on login |
| GET | `/players/me` | Current player profile + balance | Client lobby |
| POST | `/players/me/daily-bonus` | Claim 200pt daily bonus (idempotent per 24h) | Client |
| GET | `/game-modes` | List active game modes with config | Client lobby |
| GET | `/leaderboard` | Top N players by balance (default 100) | Client |
| GET | `/leaderboard/friends` | Authenticated player's friends ranked | Client |
| GET | `/friends` | Friend list with status | Client |
| POST | `/friends/request` | Send friend request `{ target_id }` | Client |
| POST | `/friends/accept` | Accept request `{ requester_id }` | Client |
| DELETE | `/friends/:id` | Remove friend | Client |
| GET | `/matches/:id` | Match result + event log (for history) | Client post-game |

All routes except `/auth/*` require `Authorization: Bearer <jwt>`.

### WebSocket (`WSS /ws`)

JWT passed as `Authorization` header on upgrade. Connection is per-player; one connection per logged-in client.

**Client → Server messages:**

| Type | Payload | Description |
|---|---|---|
| `join_queue` | `{ game_mode_id }` | Enter matchmaking queue |
| `leave_queue` | `{ game_mode_id }` | Leave queue |
| `make_move` | `{ match_id, step: { from, to, die } }` | Submit a move |
| `invite_friend` | `{ friend_id, game_mode_id }` | Send private match invite |
| `accept_invite` | `{ invite_id }` | Accept private match invite |
| `reject_invite` | `{ invite_id }` | Decline invite |

**Server → Client messages:**

| Type | Payload | Description |
|---|---|---|
| `match_found` | `{ match_id, opponent, game_mode, board_state, side_to_move }` | Match started |
| `turn_start` | `{ match_id, side, turn_timer_secs, grace_remaining_secs }` | New turn begins |
| `move_applied` | `{ match_id, step, board_state, side_to_move }` | Move broadcast to both |
| `illegal_move` | `{ match_id, reason }` | Move rejected, sent only to mover |
| `timer_warning` | `{ match_id, side, grace_remaining_secs }` | Turn timer expired, grace ticking |
| `game_over` | `{ match_id, winner_id, points_delta, final_board }` | Game ended |
| `friend_invite` | `{ invite_id, from_player, game_mode }` | Incoming match invite |
| `friend_request` | `{ from_player }` | Incoming friend request |

---

## Auth Strategy

**Flow:**
1. Client authenticates with Google/Apple, receives an ID token (OIDC).
2. Client sends ID token to `/auth/google` or `/auth/apple`.
3. Server validates token against Google/Apple public keys (JWKS endpoint).
4. Server looks up or creates user by `(provider, sub)`. First login auto-creates account + grants 1,000 points.
5. Server issues its own JWT (RS256, 24h expiry) containing: `sub` (user UUID), `display_name`, `iat`, `exp`, `jti`.
6. Client sends JWT as `Authorization: Bearer` on all subsequent requests and WS upgrade.

**JWT validation middleware** runs before every route handler and WS upgrade. Rejects expired, tampered, or missing tokens with 401.

**Authorization:** No RBAC for MVP. All authenticated users have identical permissions. Admin operations (seeding game modes) are done via direct DB or a separate internal endpoint not exposed publicly.

See ADR-004.

---

## Infrastructure Decisions

### Deployment (VPS, rootless Docker)

```
docker-compose.yml
├── nginx          (TLS termination, reverse proxy → port 3000)
├── sheshbesh-app  (Drogon, port 3000 internal)
├── postgres       (port 5432 internal)
└── redis          (port 6379 internal)
```

- TLS certificates: Let's Encrypt via certbot (mounted into nginx container).
- All services on an internal Docker network; only Nginx exposed externally (443).
- Rootless Docker compatible (no privileged ports needed — Nginx binds 443 via `net.ipv4.ip_unprivileged_port_start=0` or a port forward).

### CI/CD (GitHub Actions)

Existing workflow builds and tests the C++ library. Extend with:
1. `docker build` on every push to `main`
2. Push image to GitHub Container Registry (GHCR)
3. SSH deploy step: pull new image, `docker-compose up -d` on VPS

### Observability

- **Logging:** `spdlog` (structured JSON) — request ID, player ID, match ID on every log line.
- **Metrics:** Drogon built-in `/metrics` endpoint (request count, latency p99, active WS connections). Scrape with Prometheus if needed later.
- **Error tracking:** stderr logs → Docker log driver → VPS log files. Upgrade to Sentry C++ SDK in v2.

### External Services

| Service | Purpose | MVP? |
|---|---|---|
| Google OAuth (JWKS) | Validate Google ID tokens | Yes |
| Apple OAuth (JWKS) | Validate Apple ID tokens | Yes |
| Let's Encrypt | TLS certificates | Yes |
| GHCR | Docker image registry | Yes |

---

## Key Tradeoffs

| Decision | Optimized for | Sacrificed |
|---|---|---|
| C++ game engine | Latency, CPU efficiency | Faster iteration speed, larger hiring pool |
| Drogon (async, strand-per-match) | Concurrent game throughput | Simpler threading model (one thread per game) |
| PostgreSQL for balances | ACID correctness, no lost points | Pure-Redis simplicity |
| Redis sorted set for leaderboard | O(log N) rank reads | Must keep in sync with Postgres on every tx |
| Server-authoritative game state | Cheat prevention | Client can't do optimistic UI without server round-trip |
| Single VPS | Ops simplicity, cost | Single point of failure, vertical scale only |
| JWT RS256, 24h expiry | Stateless validation, no DB hit per request | Tokens can't be revoked mid-session without a blocklist |

---

## Open Questions

| # | Question | Blocks |
|---|---|---|
| 1 | Disconnect rule: one player disconnects → opponent wins immediately and gets the points. Both disconnect simultaneously → full refund to both. No close-code analysis needed. | resolved |
| 2 | Leaderboard pagination: offset. `?offset=N&limit=50`. Rank changes between pages are acceptable. | resolved |
| 3 | Game mode seeding: `game_modes.json` loaded and upserted on server startup. Matches copy stake/timer values at start so in-flight games are unaffected by config changes. Graceful shutdown (drain active games before redeploy) added to parking lot. | resolved |
| 4 | JWT key storage: env var in `.env` file on VPS, injected via `docker-compose env_file`. `.env` is gitignored. Migrate to Docker secrets or a secrets manager if team grows. | resolved |
