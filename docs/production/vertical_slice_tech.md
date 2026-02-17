# ⚙️ Velvet Door — Vertical Slice (Tech)

## 1. Objectif technique du vertical slice

Le vertical slice doit démontrer :

- Une architecture réseau saine
- Une expérience coop fluide et sans friction
- Une base d’input solide et extensible
- Une structure prête à évoluer sans refactor massif

---

## 2. Architecture réseau

### Modèle

- Listen Server (host = autorité)
- 2 à 4 joueurs
- État du monde autoritaire côté serveur

### Autorité serveur

Le serveur gère :

- État de la journée (Préparation / Soirée / Bilan)
- PNJ
- Chaos events
- Argent / dette
- Ressources

Les clients envoient des intentions d’action.

---

## 3. Online & Sessions

### Objectif

Permettre :

- Création de partie simple
- Rejoindre via amis
- Invites via overlay plateforme
- Présence "joinable"

### Principes

- Utilisation d’un système de lobby/session compatible multi-plateforme
- Partie joinable via présence
- Pas de code à partager
- Pas de master list publique pour le vertical slice

---

## 4. Input System

### Technologie

- Enhanced Input (Unreal)

### Exigences

- Actions abstraites (Interact, Grab, Drop, Use, etc.)
- Support QWERTY par défaut
- Support AZERTY
- Rebinding simple

### Manette

- Mapping fonctionnel de base
- Navigation UI compatible stick
- Pas de polish avancé requis au VS
- Conception compatible future optimisation manette

---

## 5. Système d’interaction réseau

Minimum requis :

- Prendre / poser objet répliqué
- État objet synchronisé
- Feedback visuel cohérent multi-client
- Pas de désynchronisation critique

---

## 6. State Manager global

Implémenter un gestionnaire d’état :

- Menu
- Lobby
- Préparation
- Soirée
- Bilan

Transitions propres et synchronisées.

---

## 7. PNJ minimal serveur-side

Vertical slice :

- PNJ clients gérés côté serveur
- Logique simple (file d’attente, impatience)
- Réplication d’état visible

---

## 8. Définition of Done (Tech)

Le vertical slice technique est validé si :

- Une partie peut être créée et rejointe sans friction
- Les joueurs voient les actions des autres en temps réel
- Aucun crash réseau majeur
- Pas de désynchronisation critique
- Input configurable fonctionnel
- Build packagée fonctionnelle (pas uniquement PIE)

---

## 9. Hors scope technique VS

- Migration d’host
- Serveur dédié
- Crossplay avancé optimisé
- Anti-cheat
- Économie persistante
- Backend dédié