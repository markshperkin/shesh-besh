# Points Economy

## Description

A virtual points economy governs match stakes. Points are escrowed at match start, transferred to the winner on game end, and refunded on double-disconnect. All balance mutations are atomic and audited. Balances cannot go negative.

## Acceptance Criteria

1. **Given** a match is created, **When** the match record is written, **Then** both players' balances are each debited by the game mode's `stake` in a single atomic Postgres transaction, and a `point_transactions` row is inserted for each player with `reason=match_escrow` and `delta=-stake`.

2. **Given** a player's `points_balance` minus the stake would result in a negative balance, **When** the escrow transaction is attempted, **Then** the transaction is rolled back, the match is not created, and the player is notified.

3. **Given** a match ends with a winner (normal completion or grace timer forfeit), **When** `game_over` is processed, **Then** the winner's balance is credited with `stake * 2` in a single atomic transaction, and a `point_transactions` row is inserted with `reason=match_win` and `delta=stake*2`.

4. **Given** a match ends with a winner due to opponent disconnection, **When** the disconnect is detected, **Then** the remaining connected player is immediately declared the winner and AC3 applies.

5. **Given** both players disconnect simultaneously (within server detection window), **When** this is detected, **Then** both players are each refunded their stake in a single atomic transaction, `point_transactions` rows inserted with `reason=match_refund`, and the match is marked `forfeited` with `forfeit_reason=double_disconnect`.

6. **Given** a point transaction of any type is executed, **When** the transaction commits, **Then** the `users.points_balance` is updated AND a `point_transactions` row is inserted in the same atomic Postgres transaction — they never diverge.

7. **Given** a player's `points_balance` is 0, **When** any operation attempts to debit it, **Then** the Postgres `CHECK >= 0` constraint prevents the debit and an error is returned.

8. **Given** a match escrow transaction fails partway through (e.g. one player debited, second fails), **When** the failure occurs, **Then** the entire transaction is rolled back and neither player's balance is changed.

9. **Given** a concurrent attempt to debit the same player twice simultaneously (e.g. joining two queues simultaneously), **When** both transactions run, **Then** only the transaction that keeps the balance >= 0 succeeds; the other is rolled back.

## Edge Cases

| Scenario | Expected Behavior |
|---|---|
| Winner's balance credit fails after loser already debited | Full transaction rollback — both balances revert |
| Match ends but both players are disconnected simultaneously | Double-disconnect refund rule applies |
| Stake is larger than both players' current balance (race: balance changed after stake check) | DB constraint prevents negative; transaction rolls back |
| Player wins but already has max BIGINT balance | Arithmetic overflow protection — not expected at MVP scale but constraint should be noted |
| Refund attempted for already-refunded match (duplicate event) | Idempotency: check match status before executing refund; skip if already refunded |

## Test Requirements

- AC1: Integration test — match creation debits both players and inserts two transaction rows atomically
- AC2: Integration test — insufficient balance prevents match creation and rolls back
- AC3: Integration test — game_over credits winner with 2x stake and inserts transaction row
- AC4: Integration test — single disconnect triggers immediate win for remaining player
- AC5: Integration test — double disconnect refunds both players and marks match forfeited
- AC6: Integration test — balance and transaction row always consistent (never one without the other)
- AC7: Integration test — balance at 0 cannot be debited; DB constraint fires
- AC8: Integration test — partial escrow failure rolls back fully (simulate second debit failure)
- AC9: Concurrency test — simultaneous double-join attempts for same player produce at most one successful escrow

## Dependencies

- `specs/auth.md` — all operations are player-scoped by JWT identity
- `specs/game-modes.md` — stake value sourced from match's copied game mode config
- `specs/matchmaking.md` — escrow triggered on match creation
- `specs/gameplay.md` — winner transfer triggered on game_over

## Out of Scope

- Rake or house cut
- Real-money transactions
- Point gifting between players
- Negative balance tolerance / debt

## Open Questions

- None.
