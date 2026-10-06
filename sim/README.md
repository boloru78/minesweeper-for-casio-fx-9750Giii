# Simulateur PC

Le simulateur exécute **exactement le même code de jeu** que la calculatrice
(tout le dossier `src/` sauf `main.c` et `platform_gint.c`), mais sur
ordinateur et sans fenêtre : il rejoue une suite de touches écrite dans un
script et enregistre les écrans demandés.

Il sert à trois choses :

- générer les captures d'écran et l'animation du README (`make screenshots`) ;
- vérifier automatiquement qu'aucun texte ne dépasse de l'écran
  (`make check-texts`, lancé par la CI) ;
- essayer une modification sans transférer l'add-in sur la calculatrice.

## Utilisation

```sh
make sim
./build/minesweeper-sim [-s graine] [-o dossier] [-k] script.txt
python3 tools/pbm_to_png.py dossier dossier-png   # captures en PNG
```

| Option | Rôle |
| --- | --- |
| `-s graine` | graine des grilles (1 par défaut) ; même graine = mêmes grilles |
| `-o dossier` | où écrire les captures et la sauvegarde `MINES.sav` (`.` par défaut) |
| `-k` | garder la sauvegarde d'une exécution précédente (sinon on repart de zéro) |

Le programme renvoie `2` si un texte est sorti de l'écran pendant le script,
`1` en cas d'erreur dans le script, `0` sinon.

## Syntaxe des scripts

Un script est une suite de mots séparés par des espaces ou des retours à la
ligne. Tout ce qui suit un `#` est un commentaire.

| Instruction | Effet |
| --- | --- |
| `UP` `DOWN` `LEFT` `RIGHT` | flèches |
| `EXE` `SHIFT` `ALPHA` `EXIT` `DEL` | touches du même nom |
| `F1` … `F6` | touches de fonction |
| `MENU` | touche MENU : sauvegarde puis retour immédiat dans le jeu |
| `OTHER` | une touche sans effet |
| `WAIT:n` | laisse passer `n` tics de 100 ms sans toucher au clavier |
| `DELAY:ms` | durée simulée de chaque appui (250 ms par défaut) |
| `SEED:n` | change la graine des grilles suivantes |
| `SHOT:nom` | enregistre l'écran actuel dans `nom.pbm` |
| `REC:on` / `REC:off` | enregistre chaque image affichée (pour `demo.gif`) |

Le temps est simulé : chaque touche le fait avancer de `DELAY` millisecondes
et chaque tic de `WAIT` de 100 ms. Les captures sont donc identiques d'une
exécution à l'autre.

Le simulateur s'arrête à la fin du script, ou plus tôt si le jeu se termine
(choix « Quitter »).

## Scripts fournis

- [`scripts/screenshots.txt`](scripts/screenshots.txt) : captures et
  animation du README ;
- [`scripts/tour.txt`](scripts/tour.txt) : passe par tous les écrans et toutes
  les boîtes de dialogue (vérification des textes).
