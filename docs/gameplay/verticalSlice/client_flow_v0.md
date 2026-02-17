# Client Flow v0 — Vertical Slice

---

## États globaux

DoorPhase :
- Spawned
- Knocking
- WaitingDecision
- Entering
- Leaving

BarPhase :
- Ordering
- WaitingDrink
- Served
- Consuming
- OptionalReorder
- Exit

---

## Porte

- Patience : 3–6 secondes
- Timeout → client part
- Refus → client part
- Acceptation → entre

---

## Bar

- Commande uniquement au bar
- Impatience augmente en WaitingDrink
- Max 1 reorder au VS
- Dépose verre/bouteille vide

---

## Capacité

Nombre max clients simultanés :
À itérer via playtest.

Recommandation départ :
6–8 maximum.

---

## Objectif VS

Tester :
- Pression porte
- Pression bar
- Logistique cave
- Coordination coop