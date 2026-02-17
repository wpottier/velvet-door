# Velvet Door — Game Framework (v0.1 / Vertical Slice)

Objectif : définir une structure Unreal propre (GameMode/GameState/PlayerState/Controller) adaptée au coop listen-server, pour éviter les erreurs classiques et préparer l’évolution du jeu.

---

## 1. Principe central UE à garder en tête

- GameMode : existe uniquement côté serveur (Authority). Logique et règles.
- GameState : répliqué à tous les clients. État global partagé.
- PlayerState : répliqué à tous. Données persistantes par joueur (nom, ready, stats…).
- PlayerController : existe côté serveur + client propriétaire. Input, UI, commandes.
- Pawn/Character : avatar contrôlé, répliqué.

Règle d’or :
- Tout ce qui doit être vu par tout le monde → GameState / PlayerState / Actors répliqués
- Tout ce qui est règles/autorité → GameMode
- Tout ce qui est UI/input local → PlayerController (et widgets)

---

## 2. Maps & modes (v0.1)

Maps :
- Map_Menu
- Map_Lobby
- Map_01_Speakeasy_Derelict

GameModes :
- GM_Menu
- GM_Lobby
- GM_Game

GameStates :
- GS_Lobby
- GS_Game

PlayerState :
- PS_VelvetPlayer (unique, utilisé partout)

PlayerController :
- PC_Velvet (unique)

Pawn/Character :
- CH_VelvetPlayer (FPS Character)

---

## 3. GM_Menu (Map_Menu)

### Rôle
- Gestion UI principale (Host/Join/Options)
- Login OSS / gestion de session (create/join)
- Pas de gameplay, pas de monde persistant

### Où mettre la logique ?
- UI et actions côté PC_Velvet
- GM_Menu peut rester très léger

### Données
- Aucune donnée de partie ici (pas de “settings de run” stockés dans le niveau)

---

## 4. GM_Lobby + GS_Lobby (Map_Lobby)

### Rôle (GM_Lobby)
- Autorité sur les règles de lobby
- Gestion des connexions joueurs
- Démarrage de partie (Start -> travel)

### Données partagées (GS_Lobby)
Stocker ici tout ce qui doit être visible par tous les joueurs dans le lobby :

- LobbyPhase (enum) : Setup, ReadyCheck, Starting
- Settings narratifs (répliqués) :
  - DebtAmountTier (enum/int)
  - PoliceMode (enum) : Active, Corrupt, Off (si tu veux)
  - ThievesEnabled (bool)
  - GroupBond (enum) : Siblings, Partners, Opportunists, etc.
- Liste joueurs (via PlayerStates)
- Countdown de lancement (optionnel)

### “Ready” system (PS_VelvetPlayer)
Chaque joueur a :
- bIsReady (bool répliqué)
- DisplayName (string)
- PreferredInput (optionnel plus tard)

### Flux lobby recommandé
1. Joueur rejoint lobby
2. Il voit la config courante
3. L’host modifie settings → GS_Lobby réplique aux clients
4. Chaque joueur toggle Ready
5. Host clique Start (si conditions OK) → ServerTravel vers map de jeu

Conditions minimales v0.1 (proposées) :
- Au moins 1 joueur
- Tous Ready (optionnel mais recommandé)
- Host seul peut lancer Start

---

## 5. GM_Game + GS_Game (Map_01_Speakeasy_Derelict)

### Rôle (GM_Game)
- Autorité sur la journée
- Spawn/possess joueurs
- Validation interactions (si nécessaire)
- Gestion PNJ (spawn/AI) côté serveur
- Déclenchement chaos events

### Données partagées (GS_Game)
Stocker ici l’état global visible par tous :

- DayPhase (enum) : Preparation, Service, WrapUp
- TimeRemaining (float/int)
- MoneyEarnedToday (int)
- DebtRemaining (int)
- Reputation (int ou float placeholder)
- Flags de config appliqués (pour debug) :
  - PoliceMode
  - ThievesEnabled
- Scoreboard simplifié (optionnel)

### Données par joueur (PS_VelvetPlayer)
- PlayerRolePreference (optionnel) : bartender/service/security (plus tard)
- Stats de soirée (tips servis, erreurs, etc. placeholder)

---

## 6. PC_Velvet (PlayerController)

### Rôle
- Input local (Enhanced Input)
- UI :
  - Menu principal
  - Lobby UI
  - HUD minimal en jeu
  - Debug overlay toggle
- Envoi de commandes au serveur :
  - Ready toggle
  - Demande Start (host)
  - Intentions d’interaction (Grab/Drop/Use)

### Règle d’implémentation
- L’UI lit l’état depuis GS_Lobby/GS_Game et PS_VelvetPlayer
- L’UI ne “décide” pas : elle demande via RPC si besoin

---

## 7. PS_VelvetPlayer (PlayerState unique)

### Rôle
Données joueur persistantes dans une session multi.

Champs v0.1 recommandés :
- DisplayName (string)
- bIsReady (bool)
- bIsHost (bool optionnel, sinon comparer ControllerId/Owner)
- InputLayout (enum) : QWERTY, AZERTY (optionnel)
- ConnectionQuality (optionnel, debug)

---

## 8. CH_VelvetPlayer (Character)

### Rôle
- Mouvement FPS
- Réplication mouvement
- “Main slot” (objet tenu) répliqué
- Interaction trace (client) + validation serveur

Champs v0.1 recommandés :
- HeldItem (Actor ref répliqué)
- InteractionComponent (référence)
- Camera + arms (placeholder)

---

## 9. Où stocker les “Settings” de partie ?

Réponse v0.1 (simple, robuste) :
- Lobby settings dans GS_Lobby
- Au moment du travel, GM_Game copie vers GS_Game (ou vers un objet “RunConfig” répliqué)

Option alternative (plus tard) :
- GameInstance pour persister entre maps (non répliqué)
- Mais attention : GameInstance n’est pas partagé entre joueurs, donc pas source de vérité multi.

Règle :
- Source de vérité multi = GS_Lobby / GS_Game (répliqués)

---

## 10. Minimum UI à prévoir pour valider l’architecture

Map_Menu :
- Host
- Join (via présence)
- Options input (layout)

Map_Lobby :
- Liste joueurs + Ready
- Réglages narratifs (Debt/Police/Thieves/GroupBond)
- Start (host)

Map_Game :
- Phase + timer
- Objectif simple (argent/dette)
- Event notifications (chaos)

---

## 11. Définition of Done (framework)

- Chaque map a son GameMode correct
- Lobby settings se répliquent à tous
- Ready se réplique par joueur
- Start lance ServerTravel et tout le monde suit
- DayPhase se réplique en jeu
- UI lit GameState/PlayerState sans hacks
