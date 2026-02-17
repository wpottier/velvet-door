# Balance Baseline v0 — Vertical Slice

Status: v0.1 Baseline (Vertical Slice)  
Location: docs/gameplay/verticalSlice/balance_baseline_v0.md  

Objectif : fournir des valeurs de départ testables pour transformer le design en système jouable.  
Ces valeurs sont volontairement simples et seront ajustées après 1–2 playtests.

---

## 1. Hypothèses de test

- Référence équilibre : 3 joueurs
- Support cible : 2 à 4 joueurs
- Objectif tension : modérée (pas “Overcooked hardcore”)
- Objectif durée : ~45 minutes par journée (macro loop)

---

## 2. Durées des phases (macro loop)

- Preparation Phase : 10 à 20 minutes (cible 15)
- Service Phase : 20 minutes
- WrapUp Phase : 5 à 10 minutes (cible 5)

Durée totale cible : ~40 à 45 minutes

---

## 3. Temps d’actions (interactions)

### 3.1 Porte (entrée clients)

- Ouvrir porte : 0.7s
- Fermer porte : 0.7s
- Patience client à la porte : 4 à 6s
- Délai prochain client après départ (refus/timeout/entrée) : 6 à 10s

Notes :
- Un seul client “en attente visible” à la porte en v0.1
- Le spawn ne dépend pas du fait que la porte soit ouverte

---

### 3.2 Trappe + Cave

- Ouvrir trappe : 1.0s
- Fermer trappe : 1.0s
- Descendre cave (temps de déplacement cible) : 3.0s
- Remonter cave (temps de déplacement cible) : 3.0s

Total “aller-retour cave minimal” : ~8s (hors action en cave)

---

### 3.3 Préparation boissons

- Couper citron : 2.0s
- Presser citron : 1.0s
- Remplir bouteille depuis tonnelet : 4.0s
- Mélanger cocktail : 3.0s
- Verser au verre : 1.0s
- Rincer verre ou bouteille : 1.5s

Temps indicatifs :
- Cocktail complet sans cave : ~7 à 9s
- Cocktail complet avec cave (aller-retour + remplissage) : ~15 à 18s

---

## 4. Clients (paramètres baseline)

### 4.1 Patience au bar

- Patience base : 14s

Archétypes v0.1 :
- Pressé : 10s
- Standard : 14s
- Patient : 18s

Règle :
- Si le service dépasse la patience, Satisfaction chute fortement

---

### 4.2 Consommation

- Durée consommation : 8 à 12s
- Reorder max : 1 (v0.1)

---

## 5. Capacité & pacing (à itérer en playtest)

### 5.1 Capacité maximale de clients simultanés

Valeur de départ : 6

Notes :
- 6 clients simultanés est la base d’équilibrage
- On testera 7–8 après validation du fun

### 5.2 Rythme d’arrivée

Délai prochain client : 6 à 10s (voir porte)

Notes :
- Le timer exact sera influencé légèrement par la réputation (cf. reputation_system_v0)

---

## 6. Ressources initiales (début de partie)

Objectif : forcer la logistique et la coordination dès le Tier 1.

- Bouteilles vides initiales : 4
- Citrons : 6
- Tonnelet capacité : 20 verres (ou portions)
- Glace : 0 (non disponible au départ, introduite via achat futur)

Notes :
- Les fruits coupés non utilisés sont jetés en fin de journée (v0.1)
- Les bouteilles/verres vides sont récupérables (pour rinçage + réutilisation)

---

## 7. Prix / argent (baseline très simple)

But : permettre un bilan clair sans économie complexe.

- Prix boisson standard : 5
- Pourboire (si très satisfait) : +1 à +2
- Pénalité refus/timeout porte (réputation) : voir reputation_system_v0

Notes :
- Les prix et budgets sont abstraits (valeur narrative “$”)
- L’objectif est de fournir un feedback immédiat au joueur

---

## 8. Objectifs de test (ce que doit valider ce baseline)

Le baseline v0.1 doit permettre d’observer :

- Si le trip cave (~8s + action) crée une tension “intéressante” et non “pénible”
- Si la patience client (10–18s) est compatible avec 2–4 joueurs
- Si 6 clients simultanés est un bon maximum pour un VS compréhensible
- Si la préparation (citron/bouteilles) est utile mais pas obligatoire à chaque seconde
- Si le rôle “porte” ajoute du fun et de la coordination

---

## 9. Paramètres prévus pour itération (après playtest)

Ajustements attendus :
- Patience bar : +/- 3s
- Délai spawn porte : +/- 2s
- Capacité max clients : 6 → 7/8 si trop facile
- Temps cave : 3s/3s → +/- 1s
- Temps remplissage bouteille : 4s → +/- 1s

Règle :
Ne changer qu’un petit groupe de paramètres entre deux playtests pour comprendre l’impact.