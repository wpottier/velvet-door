# EOS Online Flow — Velvet Door (Steam + Epic via EOS + Redpoint)

Status: Draft v0.1 (Vertical Slice)  
Goal: Seamless coop 2–4, host-authority, invites/presence cross-platform

---

## 0. Pourquoi EOS comme “couche unique”

Objectif produit : l’utilisateur doit pouvoir inviter/rejoindre des amis sans friction, que l’ami soit sur Steam ou Epic.

Approche retenue :
- Utiliser Epic Online Services (EOS) comme couche unifiée (sessions, invites, presence, friends)
- Utiliser Redpoint EOS Online Framework pour accélérer l’intégration (login auto, parties auto, binding UMG, etc.).  [oai_citation:0‡docs.redpoint.games](https://docs.redpoint.games/?utm_source=chatgpt.com)

---

## 1. Concepts (glossaire minimal)

- Local Platform Account: Steam ou Epic (selon le launcher)
- EOS Connect: permet de “log-in” via un provider externe (Steam, etc.) sans nécessairement passer par un compte Epic obligatoire.  [oai_citation:1‡Epic Games Developers](https://dev.epicgames.com/docs/dev-portal/identity-provider-management?utm_source=chatgpt.com)
- Cross-platform account provider: stratégie choisie pour gérer (ou non) un compte cross-platform (ex: Epic account, ou “None” selon config).  [oai_citation:2‡docs.redpoint.games](https://docs.redpoint.games/docs/auth/configuration/?utm_source=chatgpt.com)
- Party: groupe persistant de joueurs (avant/pendant/après la partie)
- Session/Lobby/Game: notions UE/OSS pour rassembler les joueurs et faire le travel

Redpoint apporte des composants “haut niveau” (login auto, parties auto, lobby sync, nodes Blueprint) qui remplacent beaucoup de plumbing maison.  [oai_citation:3‡Epic Developer Community Forums](https://forums.unrealengine.com/t/redpoint-games-eos-online-framework/2595124?utm_source=chatgpt.com)

---

## 2. Objectifs UX (non négociables)

- Un joueur clique “Inviter” depuis le lobby → l’ami rejoint en 1 action (Steam ou Epic)
- Un joueur non-host peut voir les parties actives de ses contacts et rejoindre sans code
- Reconnexion en cours de partie possible (au minimum: rejoin session si host toujours en jeu)
- Pas de master server list au VS (mais ne pas s’interdire d’en faire une plus tard)

---

## 3. Flux global (Happy Path)

### 3.1 Startup / Login
1) Le jeu affiche un Startup Screen
2) Le framework connecte l’utilisateur à EOS avant que ton gameplay démarre
3) L’utilisateur est “online” (presence/friends utilisables)

Redpoint fournit un mécanisme de Startup Screen + login automatique.  [oai_citation:4‡docs.redpoint.games](https://docs.redpoint.games/docs/framework/automatic_login/?utm_source=chatgpt.com)

Décision “Velvet Door v0.1” :
- On vise un login sans friction (auto quand possible)
- On évite toute UI de compte Epic si on peut (sauf cas où c’est requis par la config choisie)

### 3.2 Party (groupe)
Dès login EOS :
- Une party est créée automatiquement (ou rétablie) pour l’utilisateur
- Les membres de party seront synchronisés vers l’UI lobby

Redpoint propose l’auto-party et la synchronisation “party members” vers UI.  [oai_citation:5‡docs.redpoint.games](https://docs.redpoint.games/docs/framework/automatic_parties?utm_source=chatgpt.com)

### 3.3 Lobby (pré-game)
- L’host crée une “party/lobby session”
- Les invités rejoignent via invitation (platform overlay ou UI in-game)
- Tous arrivent sur `Map_Lobby`
- Le lobby UI montre les membres (Ready, settings, etc.)

### 3.4 Start game
- L’host lance la partie
- ServerTravel vers `Map_01_Speakeasy_Derelict?listen`
- Tous les clients suivent

### 3.5 Rejoin / Reconnect (VS-friendly)
- Si un joueur drop, il peut rejoindre à nouveau via:
  - présence / party / “join friend”
- Tant que l’host (listen-server) est vivant, on autorise le rejoin
- Si l’host quitte, la partie est perdue (assumé pour v0.1)

---

## 4. Flux d’invitation et “join friend”

### 4.1 Inviter (host ou membre party)
- Source de vérité: party EOS
- Un joueur invite depuis:
  - overlay Steam/Epic
  - ou UI in-game (plus tard)

But :
- Invites cross-platform (Steam ↔ Epic) dès que possible via EOS.  [oai_citation:6‡docs.redpoint.games](https://docs.redpoint.games/?utm_source=chatgpt.com)

### 4.2 Rejoindre une partie d’un ami
- Le joueur voit ses amis (EOS unified friends / presence)
- Pour un ami “in game / joinable”, on propose “Join”
- Le join exécute:
  - Join session/lobby
  - Travel vers Map_Lobby ou Map_Game selon l’état

Note: Redpoint met en avant la gestion d’avatars cross-platform et l’unification friends/presence/invites.  [oai_citation:7‡docs.redpoint.games](https://docs.redpoint.games/?utm_source=chatgpt.com)

---

## 5. Stratégie “Steam via EOS” (sans OSS Steam direct)

### But
Avoir un seul système pour:
- identity
- invites
- sessions
- presence

### Principes
- Steam sert de provider d’identité (EOS Connect)
- EOS devient le centre de gravité

EOS Connect supporte les providers externes comme Steam.  [oai_citation:8‡Epic Games Developers](https://dev.epicgames.com/docs/dev-portal/identity-provider-management?utm_source=chatgpt.com)

Côté Redpoint, la doc “platform setup for Steam” et la config auth guident ce montage.  [oai_citation:9‡docs.redpoint.games](https://docs.redpoint.games/docs/setup/platforms/steam/?utm_source=chatgpt.com)

---

## 6. Architecture UE recommandée (qui parle avec EOS)

### 6.1 Où vit la logique “online”
- `UGameInstance` (ou un Subsystem) = orchestration online
  - init/login state
  - create/join
  - events (invite received, join accepted)

### 6.2 Données partagées
- `GS_Lobby` / `GS_Game` : état répliqué de gameplay (pas l’auth)
- `PS_VelvetPlayer` : ready, display name, etc.

### 6.3 UI
- Menu:
  - état login (Connecting / Online / Error)
  - Host / Join friend
- Lobby:
  - party members list (source: EOS party)
  - ready + settings

Redpoint propose des composants pour “bind friends list / party members” à des widgets UMG (tree view), et maintenir les entrées à jour.  [oai_citation:10‡Epic Developer Community Forums](https://forums.unrealengine.com/t/redpoint-games-eos-online-framework/2595124?utm_source=chatgpt.com)

---

## 7. Décisions “Vertical Slice” (scope control)

### Ce qu’on fait au VS
- Login automatique (ou 1 clic)
- Party auto (ou création simple)
- Inviter via overlay (Steam/Epic) ou UI minimal si facile
- Join friend (présence) si accessible rapidement
- Listen-server host gardien
- Rejoin tant que host vivant

### Ce qu’on ne fait PAS au VS
- Matchmaking public / team matchmaking (paid feature)  [oai_citation:11‡Fab.com](https://www.fab.com/ru/listings/6a9b045b-e29e-4817-a7f1-23dccf4770ec?utm_source=chatgpt.com)
- Master list public
- Dedicated servers
- Migration host
- Consoles

---

## 8. Risques & mitigations

### Risque A: complexité de config EOS (Dev Portal, providers)
Mitigation:
- viser un “happy path” (Steam + Epic, login minimal)
- documenter la config dans un fichier séparé (eos_devportal_setup.md)

EOS Connect / identity providers se configurent côté Dev Portal.  [oai_citation:12‡Epic Games Developers](https://dev.epicgames.com/docs/dev-portal/identity-provider-management?utm_source=chatgpt.com)

### Risque B: dépendance plugin (Redpoint)
Mitigation:
- isoler l’usage derrière un “OnlineSubsystem/Service layer” interne (thin wrapper)
- éviter de mélanger code gameplay et code EOS

---

## 9. Decisions locked (Velvet Door v0.1)

### D1 — No Epic account required (Steam-friendly)
- Goal: Steam players must not be forced to create/link an Epic account.
- Approach: Prefer EOS Connect (external identity providers) and avoid mandatory EAS flows.
- Epic account remains optional for players who already have it and want cross-network benefits.

### D2 — In-game minimal UI for Friends/Invites/Join
- Goal: Stay in-game; avoid “alt-tab overlay friction” as primary UX.
- Provide minimal Friends list UI:
  - show friends who own the game (when possible)
  - show presence / joinable status
  - show available slots
  - Invite / Join actions
- Platform overlay remains as fallback.

### D3 — Join Friend ASAP (Vertical Slice validation)
- Goal: Validate Redpoint EOS framework value early:
  - auto-login
  - presence joinable
  - in-game invite/join
  - rejoin while host is alive

### D4 — Optional Join by Code (streamer-friendly)
- Not mandatory, but supported as an alternate path:
  - Host can display a short “room code”
  - Others can enter the code to join (without being friends)
- This does not replace invites; it complements them.