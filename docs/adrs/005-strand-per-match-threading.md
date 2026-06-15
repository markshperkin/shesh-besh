# ADR-005: Strand-per-Match Threading Model

## Status
Accepted

## Context
Drogon runs multiple I/O threads. Two players in the same match can send WebSocket messages simultaneously, causing concurrent writes to `GameState`. We need match-level isolation without the cost of one OS thread per game.

## Decision
Each `MatchSession` owns a **`trantor` strand** (a serialization context on top of Drogon's shared event loop thread pool). All operations on a session (apply move, timer tick, disconnect handling) are posted to that session's strand via `loop->runInLoop(...)`.

## Why Not One Thread Per Match
- OS threads cost ~1MB stack each by default
- 10,000 concurrent games = 10GB of stack space allocated
- Context switching overhead grows with thread count
- Strands give the same serial-execution guarantee at a fraction of the cost — the underlying threads are shared across all strands

## Guarantees Provided
- No two operations on the same `MatchSession` run concurrently
- `GameState` never races — only the strand executes against it
- `MatchMaker` queue protected by `std::mutex` (shared across all strands)
- `PlayerRegistry` protected by `std::shared_mutex` (frequent reads, rare writes)

## Consequences
- All `MatchSession` public methods must post work to the strand, not execute directly
- `MatchSession` becomes slightly harder to unit-test (need to drain the event loop in tests)
- Blocking calls (DB queries) must use Drogon's async DB API — never block the strand
- `multiGameConcurrent` test must pass under ThreadSanitizer (`-fsanitize=thread`)
