# PRD: Shesh-Besh — Real-Time Multiplayer Backgammon Server

## Problem Statement

Backgammon players have no modern, cross-platform app that combines real-time competitive play, a social layer (friends, ranks), and a points-based stakes economy. Existing apps are either platform-locked, lack real-time feel, or have no social/competitive hooks that keep players engaged. Shesh-Besh solves this with a fast C++ game server delivering real-time WebSocket gameplay, a virtual points economy, and a global leaderboard — playable across Android, iOS, and Web with full cross-play.

---

## Jobs-to-be-Done

| Segment | Job Statement |
|---|---|
| Casual player | When I have free time, I want to jump into a quick backgammon match at low stakes, so I can enjoy the game without pressure. |
| Competitive player | When I want to test my skill, I want to play high-stakes matches and see my rank vs. the global leaderboard and my friends, so I can feel rewarded for improving. |

---

## Audience Segments

| Segment | Context | Primary JTBD | Key Outcome Metric | Current Workaround |
|---|---|---|---|---|
| Casual player | Plays a few games per session, low stakes, just wants fun | Play a quick real-time backgammon match | Games completed per session | Random opponent apps, local play |
| Competitive player | Plays daily, tracks rank, plays high-stakes tables | Win points, climb leaderboard, beat friends | Leaderboard rank, point balance | No good cross-platform option |

---

## Story Map

### 1. Authentication
- **Sign in**
  - As a new user, I want to sign in with Google or Apple so I don't need a username/password.
  - As a returning user, I want my session to persist so I don't sign in every time.
- **Account creation**
  - As a new user, I want to receive 1,000 starting points automatically on first sign-in.

### 2. Daily Bonus
- **Claim daily points**
  - As a player, I want to claim 200 free points once per day so I can keep playing if my balance is low.

### 3. Lobby & Game Mode Selection
- **Browse game modes**
  - As a player, I want to see available game modes with their stake and timer settings so I can pick one I can afford and want to play.
- **Stake enforcement**
  - As a player, I want to be blocked from entering a game mode I can't afford so I never go into negative points.

### 4. Matchmaking
- **Join queue**
  - As a player, I want to enter a queue for my chosen game mode so I'm matched with another player at the same stake level.
- **Private match**
  - As a player, I want to invite a friend to a private match at a chosen game mode so we can play head-to-head.
- **Match found**
  - As a player, I want to be notified when a match is found and the game starts automatically.

### 5. Gameplay
- **Real-time moves**
  - As a player, I want to see my opponent's move appear on my board immediately after they play it.
- **Turn timer**
  - As a player, I want to see a countdown timer for the current player's turn so I know how long they have.
- **Grace timer**
  - As a player, I want to know my opponent has forfeited if their grace timer runs out, so the game ends fairly.
- **Legal move enforcement**
  - As a player, I want illegal moves rejected by the server so neither player can cheat.
- **Game over**
  - As a player, I want to see the result (win/loss) and my updated point balance immediately after the game ends.

### 6. Points Economy
- **Winner takes stake**
  - As the winner, I want the full staked points transferred to my balance automatically.
- **Loser deducted**
  - As the loser, I accept that my staked points are deducted at game start (held in escrow) and forfeited on loss.

### 7. Leaderboard
- **Global leaderboard**
  - As a player, I want to see the top players ranked by total points so I know where I stand globally.
- **My rank**
  - As a player, I want to see my own rank highlighted on the leaderboard even if I'm not in the top N.

### 8. Friends
- **Add friend**
  - As a player, I want to add another player as a friend so I can track their rank and challenge them.
- **Friends on leaderboard**
  - As a player, I want to filter the leaderboard to see only my friends' ranks so I can compete with people I know.
- **Invite to match**
  - As a player, I want to send a match invite to a friend so we can play a private game at a stake we agree on.

---

## V1 Scope (SLC Slice)

**In scope for MVP (server only):**

| # | Story |
|---|---|
| 1 | Sign in with Google / Apple OAuth2 → JWT |
| 2 | Auto-grant 1,000 points on first sign-in |
| 3 | Daily 200-point bonus (once per 24h) |
| 4 | Browse game modes (vanilla backgammon, multiple stake tiers) |
| 5 | Stake enforcement — can't join game you can't afford |
| 6 | Global matchmaking queue per game mode |
| 7 | Private match invite to friend |
| 8 | Real-time WebSocket gameplay — move push to both players |
| 9 | Turn timer + grace timer (configurable per game mode) |
| 10 | Server-side legal move validation |
| 11 | Winner-takes-all point transfer on game end |
| 12 | Global leaderboard (points-ranked) |
| 13 | Friends list + friends' leaderboard rank |
| 14 | Forfeit on grace timer expiry |

**SLC rationale:** These 14 stories form the minimum end-to-end loop: sign in → pick a game → get matched → play in real time → see result → check rank. Cutting any of them breaks the core job. The points economy (stakes + leaderboard) is what differentiates this from a toy — it must be in V1.

### Out of Scope for V1

- Android / iOS / Web clients (server only)
- Best-of-N / tournament match format
- Referral points
- Ad-based point earning
- Spectator mode
- Game replay
- Push notifications (mobile)
- In-match chat
- Additional rules engines (Narde, Acey-Deucey, etc.)
- Apple / Google IAP (no real money)

---

## Game Mode Config Schema

Each game mode is a named configuration combining rules engine, stake, and timer settings:

| Field | Description | Example |
|---|---|---|
| `id` | Unique identifier | `vanilla_100` |
| `name` | Display name | `"Classic – 100pts"` |
| `rules_engine` | Rules implementation | `"vanilla"` |
| `stake` | Points wagered per player | `100` |
| `turn_timer_seconds` | Seconds per turn | `30` |
| `grace_timer_seconds` | Total grace budget per game | `60` |

New game modes (different rules, stakes, or timers) are added by config — no code change required.

---

## Timer Behavior

- **Turn timer**: resets to `turn_timer_seconds` at the start of each turn. Client renders countdown locally from server-sent start signal.
- **Grace timer**: a global budget per player per game. Starts ticking only when turn timer expires. Deducted each time a player overruns their turn timer. When exhausted → auto-forfeit. Persists across turns (e.g. used 11s of 60s grace on turn 1 → 49s remaining for rest of game).
- Both timers configurable per game mode. Both values sent to clients at match start.

---

## Points Economy Rules

- New player starts with **1,000 points**.
- **Daily bonus**: 200 points, claimable once per 24h.
- **Stake escrow**: points deducted from both players at match start, held until game ends.
- **Winner takes all**: full escrow transferred to winner.
- **Floor**: balance cannot go below 0. Players blocked from entering any game mode with `stake > balance`.
- No rake. No house cut.

---

## Parking Lot

- Referral points program (v1.1)
- Ad-based point earning (v1.1)
- Best-of-N / tournament bracket (v2)
- Spectator mode (v2)
- Game replay (v2)
- Push notifications (v2)
- In-match chat (v2)
- Narde, Acey-Deucey, Hypergammon rules engines (v2+)
- Real-money IAP / premium currency (explicitly out — not in product vision)
- Graceful shutdown / drain pattern (SIGTERM → stop new matches → wait for active games → exit) — needed before high-traffic deploys

---

## Resolved Decisions

| # | Question | Decision |
|---|---|---|
| 1 | Leaderboard ranking basis | All-time, no reset. Rank = current point balance. A player ranked #1 in 2026 keeps that rank as long as their balance stays highest. |
| 2 | Double-disconnect | Detect cause: if connectivity loss (e.g. both drop within same short window / network error signal) → refund both. If both players intentionally quit → no refund, points forfeited. Best-effort detection via WebSocket close code + grace window. |
| 3 | Friend system | Bidirectional mutual accept (send request → other player accepts). |
| 4 | Grace timer scope | Per-player. Each player has their own independent grace budget. |
| 5 | Username | Auto-generated from OAuth profile name on first login. |
