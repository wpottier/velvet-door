# Reputation System v0 — Vertical Slice

Status: v0.1 Baseline (Vertical Slice)  
Location: docs/gameplay/verticalSlice/reputation_system_v0.md  

---

## 1. Objectif

Introduire une mécanique de réputation du lieu qui :
- Donne un sens aux performances de la soirée
- Influence le flux et la valeur des clients
- Prépare l’évolution future (familles, allégeances, VIP) sans complexité excessive

Le système doit rester simple et testable au Vertical Slice.

---

## 2. Vision long terme (hors VS)

Chaque client est généré par le système à partir d’un profil comprenant :
- Budget
- Attentes de qualité
- Attentes d’ambiance
- Patience
- Sensibilité aux risques (police, bagarre, etc.)
- Typologie / famille / allégeance
- Potentiel VIP

À terme :
- La satisfaction client alimente une réputation globale
- La réputation influence le type de clients, leur nombre, et leur valeur
- La réputation est modulée par factions (mafia, notables, police, etc.)

---

## 3. Scope Vertical Slice (v0.1)

### 3.1 Variables client minimales

Chaque client v0.1 possède :

- Budget
  - Valeur de base : 10 à 25 (unité abstraite, $ narratif)
- Patience
  - Dérivée de l’archétype (Patient / Pressé / Exigeant)
- QualitySensitivity
  - Low ou Medium

Optionnel v0.1 (si simple à intégrer) :
- TipPotential
  - faible / normal

---

### 3.2 Satisfaction client (score 0–100)

Le score client doit être calculable de manière simple et lisible.

Composants recommandés :

- ServiceSpeedScore (0–60)
  - Basé sur le temps entre commande et service
- DrinkQualityScore (0–30)
  - Basé sur la qualité du produit servi (Low/Standard)
  - Modulé par QualitySensitivity
- DoorDecisionPenalty (0–10)
  - Refus ou timeout à la porte : pénalité (si le client n’est jamais entré)
- EventPenalty (0–10)
  - Placeholder v0.1 pour événements chaos futurs

Un score final client est clampé entre 0 et 100.

---

### 3.3 Réputation du lieu (score 0–100)

La réputation du lieu est une agrégation des scores de la soirée.

Recommandation v0.1 :
- ReputationNight = moyenne des scores clients servis
- DoorRefusals et timeouts peuvent influencer légèrement la moyenne (optionnel)

Valeur de départ :
- Reputation initiale = 50

Variation maximale par soirée :
- Capée à +/- 10 (pour éviter oscillations extrêmes au VS)

---

## 4. Effets de la réputation (v0.1)

Au Vertical Slice, la réputation influence légèrement :

### 4.1 Flux de clients (timer porte)

Base spawn timer : 6–10 secondes (défini ailleurs)

Effet réputation (léger) :
- Reputation >= 60 : base timer -1 seconde
- Reputation <= 40 : base timer +1 seconde

Objectif :
- Récompenser sans casser le pacing
- Garder la boucle jouable même en soirée ratée

---

### 4.2 Valeur des clients (budget moyen)

Effet réputation (léger) :
- Reputation >= 60 : Budget min +2, Budget max +2
- Reputation <= 40 : Budget min -2, Budget max -2

Option v0.1 :
- Si Reputation >= 70, chance d’un client “Medium+” (plus exigeant, plus riche)

---

## 5. Interaction avec la boucle macro (v0.1)

- Preparation : aucune influence directe
- Service : les performances génèrent Satisfaction
- WrapUp : calcul ReputationNight, affichage bilan
- DayStart suivant : appliquer effets spawn timer + budget

---

## 6. Notes d’implémentation (guide)

### Où stocker quoi

- Client : stocke ses valeurs (Budget/Patience/QualitySensitivity/SatisfactionScore)
- GS_Game (ou un gestionnaire de soirée) :
  - agrège les scores
  - calcule ReputationNight
- Système de porte :
  - lit la réputation au début de la soirée (ou dynamiquement si souhaité)
  - ajuste le spawn interval

### Lisibilité
Le joueur doit comprendre :
- Pourquoi la réputation monte/baisse
- Quelles actions influencent la satisfaction

Même au VS, un simple résumé “Top 3 reasons” au bilan est recommandé.

---

## 7. Ce qui est explicitement hors scope VS

- Réputation par faction/famille
- Modulation par ambiance (musique, danse, décor)
- VIP complexes
- Police influence directe
- Système de rumeurs / notoriété

Ces éléments seront ajoutés après validation du fun et de la boucle de base.