# EOS + Redpoint — Acceptance Tests (Vertical Slice)

Status: v0.1  
Goal: Prove seamless coop flow (Steam + Epic via EOS) early.

---

## A. Login / Startup

A1. Auto-login
- Launch game → user becomes Online without manual steps (when possible).

A2. No Epic account forced
- Steam user can become Online without creating/linking an Epic account (Connect-first).

A3. Error handling
- If offline / EOS unreachable: clear UI message + retry path.

---

## B. Friends list (in-game)

B1. Friends list loads
- In-game UI shows friends list (even if minimal).
- Shows Online/Offline and In-Game status.

B2. Joinable presence
- When a friend is in Lobby/Game and has free slots:
  - UI indicates Joinable
  - Join button is enabled

---

## C. Invites

C1. Invite friend
- From in-game friends UI: send invite to friend.

C2. Accept invite
- Friend accepts invite → transitions into Lobby (or directly Game if designed).

C3. Multi-platform
- Repeat C1–C2 with Steam host + Epic friend (and inverse) as soon as possible.

---

## D. Lobby to Game (host authority)

D1. Host creates lobby
- Host creates a party/lobby that others can join.

D2. Start game
- Host clicks Start → all clients follow via travel into Map_01.

D3. Non-host rejoin (host alive)
- Client disconnects/crashes
- Client can rejoin via Join Friend / Party while host is still alive.

---

## E. Optional “Join by code” (stretch)

E1. Host sees code
- Host can display/share a short code.

E2. Viewer joins
- Another player enters code and joins without being friends.

---

## Done criteria for VS
- A1 + A2 + B1 + B2 + C1 + C2 + D2 are mandatory.
- D3 strongly recommended.
- C3 and Join-by-code are stretch goals.