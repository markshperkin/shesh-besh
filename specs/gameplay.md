# Gameplay

## Description

Real-time backgammon match played over WebSocket. The server is authoritative: it validates every move, maintains game state, enforces turn and grace timers per player, broadcasts state to both players after each move, and determines the winner.

## Acceptance Criteria

1. **Given** a match has started, **When** it is a player's turn, **Then** the server sends `turn_start { match_id, side, turn_timer_secs, grace_remaining_secs }` to both players.

2. **Given** it is a player's turn, **When** they send `make_move { match_id, step }`, **Then** the server validates the step against the current `GameState` using the rules engine.

3. **Given** the step is legal, **When** validated, **Then** the server applies it to the game state, appends a `move` event to the event log, and broadcasts `move_applied { match_id, step, board_state, side_to_move }` to both players.

4. **Given** the step is illegal, **When** validated, **Then** the server sends `illegal_move { match_id, reason }` only to the moving player and the game state is unchanged.

5. **Given** a player submits a move when it is not their turn, **When** the server receives it, **Then** it is rejected with `illegal_move` and the game state is unchanged.

6. **Given** a player has remaining legal moves for their remaining dice, **When** they attempt to end their turn early, **Then** the server rejects this and requires the player to use the maximum number of dice possible.

7. **Given** a player has no legal moves for their remaining dice, **When** this is detected, **Then** the server automatically passes the turn and sends `turn_start` for the opponent.

8. **Given** the turn timer expires for the active player, **When** the timer elapses, **Then** the server sends `timer_warning { match_id, side, grace_remaining_secs }` to both players and begins deducting from that player's grace timer.

9. **Given** the grace timer for a player reaches zero, **When** this occurs, **Then** the server declares the opponent the winner, the match is marked `forfeited` with `forfeit_reason=grace_timer`, and `game_over` is sent to both players.

10. **Given** all 15 checkers of one side are borne off, **When** this is detected after a move is applied, **Then** the server declares that player the winner, sends `game_over { match_id, winner_id, points_delta, final_board }` to both players, and the match is marked `completed`.

11. **Given** a game ends (win or forfeit), **When** `game_over` is sent, **Then** the points transfer is executed atomically (see `specs/points-economy.md`).

12. **Given** a move is applied, **When** the event log is updated, **Then** a `match_events` row is inserted with the correct `seq`, `event_type=move`, and `payload` containing the step and resulting board state.

13. **Given** doubles are rolled, **When** the turn begins, **Then** the player receives 4 dice of the same value and must use as many as legally possible.

14. **Given** a player sends `make_move` for a match that is not in `in_progress` status, **When** the server receives it, **Then** it is rejected with an error.

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| Player sends two moves simultaneously (race) | Strand serializes both; second processed after first; illegal if state changed |
| Move references a non-existent match_id | Rejected with error |
| Player disconnects mid-turn | Opponent's turn continues; disconnect rule from economy spec applies |
| Board state diverges between client and server | Server state is authoritative; clients must use server-provided `board_state` |
| Grace timer reaches exactly 0 during move processing | Move is applied if it arrived before expiry; forfeit triggered after |
| Opening roll produces equal dice | Server re-rolls until unequal; first mover determined |
| Player has checkers on the bar and tries to move a board piece | Rejected as illegal — must re-enter from bar first |

## Test Requirements

- AC1: Integration test — turn_start sent to both players at match start and after each turn
- AC2–3: Integration test — legal move applied, move_applied broadcast to both
- AC4: Integration test — illegal move rejected, game state unchanged
- AC5: Integration test — out-of-turn move rejected
- AC6: Integration test — early turn-end rejected when legal moves remain
- AC7: Integration test — forced pass when no legal moves; turn switches automatically
- AC8: Integration test — turn timer expiry triggers timer_warning and grace deduction
- AC9: Integration test — grace timer expiry triggers forfeit and game_over
- AC10: Integration test — bearing off 15th checker triggers game_over with correct winner
- AC11: Integration test — points transfer called on game_over (cross-spec)
- AC12: Integration test — match_events row inserted per move with correct seq
- AC13: Integration test — doubles produce 4 dice; all used or skipped only if blocked
- AC14: Unit test — make_move on non-in_progress match rejected

## Dependencies

- `specs/auth.md` — WebSocket requires valid JWT
- `specs/matchmaking.md` — match must exist and be in_progress
- `specs/points-economy.md` — game_over triggers point transfer

## Out of Scope

- Doubling cube
- Gammons / backgammons scoring multipliers
- Spectator move observation
- Move undo
- In-match chat

## Open Questions

- None.
