# Leaderboard

## Description

A global leaderboard ranks all players by current point balance in descending order. Players can also view a filtered leaderboard showing only their accepted friends. Pagination is offset-based.

## Acceptance Criteria

1. **Given** an authenticated player calls `GET /leaderboard`, **When** the server responds, **Then** it returns a list of players ranked by `points_balance DESC` with fields: `rank`, `player_id`, `display_name`, `points_balance`.

2. **Given** `GET /leaderboard` is called with no query params, **When** the server responds, **Then** it returns the top 50 players (default limit).

3. **Given** `GET /leaderboard?offset=50&limit=50` is called, **When** the server responds, **Then** it returns players ranked 51–100 at the time of the query.

4. **Given** `GET /leaderboard` is called with `limit` exceeding 100, **When** the server processes the request, **Then** it clamps the limit to 100 and returns at most 100 results.

5. **Given** the leaderboard has fewer than `limit` players, **When** `GET /leaderboard` is called, **Then** all available players are returned with no error.

6. **Given** an authenticated player calls `GET /leaderboard/friends`, **When** the server responds, **Then** it returns only players who have an accepted friendship with the requesting player, ranked by `points_balance DESC`, with the same field structure as the global leaderboard.

7. **Given** a player has no accepted friends, **When** `GET /leaderboard/friends` is called, **Then** an empty list is returned with HTTP 200.

8. **Given** the authenticated player's own rank, **When** `GET /leaderboard` is called, **Then** the response includes a top-level field `my_rank` containing the requesting player's global rank and balance, regardless of whether they appear in the current page.

9. **Given** an unauthenticated request, **When** either leaderboard endpoint is called, **Then** HTTP 401 is returned.

10. **Given** two players have identical `points_balance`, **When** ranked, **Then** the server applies a consistent tiebreaker (e.g. `created_at ASC` — earlier account ranks higher) so order is deterministic.

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| `offset` larger than total player count | Empty list returned with HTTP 200 |
| `limit=0` | Return HTTP 400 — limit must be >= 1 |
| Player's balance changes between page 1 and page 2 request | Page 2 reflects updated state — rank drift between pages is acceptable |
| Player has no friends but calls `/leaderboard/friends` | Empty list, HTTP 200 |
| Negative `offset` | HTTP 400 |

## Test Requirements

- AC1–2: Integration test — GET /leaderboard returns top 50 with correct fields and rank values
- AC3: Integration test — offset pagination returns correct slice
- AC4: Unit test — limit > 100 is clamped to 100
- AC5: Integration test — leaderboard with < limit players returns all
- AC6: Integration test — friends leaderboard returns only accepted friends ranked correctly
- AC7: Integration test — empty friends list returns []
- AC8: Integration test — my_rank field present and correct regardless of page
- AC9: Unit test — unauthenticated request returns 401
- AC10: Integration test — identical balance produces deterministic ordering

## Dependencies

- `specs/auth.md` — both endpoints require JWT
- `specs/friends.md` — friends leaderboard requires accepted friendship records

## Out of Scope

- Weekly or monthly leaderboard resets (all-time only)
- Per-game-mode leaderboards
- Leaderboard push notifications

## Open Questions

- None.
