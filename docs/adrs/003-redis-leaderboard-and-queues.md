# ADR-003: Redis for Matchmaking Queues and Ephemeral State

## Status
Accepted (revised — leaderboard moved to Postgres only)

## Context
Matchmaking queues need atomic FIFO operations: enqueue a player, dequeue a matched pair atomically so two concurrent match attempts can't claim the same player. Friend match invites are ephemeral (60s TTL). These are not a good fit for Postgres row locking.

The leaderboard was initially considered for Redis (sorted set), but the expected user base does not justify the operational complexity of keeping Postgres and Redis in sync. Postgres with an index on `points_balance DESC` is sufficient.

## Decision
Use **Redis** for matchmaking queues and ephemeral invites only. Leaderboard stays in Postgres.

## Data Structures
- `queue:{game_mode_id}` → List (`RPUSH` to enqueue, `LPOP`×2 atomically via Lua script to dequeue a pair)
- `invite:{uuid}` → Hash with TTL (60s ephemeral friend match invites)

## Reasons
- Atomic `LPOP` on list prevents double-matching the same player under concurrent requests
- TTL on invite keys requires no cleanup job — Redis handles expiry natively
- Matchmaking queue has no persistent value — data loss on Redis restart is acceptable (players re-queue)

## Rejected Alternatives
- **Postgres queue**: achievable with `SELECT ... FOR UPDATE SKIP LOCKED`, but adds schema complexity and lock contention for a use case Redis handles trivially
- **Redis leaderboard sorted set**: rejected — adds sync complexity (Postgres + Redis must stay consistent on every point transaction) for a scale we are unlikely to reach

## Consequences
- `docker-compose` includes a Redis service
- Hiredis (or Drogon's built-in Redis client) for Redis access
- Redis restart loses queued players — acceptable, they reconnect and re-queue
- Leaderboard query: `SELECT id, display_name, points_balance, RANK() OVER (ORDER BY points_balance DESC) FROM users LIMIT 100` with index on `(points_balance DESC)`
