# Game Modes + Lobby

## Description

Game modes are configurations that define a backgammon variant's stake, timer settings, and rules engine. They are loaded from a JSON config file at server startup and served to clients via REST. Players are blocked from entering a game mode they cannot afford.

## Acceptance Criteria

1. **Given** a `game_modes.json` file is present at server startup, **When** the server starts, **Then** each mode in the file is upserted into the `game_modes` table (insert on new `id`, update fields on existing `id`).

2. **Given** the server has started and game modes are seeded, **When** an authenticated player calls `GET /game-modes`, **Then** the response returns all modes where `active=true` with fields: `id`, `name`, `rules_engine`, `stake`, `turn_timer_secs`, `grace_timer_secs`.

3. **Given** a game mode has `active=false`, **When** `GET /game-modes` is called, **Then** that mode is excluded from the response.

4. **Given** an unauthenticated request, **When** `GET /game-modes` is called, **Then** the server returns HTTP 401.

5. **Given** a player with `points_balance` less than a game mode's `stake`, **When** they attempt to join that game mode's queue or accept an invite for it, **Then** the server rejects the action with an appropriate error and the player's balance is unchanged.

6. **Given** a player with `points_balance` equal to or greater than a game mode's `stake`, **When** they attempt to join that game mode's queue, **Then** the action is accepted (stake enforcement passes).

7. **Given** a match starts, **When** the match record is created, **Then** the `stake` and timer values from the game mode at that moment are copied into the match record so future config changes do not affect the in-progress game.

8. **Given** `game_modes.json` is missing or malformed at startup, **When** the server starts, **Then** the server logs an error and refuses to start (fail fast).

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| `game_modes.json` contains a mode with `stake=0` | Rejected at startup — `stake` must be > 0 per schema constraint |
| Game mode `id` in JSON matches existing DB row | Row is updated (upsert), no duplicate created |
| Player balance equals exactly the stake | Allowed to join |
| Player balance is 0 | Blocked from all game modes |
| `game_modes.json` updated and server restarted while games are active | In-flight matches use their copied values; new matches use updated config |

## Test Requirements

- AC1: Integration test — start server with known JSON, assert DB rows match
- AC2: Integration test — GET /game-modes returns only active modes with correct fields
- AC3: Integration test — inactive mode excluded from response
- AC4: Unit test — unauthenticated request returns 401
- AC5: Integration test — join attempt with insufficient balance rejected
- AC6: Integration test — join attempt with sufficient balance accepted
- AC7: Integration test — match record contains copied timer/stake values independent of later config changes
- AC8: Integration test — server fails to start with missing/malformed JSON

## Dependencies

- `specs/auth.md` — GET /game-modes requires JWT

## Out of Scope

- Admin endpoint to create/update game modes at runtime
- Game modes with rules engines other than `vanilla`
- Changing a game mode's stake while players are in queue for it

## Open Questions

- None.
