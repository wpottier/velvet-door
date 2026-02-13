# 🤲 Velvet Door — Interaction System

## 1. Rôle du système d’interaction

Le système d’interaction définit **comment le joueur agit concrètement dans le monde** de Velvet Door.

Il est au cœur de :
- l’immersion,
- la coopération,
- le rythme,
- le plaisir de jeu moment-to-moment.

Velvet Door privilégie des **interactions physiques, diégétiques et compréhensibles**, plutôt que des menus abstraits ou des actions automatisées.

---

## 2. Principes fondamentaux

### 2.1 Interaction physique en FPS

Toutes les actions importantes sont réalisées :
- en vue à la première personne,
- par manipulation directe d’objets ou d’éléments du décor,
- avec un feedback visuel et sonore clair.

Le joueur **fait les choses**, il ne les déclenche pas via des interfaces complexes.

---

### 2.2 Diégèse avant abstraction

Chaque fois que possible :
- l’interface est intégrée au monde (papier, tableau, objets),
- l’information est portée par l’environnement,
- les décisions passent par des actions concrètes.

Un HUD non diégétique minimal peut exister en support, mais ne doit jamais être central.

---

### 2.3 Actions simples, combinables

Une tâche est composée de **plusieurs actions simples**, combinables entre joueurs.

Règle clé :
> Une tâche complète doit nécessiter **3 à 5 actions maximum**.

Au-delà, elle devient pénible ; en dessous, elle perd de son intérêt coopératif.

---

## 3. Templates d’interaction (socle commun)

Velvet Door repose sur un nombre limité de **templates d’interaction génériques**, réutilisés dans de nombreux contextes.

Ces templates constituent la base du gameplay.

---

### 3.1 Prendre / Poser

- saisir un objet
- le poser sur une surface
- le déposer dans un contenant

Utilisé pour :
- bouteilles
- verres
- plateaux
- ingrédients
- documents
- argent

---

### 3.2 Verser / Transférer

- verser un liquide ou une ressource
- transférer d’un contenant à un autre

Utilisé pour :
- alcool
- jus
- ingrédients
- argent
- jetons de casino

---

### 3.3 Découper / Préparer

- utiliser un outil sur un objet
- produire un élément préparé

Utilisé pour :
- fruits
- nourriture
- garnitures
- éléments de préparation périssables

---

### 3.4 Mélanger / Assembler

- combiner plusieurs éléments
- produire un objet final ou intermédiaire

Utilisé pour :
- cocktails
- plats
- documents préparés
- organisation matérielle

---

### 3.5 Transporter

- déplacer un ou plusieurs objets
- gérer l’encombrement et les collisions

Utilisé pour :
- plateaux de verres
- caisses
- documents sensibles
- jetons

---

### 3.6 Activer / Configurer

- interagir avec un élément fixe
- modifier son état

Utilisé pour :
- éclairage
- musique
- accès
- tables de jeu
- zones du club

---

## 4. Tâches composées (exemple clé : cocktail)

Une tâche emblématique comme la préparation d’un cocktail illustre la philosophie du système.

Exemple générique :
- prendre un shaker
- ajouter des ingrédients (ordre flexible)
- préparer une garniture (optionnel)
- mélanger / secouer
- servir dans un verre
- transporter au client

Caractéristiques :
- certaines étapes peuvent être anticipées (préparation),
- plusieurs joueurs peuvent intervenir,
- des erreurs sont possibles (oubli, mauvaise quantité),
- les conséquences sont lisibles et parfois humoristiques.

---

## 5. Préparation et anticipation

Certaines interactions peuvent être effectuées **avant la soirée**.

Exemples :
- ingrédients découpés
- garnitures prêtes
- plateaux préparés
- organisation matérielle

### Règles
- la préparation consomme du temps et des ressources,
- les éléments préparés sont périssables,
- ce qui n’est pas utilisé est perdu en fin de soirée.

La préparation facilite le rush mais ne le supprime jamais.

---

## 6. Interaction coopérative

Le système est conçu pour encourager naturellement la coopération.

### Principes
- aucune tâche n’est strictement réservée à un joueur,
- les responsabilités émergent du contexte,
- l’entraide est toujours possible.

Exemples :
- un joueur prépare pendant qu’un autre sert,
- un joueur gère le bar pendant qu’un autre s’occupe des clients,
- un joueur rattrape une erreur d’un autre.

---

## 7. Erreurs, imprécisions et chaos

Les interactions acceptent une **marge d’erreur volontaire**.

Exemples :
- cocktail imparfait,
- oubli d’ingrédient,
- mauvais client servi,
- objet renversé ou cassé.

Les erreurs :
- ne bloquent pas le jeu,
- créent des situations,
- nourrissent l’humour noir et le chaos émergent.

---

## 8. Feedback joueur

Chaque interaction doit fournir :
- un feedback visuel clair,
- un feedback sonore distinct,
- une conséquence lisible.

Le joueur doit toujours comprendre :
- ce qu’il a fait,
- si c’était efficace,
- ce que cela implique pour la suite.

---

## 9. Charge cognitive et lisibilité

Le système d’interaction vise à :
- limiter la surcharge mentale,
- permettre la priorisation,
- rendre le chaos gérable.

Même en situation de stress élevé, le joueur doit pouvoir :
- identifier une action utile,
- comprendre où intervenir,
- agir rapidement.

---

## 10. Évolution du système

La progression du jeu n’ajoute pas de nouvelles commandes complexes.

Elle apporte :
- plus de variantes d’objets,
- plus de combinaisons possibles,
- plus de simultanéité,
- plus de conséquences.

La complexité vient du contexte, pas des contrôles.

---

## 11. Portée du document

Ce document définit :
- la philosophie d’interaction,
- les types d’actions possibles,
- la manière dont le joueur agit dans le monde.

Il ne définit pas :
- les mappings précis de contrôles,
- les animations détaillées,
- les implémentations techniques Unreal.

Ces éléments seront traités dans les documents techniques.