# Player Account + Daily Bonus

## Description

Players can retrieve their profile and current point balance. Once per 24-hour window, a player can claim a 200-point daily bonus to replenish their balance.

## Acceptance Criteria

1. **Given** an authenticated player, **When** they call `GET /players/me`, **Then** the response includes `id`, `display_name`, `points_balance`, and `created_at`.

2. **Given** an unauthenticated request, **When** `GET /players/me` is called, **Then** the server returns HTTP 401.

3. **Given** an authenticated player who has never claimed the daily bonus, **When** they call `POST /players/me/daily-bonus`, **Then** their `points_balance` increases by 200, the `last_daily_bonus` timestamp is set, and HTTP 200 is returned with the updated balance.

4. **Given** an authenticated player whose `last_daily_bonus` was set more than 24 hours ago, **When** they call `POST /players/me/daily-bonus`, **Then** their balance increases by 200 and the timestamp is updated.

5. **Given** an authenticated player whose `last_daily_bonus` was set within the last 24 hours, **When** they call `POST /players/me/daily-bonus`, **Then** HTTP 429 is returned, balance is unchanged, and the response includes the time remaining until the next claim.

6. **Given** the daily bonus is claimed, **When** the transaction is recorded, **Then** a `point_transactions` row is inserted with `reason=daily_bonus`, correct `delta=200`, and the player's `user_id`.

7. **Given** two simultaneous `POST /players/me/daily-bonus` requests from the same player, **When** both are processed, **Then** only one bonus is granted (idempotent — no double credit).

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| Player's balance is 0 before claiming bonus | Bonus is still granted; balance becomes 200 |
| Clock skew between server instances (future consideration) | Server clock is authoritative; `last_daily_bonus` compared against server UTC |
| Player account deleted between JWT issue and request | Return HTTP 404 |
| `last_daily_bonus` exactly 24 hours ago (boundary) | Bonus is claimable |

## Test Requirements

- AC1: Integration test — authenticated GET returns correct fields
- AC2: Unit test — unauthenticated request returns 401
- AC3: Integration test — first-time claim increases balance by 200 and inserts transaction row
- AC4: Integration test — claim after 24h window reopens correctly
- AC5: Integration test — claim within 24h returns 429 with time-remaining field
- AC6: Integration test — verify `point_transactions` row inserted with correct fields
- AC7: Concurrency test — two simultaneous requests for same player produce exactly one credit

## Dependencies

- `specs/auth.md` — all endpoints require valid JWT

## Out of Scope

- Streak bonuses (consecutive days)
- Referral points
- Ad-based point earning

## Open Questions

- None.
