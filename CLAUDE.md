# Notes pour Claude Code

Ce fichier est lu automatiquement par Claude Code au début de chaque session
ouverte dans ce dépôt. Il résume le projet, les choix déjà faits avec
l'auteur et ce qu'il reste à faire. **Mettez à jour la section « État actuel »
à la fin de chaque session.**

## Le projet

Un démineur pour la calculatrice **Casio fx-9750GIII**, écrit en C avec
[gint](https://git.planet-casio.com/Lephenixnoir/gint) et le
[fxSDK](https://git.planet-casio.com/Lephenixnoir/fxsdk). Il est livré sous
forme d'add-in `Mines.g1a`. Le README détaille les fonctionnalités,
l'installation et l'organisation du code.

## L'auteur et ses préférences

- Auteur : **boloru78**, sur **macOS**, avec l'application Claude Desktop.
- But : jouer, et montrer le projet (portfolio, communauté Planète Casio et
  Cemetech). Le dépôt est **public**, sous **licence MIT**.
- **Tout est en français** : README, commentaires, textes du jeu, messages de
  commit. Les identifiants du code restent en anglais.
- Claude écrit le code ; l'auteur teste sur sa calculatrice et fait ses
  retours. Il débute avec Git et le Terminal : expliquer les commandes pas à
  pas.
- Seule la fx-9750GIII est visée pour l'instant, pas d'autres modèles.

## Choix déjà faits (ne pas les remettre en cause sans demander)

- C + gint plutôt que Casio Basic ou Python : le Python officiel n'a pas de
  `getkey()`, ce qui le rend injouable.
- Niveaux qui tiennent à l'écran, sans défilement : Facile 9×7 (8 mines),
  Moyen 12×7 (14 mines), Difficile 16×7 (23 mines). Cases de 8×8 pixels sous
  un bandeau de 8 pixels.
- Fonctionnalités : premier coup sûr (la case et ses 8 voisines), cascade,
  drapeaux, compteur, chronomètre, records et statistiques, pause, reprise de
  partie, aide. Volontairement absents pour l'instant : chording, marques « ? »,
  partie personnalisée, grilles sans hasard.
- Commandes : flèches ; EXE ou SHIFT pour découvrir ; F1 ou ALPHA pour le
  drapeau ; EXIT pour la pause ; MENU pour le menu Casio (avec sauvegarde).
- SHIFT agit au **relâchement**, pour que SHIFT puis AC/ON éteigne la
  calculatrice sans découvrir de case (`resolve_shift()` dans
  `src/platform_gint.c`).
- Le nom affiché dans le menu Casio est **« Mines »** : le format `.g1a`
  limite le nom à 8 caractères. L'écran titre affiche « Minesweeper ».
- La police (avec accents) et les images sont dessinées en texte dans
  `assets/`. `tools/gen_assets.py` produit `src/assets.c`, à ne jamais
  modifier à la main.
- Le code du jeu est portable : seuls `src/main.c` et `src/platform_gint.c`
  utilisent gint. Le simulateur PC (`sim/`) implémente la même interface
  (`src/platform.h`).
- Une partie abandonnée compte comme perdue, après confirmation. La pause
  cache la grille. Extinction automatique après 10 minutes sans appui.

## Commandes utiles

```sh
make check          # tests + parcours des écrans + ressources à jour (comme la CI)
make test           # tests de la logique seulement
make screenshots    # régénère docs/images/ (après tout changement visuel)
make assets         # après une modification de assets/*.txt
```

Compiler l'add-in sur le Mac de l'auteur (Docker Desktop requis) :

```sh
docker run --rm -v "$PWD":/src -w /src manawyrm/fxsdk:0.0.3 \
    bash -c 'PATH=/root/.local/bin:$PATH fxsdk build-fx'
```

La CI (`.github/workflows/build.yml`) fait tout cela sur chaque push et chaque
Pull Request. Elle publie `Mines.g1a` comme artefact, et crée une release pour
chaque tag `v*`.

## Conventions

- C11, indentation de 4 espaces, commentaires en français.
- Compilation avec `-Wall -Wextra -Werror`, sur la calculatrice comme sur PC.
- Toute chaîne affichée doit tenir dans 128 pixels : `make check-texts` le
  vérifie. Pour un nouvel écran ou dialogue, l'ajouter à
  `sim/scripts/tour.txt`.
- Pour changer de version : `APP_VERSION` dans `src/app.h`, puis `project()`
  et `VERSION` (format `MM.mm.pppp`) dans `CMakeLists.txt`, puis un tag
  `vX.Y.Z`.
- Les fichiers de la calculatrice se lisent et s'écrivent dans
  `gint_world_switch()` (voir `pf_save_read()` / `pf_save_write()`).

## Transférer le jeu sur la calculatrice (macOS)

La calculatrice se branche en USB et se met en mode **USB Flash** (F1). Elle
apparaît alors dans `/Volumes/`. Le Finder ajoute des fichiers cachés (`._*`,
`.fseventsd`…), qui prennent de la place inutilement. Il vaut mieux copier
depuis le Terminal :

```sh
ls /Volumes                                  # nom du disque de la calculatrice
cp -X Mines.g1a "/Volumes/NOM/"              # copie sans fichier caché
dot_clean -m "/Volumes/NOM"                  # supprime les ._ éventuels
```

Éjecter ensuite le disque avant de débrancher. Le jeu occupe environ 43 Ko de
la mémoire de stockage (3 Mo) ; les programmes Basic sont dans la mémoire
principale (61 Ko) et n'apparaissent pas sur le disque USB.

## Historique

- **06/10/2026** : première session, dans le cloud. Questions sur les
  objectifs de l'auteur, puis écriture de tout le projet : jeu, simulateur,
  tests, outils, README, CI, modèles d'issues. Le `.g1a` a été compilé avec la
  vraie chaîne fxSDK 2.11 / gint 2.11 (43 Ko).
- **07/10/2026** : installation de l'application GitHub Claude sur le dépôt,
  push de la branche `claude/casio-minesweeper-project-h4751y`, ouverture de la
  [Pull Request #1](https://github.com/boloru78/minesweeper-for-casio-fx-9750Giii/pull/1).
  CI verte : tests et compilation de l'add-in.

## État actuel (07/10/2026)

- La PR #1 est ouverte et verte, mais **pas encore fusionnée** dans `main`.
- Le jeu **n'a pas encore été testé sur la vraie calculatrice**.

## Prochaines étapes

1. **Ménage de la calculatrice**, demandé par l'auteur. Lister les fichiers de
   la mémoire de stockage, puis tout supprimer **sauf Geometry Dash et ses
   fichiers de sauvegarde**, en demandant confirmation avant chaque
   suppression. Les applications intégrées et les programmes Basic ne sont pas
   sur le disque USB : ils ne risquent rien. Supprimer aussi les fichiers
   cachés de macOS.
2. **Copier `Mines.g1a`** sur la calculatrice (voir plus haut). Le fichier est
   dans l'artefact `Mines-g1a` de la dernière exécution de la CI, ou dans le
   dossier Téléchargements de l'auteur.
3. **Tester sur la calculatrice**, en priorité :
   - la sauvegarde : MENU, ouvrir une autre application, puis relancer le jeu
     (la partie doit reprendre) ;
   - SHIFT puis AC/ON (extinction sans découvrir de case) ;
   - le chronomètre et la répétition des flèches maintenues ;
   - l'affichage de l'icône « MINES » dans le menu.
4. **Fusionner la PR #1**, puis créer le tag `v1.0.0` pour publier la première
   release.
5. Idées pour la suite : voir la section du même nom dans le README.
