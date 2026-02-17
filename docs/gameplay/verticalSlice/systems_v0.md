# Systems v0 — Vertical Slice

Status: Active  

---

## 1. Système Porte

Flow :

1. Client spawn dans Spawn Cabinet
2. Frappe
3. Attend décision
4. Accepté → entre
5. Refusé → part
6. Timeout → part

### Règles

- Un seul client en attente visible
- Si refus ou timeout → timer aléatoire avant prochain spawn
- Spawn ne se déclenche pas si capacité max atteinte (à itérer)

---

## 2. Trappe Cave

- Ouverture 1s
- Fermeture 1s
- Bloque partiellement zone service
- Coordination nécessaire

---

## 3. Tonnelet → Bouteille

- Alcool stocké en cave
- Nécessite bouteille vide
- Mise en bouteille manuelle
- Stock limité

---

## 4. Gestion Bouteilles

- Nombre initial limité
- Verres/bouteilles vides récupérables
- Rinçage obligatoire

---

## 5. Fruits

- Ressource limitée
- Découpe manuelle
- Jetés fin journée

---

## 6. Qualité Alcool

QualityTier :
- Low
- Standard
- Premium (futur)

Impact potentiel :
- Satisfaction
- Événements

---

## 7. Fin de Journée

Conditions :
- Plus de nouveaux clients
- Clients restants quittent
- Tous joueurs se rendent au Bureau
- Interaction “Close for the night”
- Affichage bilan

---

## Exclus VS

- Inspection police complète
- Vol complexe
- Système sanitaire avancé