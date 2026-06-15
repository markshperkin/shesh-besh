# Matchmaking

## Description

Players enter a FIFO queue per game mode and are matched with the next waiting player. Alternatively, a player can invite a friend to a private match. Both flows result in a match session starting with both players notified over WebSocket.

## Acceptance Criteria

1. **Given** an authenticated player with sufficient balance sends `join_queue { game_mode_id }` over WebSocket, **When** the server receives it, **Then** the player is added to the tail of the Redis queue for that game mode and the server acknowledges.

2. **Given** a player is already in a queue, **When** they send `join_queue` for the same game mode again, **Then** the server does not add them twice and returns an acknowledgement indicating they are already queued.

3. **Given** a player is in a queue, **When** they send `leave_queue { game_mode_id }`, **Then** the player is removed from that queue and the server acknowledges.

4. **Given** two players are in the same game mode queue, **When** the matchmaker dequeues them atomically, **Then** both players receive a `match_found` message containing `match_id`, `opponent`, `game_mode`, `board_state`, and `side_to_move`, and a `matches` row is created with status `in_progress`.

5. **Given** a match is found, **When** the match record is created, **Then** the escrow deduction for both players occurs atomically in the same transaction (see `specs/points-economy.md`).

6. **Given** one player disconnects while in queue, **When** the disconnect is detected, **Then** the player is removed from the queue automatically.

7. **Given** an authenticated player sends `invite_friend { friend_id, game_mode_id }`, **When** the server processes it, **Then** the invite is stored in Redis with a 60-second TTL and the friend receives a `friend_invite` WebSocket message containing `invite_id`, `from_player`, and `game_mode`.

8. **Given** a friend invite exists and has not expired, **When** the recipient sends `accept_invite { invite_id }`, **Then** a match is created between the two players, both receive `match_found`, and the invite is removed from Redis.

9. **Given** a friend invite exists, **When** the recipient sends `reject_invite { invite_id }`, **Then** the invite is removed from Redis and the sender receives no further notification for this invite.

10. **Given** a friend invite TTL of 60 seconds expires with no response, **When** the TTL elapses, **Then** the invite is automatically removed from Redis and no match is created.

11. **Given** a player attempts to invite someone who is not their accepted friend, **When** the server processes `invite_friend`, **Then** the server rejects the request with an error.

12. **Given** a player with insufficient balance for the game mode sends `join_queue` or `accept_invite`, **When** the server validates the action, **Then** it is rejected and the player is notified.

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| Two concurrent matchmakers dequeue the same player | Redis atomic LPOP (Lua script) prevents double-match; only one pair is dequeued |
| Invited friend is offline (no active WS connection) | Invite stored in Redis; delivered when friend reconnects within TTL |
| Invited friend is already in a match | Server rejects `accept_invite` with error "already in game" |
| Inviter disconnects before invite is accepted | Invite remains until TTL expires; match cannot start without both connections |
| Player in queue starts a private match | Player is removed from public queue before private match starts |
| `accept_invite` arrives after TTL expiry | Redis key gone; server returns error "invite expired" |

## Test Requirements

- AC1: Integration test — join_queue adds player to Redis list
- AC2: Integration test — duplicate join_queue is idempotent
- AC3: Integration test — leave_queue removes player from Redis list
- AC4: Integration test — two queued players receive match_found; match row created
- AC5: Integration test — escrow deducted atomically on match creation (see economy spec)
- AC6: Integration test — disconnecting player is removed from queue
- AC7: Integration test — invite_friend creates Redis key with TTL and delivers WS message
- AC8: Integration test — accept_invite creates match and delivers match_found to both
- AC9: Integration test — reject_invite removes Redis key
- AC10: Integration test — expired invite (TTL elapsed) produces no match
- AC11: Unit test — invite to non-friend rejected
- AC12: Integration test — insufficient balance blocks queue join and invite accept

## Dependencies

- `specs/auth.md` — WebSocket connection requires valid JWT
- `specs/game-modes.md` — stake enforcement checked before queue entry
- `specs/points-economy.md` — escrow deducted on match creation
- `specs/friends.md` — friend relationship required for private invite

## Out of Scope

- Best-of-N or tournament bracket matchmaking
- Skill-based or ELO matchmaking (current model: FIFO)
- Spectator join flow

## Open Questions

- None.
