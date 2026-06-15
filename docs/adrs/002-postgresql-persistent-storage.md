# ADR-002: PostgreSQL for Persistent Storage

## Status
Accepted

## Context
The system needs to persist: user accounts, point balances, match history, event logs, friendships, and point transaction audit logs. The points economy requires atomic debit+credit operations (escrow). Deployment is a single VPS with rootless Docker.

## Decision
Use **PostgreSQL** as the primary persistent data store.

## Reasons
- ACID transactions required for escrow: deduct both players' stakes and credit winner atomically — no partial failures
- Relational model fits the data naturally (users → matches → events, friendships, transactions)
- `points_balance CHECK >= 0` constraint enforced at DB level — impossible to go negative even under race conditions
- Mature C++ client (`libpq`, or Drogon's built-in ORM/raw query support)
- Runs well in rootless Docker

## Rejected Alternatives
- **SQLite**: can't handle concurrent writes from Drogon's thread pool without serialization; not suitable for a multi-connection server
- **Redis only**: no relational queries, no ACID multi-key transactions for balance ops, data loss risk without persistence config
- **MongoDB**: document model adds no value here; ACID multi-document transactions are more complex than Postgres

## Consequences
- `docker-compose` includes a Postgres service
- Schema managed via SQL migration files (one file per version, applied in order)
- Drogon connects via a connection pool (configurable pool size in `config.json`)
- Redis used alongside Postgres for read-optimized structures (leaderboard, queues) — see ADR-003
