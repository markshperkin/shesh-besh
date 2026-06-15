# Friends

## Description

Players can add each other as friends via a mutual-accept model. Friends can view each other's leaderboard rank and send private match invites. Friend requests are delivered over WebSocket if the recipient is connected.

## Acceptance Criteria

1. **Given** an authenticated player sends `POST /friends/request { target_id }`, **When** the server processes it, **Then** a `friendships` row is inserted with `status=pending`, `requester_id` set to the calling player, and `addressee_id` set to `target_id`.

2. **Given** the target player is connected via WebSocket at the time of the request, **When** the friendship request is created, **Then** the target receives a `friend_request { from_player }` WebSocket message.

3. **Given** an authenticated player sends `POST /friends/accept { requester_id }`, **When** a pending friendship from `requester_id` to the calling player exists, **Then** the `friendships` row is updated to `status=accepted`.

4. **Given** a player attempts to accept a request that does not exist or was not directed at them, **When** the server processes the accept, **Then** HTTP 404 is returned and no row is modified.

5. **Given** an authenticated player calls `GET /friends`, **When** the server responds, **Then** it returns all friendships where either `requester_id` or `addressee_id` matches the calling player and `status=accepted`, with fields: `friend_id`, `display_name`, `points_balance`.

6. **Given** an authenticated player calls `DELETE /friends/:id`, **When** the server processes it, **Then** the friendship row is deleted regardless of which side initiated the original request.

7. **Given** a player attempts to send a friend request to themselves, **When** the server processes it, **Then** HTTP 400 is returned.

8. **Given** a player attempts to send a friend request to someone they already have an accepted friendship with, **When** the server processes it, **Then** HTTP 409 is returned and no new row is inserted.

9. **Given** a player attempts to send a duplicate pending friend request to the same target, **When** the server processes it, **Then** HTTP 409 is returned and no duplicate row is inserted.

10. **Given** two players have an accepted friendship, **When** one player sends `invite_friend` over WebSocket, **Then** the invite is processed (see `specs/matchmaking.md`). This spec covers the relationship prerequisite only.

11. **Given** an unauthenticated request is made to any friends endpoint, **When** the server processes it, **Then** HTTP 401 is returned.

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| Player A requests Player B; Player B also requests Player A before accepting | Second request treated as a duplicate (409) — only one pending row should exist |
| Target player does not exist | HTTP 404 returned |
| Player deletes a friend who has pending friend requests from others | Only the specific friendship between the two players is deleted |
| Player accepts a request after requester deleted their account | HTTP 404 — requester user not found |
| Offline target receives friend request | Request stored; `friend_request` WS message delivered on next connection if within session validity |

## Test Requirements

- AC1: Integration test — POST /friends/request inserts pending row
- AC2: Integration test — online target receives WS friend_request message
- AC3: Integration test — POST /friends/accept updates row to accepted
- AC4: Integration test — accept non-existent or misdirected request returns 404
- AC5: Integration test — GET /friends returns all accepted friends with correct fields
- AC6: Integration test — DELETE /friends/:id removes row from both directions
- AC7: Unit test — self-request returns 400
- AC8: Integration test — duplicate accepted-friend request returns 409
- AC9: Integration test — duplicate pending request returns 409
- AC10: Cross-spec — covered by matchmaking spec
- AC11: Unit test — unauthenticated requests return 401

## Dependencies

- `specs/auth.md` — all endpoints require JWT
- `specs/matchmaking.md` — accepted friendship required for private match invite

## Out of Scope

- Friend search by username or display name (v2)
- Blocking players
- Friend activity feed

## Open Questions

- None.
