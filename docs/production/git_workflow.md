# Velvet Door — Git Workflow (Solo Unreal Project)

## Objectif

Mettre en place un workflow Git simple et robuste, adapté à Unreal Engine et au développement solo.

Ce workflow vise à :
- Éviter les conflits sur assets binaires
- Garder une base stable en permanence
- Permettre l’expérimentation sans dupliquer les maps

---

## Structure des branches

### main
Version stable et jouable.
Toujours compilable.
Jamais cassée.

### dev
Branche de développement courant.
Peut être temporairement instable.
Fusionnée régulièrement vers main.

### feature/<nom>
Branches courtes pour une fonctionnalité précise.

Exemples :
- feature/network-lobby
- feature/input-rebinding
- feature/bar-blockout

Règles :
- Une feature branch = un ticket logique
- Durée courte
- Merge rapide vers dev
- Suppression après merge

---

## Versioning via tags

On ne duplique pas les maps pour marquer les étapes.
On utilise des tags Git.

Exemples :
- v0.1-vs-alpha
- v0.1-vs-beta
- v0.1-vs-demo

Un tag = photographie stable du projet.

---

## Gestion des assets Unreal

Utiliser Git LFS pour :
- .uasset
- .umap

Ne jamais versionner :
- Binaries
- Intermediate
- Saved
- DerivedDataCache

---

## Règles Unreal importantes

- Éviter les branches longues (assets binaires difficiles à merger)
- Ne pas dupliquer une map pour tester une idée
- Tester via feature branch, puis merge ou abandon

---

## Workflow quotidien recommandé

1. Pull dev
2. Créer feature/<nom>
3. Développer
4. Commit régulièrement
5. Tester localement
6. Merge vers dev
7. Supprimer la branche
8. À milestone stable → merge dev vers main + tag

---

## Règle d’or

Une seule version de chaque map.
Une seule source de vérité.
Git gère l’historique.
Unreal gère la structure.