# 🎷 Velvet Door — Game Design Document (v0.1)

---

## 1. Présentation générale

### Titre
**Velvet Door**

### Genre
Simulation coopérative sociale immersive  
Gestion de crise et d’organisation

### Plateforme cible
PC (Steam en priorité)

### Mode de jeu
Multijoueur coopératif — 2 à 4 joueurs

### Vue
3D — Première personne (FPS)

---

## 2. Vision synthétique

Velvet Door est un jeu coopératif où les joueurs dirigent un speakeasy pendant la prohibition.  
Ils préparent leurs soirées le jour et affrontent le chaos social la nuit, en jonglant entre élégance de façade, criminalité clandestine, corruption et improvisation collective.

Le cœur du jeu repose sur :
- l’organisation sociale,
- la préparation intelligente,
- la coopération sous pression,
- et des systèmes générant du chaos émergent teinté d’humour noir.

---

## 3. Boucle de gameplay principale

### Boucle macro

Velvet Door est structuré autour d’une loop macro appelée “journée”, conçue pour durer environ 45 minutes de jeu réel.

Une journée se compose de trois temps distincts :

Phase de préparation (10 à 20 minutes) :
personnalisation du club, préparation des ressources, choix stratégiques, gestion des PNJ et des relations.

Phase de soirée (environ 20 minutes) :
exécution, organisation sous pression et gestion du chaos vivant.

Phase de bilan (5 à 10 minutes) :
conséquences, progression, déblocages et nouvelles opportunités.

Une session de jeu peut contenir une ou plusieurs journées, selon le rythme et la disponibilité du groupe.

---

## 4. Structure temporelle

### 🌤️ Phase Journée (10 à 20 minutes) — Préparation & narration

Objectifs :
- gérer les stocks et ressources
- préparer des éléments périssables
- recruter / configurer les employés PNJ
- négocier avec la mafia
- gérer la pression policière (pots-de-vin, couverture)
- signer des contrats (événements, artistes, VIP)

La journée est un temps de **choix stratégiques et de paris**.  
Les conséquences de ces choix se manifestent toujours pendant la soirée.

---

### 🌙 Phase Soirée (20 minutes) — Exécution & chaos vivant

Objectifs :
- accueillir et servir les clients
- maintenir l’ambiance et l’illusion de contrôle
- gérer les imprévus et incidents
- improviser collectivement
- protéger réputation et relations

Le chaos est :
- progressif
- lisible
- systémique
- jamais purement punitif

---

## 5. Coopération & rôles

Velvet Door ne repose pas sur des rôles fixes.

Les joueurs se répartissent **des responsabilités**, librement et dynamiquement :
- bar & cocktails
- service clients
- sécurité & entrée
- ambiance & musique
- relations (mafia, police, VIP)
- casino clandestin (late game)

Si un système n’est pas pris en charge, il se dégrade.

---

## 6. Interactions gameplay

### Philosophie générale

Le gameplay est basé sur des **interactions physiques diégétiques** en vue FPS.

Exemples :
- préparer un cocktail étape par étape
- manipuler des objets (bouteilles, verres, plateaux)
- découper, verser, mélanger, transporter
- gérer argent, documents, marchandises

Règle de design :
> Une tâche doit impliquer 3 à 5 actions maximum pour rester satisfaisante.

---

## 7. Préparation stratégique & ressources périssables

Certaines tâches peuvent être effectuées **avant l’ouverture** :
- ingrédients préparés
- organisation du bar
- configuration sécurité
- préparation d’événements

Caractéristiques :
- la préparation réduit la pression du rush
- elle consomme temps et ressources
- les éléments non utilisés sont perdus en fin de soirée

Objectif :
Encourager anticipation, prise de risque mesurée et discussion coop.

---

## 8. PNJ — philosophie générale

Les PNJ sont un pilier du jeu, à la fois gameplay et narratif.

Ils existent sous deux grandes catégories :
- **Employés** (barmen, serveurs, vigiles, musiciens, croupiers…)
- **Clients** (VIP, mafieux, notables, artistes, joueurs…)

Chaque PNJ possède :
- des compétences
- des défauts
- une personnalité lisible
- un potentiel de chaos

Les PNJ n’éliminent jamais le gameplay joueur :  
ils **amplifient les choix** et introduisent des risques.

---

## 9. Configuration des employés & difficulté émergente

Avant une session, les joueurs peuvent :
- autoriser ou interdire certains types d’employés PNJ

Cette configuration agit comme un **réglage de difficulté émergent** :

- Plus de PNJ :
  - charge cognitive réduite
  - stabilité accrue
  - gains potentiels réduits
  - risques narratifs supplémentaires

- Moins de PNJ :
  - pression élevée
  - gameplay plus manuel
  - gains maximisés si maîtrisé

Il n’existe pas de mode “facile / difficile” explicite.

---

## 10. Progression du club

La progression suit une logique de **statut social et d’influence**.

### Tier 1 — Speakeasy clandestin
- 100 % illégal
- faible capacité
- forte pression policière
- survie et débrouille

### Tier 2 — Façade légale + bar caché
- activité légale visible
- arrière-salle clandestine
- double gestion clientèle
- manipulation et diversion

### Tier 3 — Institution respectable
- façade publique crédible
- clientèle VIP élitiste
- opérations clandestines premium
- casino clandestin
- influence mafieuse et politique

---

## 11. Mafia, police et relations

Le monde du jeu est structuré par des forces sociales :

- familles mafieuses locales
- police corruptible
- notables et élites

Chaque famille mafieuse offre :
- des bonus spécifiques
- des contraintes et rivalités

Aucune alliance n’est objectivement optimale.  
Chaque choix ferme certaines portes et en ouvre d’autres.

---

## 12. Ressources principales

- 💵 Argent
- ⭐ Réputation
- 🚔 Pression policière
- 🟥 Relations mafieuses

Ces ressources conditionnent :
- l’accès au contenu
- la progression
- la difficulté des soirées

---

## 13. Casino clandestin (late game)

Le casino est une extension du club au Tier 3 :
- poker
- roulette
- paris illégaux

Il génère :
- revenus élevés
- nouveaux types de clients
- événements spécifiques
- un second front coop pendant la soirée

---

## 14. Interface & UX

Priorité à une **UI diégétique** :
- carnets papier
- tableaux à craie
- lettres et documents
- registres comptables

Un HUD minimal non diégétique peut exister en support si nécessaire.

---

## 15. Philosophie de l’échec

L’échec n’est jamais bloquant.

Une soirée ratée :
- raconte une histoire
- modifie les relations
- débloque parfois de nouvelles opportunités

Le jeu privilégie la continuité narrative à la punition.

---

## 16. Ce qui reste volontairement ouvert

Les éléments suivants seront détaillés dans des documents dédiés :
- archétypes précis de PNJ
- système d’événements et de signaux
- équilibrage économique fin
- vertical slice jouable
- architecture technique Unreal

---

## 17. Rôle du GDD

Ce document sert de :
- référence globale
- contrat de cohérence
- point d’entrée pour toute contribution future

Les détails d’implémentation sont volontairement exclus.