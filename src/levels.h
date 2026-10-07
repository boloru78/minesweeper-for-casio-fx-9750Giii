/* levels.h — Niveaux de difficulté.
 *
 * Toutes les grilles tiennent sur l'écran de 128×64 pixels : des cases de
 * 8×8 pixels, sous un bandeau de 8 pixels, donnent au plus 16 × 7 cases. */
#ifndef LEVELS_H
#define LEVELS_H

typedef struct {
    char const *name; /* nom affiché dans les menus */
    int w, h;         /* dimensions de la grille, en cases */
    int mines;        /* nombre de mines */
} level_t;

enum { LEVEL_EASY, LEVEL_MEDIUM, LEVEL_HARD, LEVEL_COUNT };

extern level_t const LEVELS[LEVEL_COUNT];

#endif /* LEVELS_H */
