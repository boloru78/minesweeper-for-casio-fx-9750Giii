/* game.h — État d'une partie : grille, niveau, curseur et chronomètre.
 *
 * Comme board.h, ce module ne fait que de la logique ; la boucle interactive
 * (clavier, affichage) se trouve dans play.c. */
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>
#include "board.h"

/* Le chronomètre s'arrête à 999,9 secondes, comme sous Windows. */
#define GAME_MAX_TIME_MS 999900u

typedef struct {
    board_t board;
    uint8_t level;    /* indice dans LEVELS[] */
    uint8_t cx, cy;   /* position du curseur */
    uint32_t time_ms; /* temps de jeu écoulé */
} game_t;

/* Commence une nouvelle partie au niveau indiqué, curseur au centre. */
void game_new(game_t *g, int level);

/* Déplace le curseur ; il réapparaît de l'autre côté en sortant de la grille. */
void game_move_cursor(game_t *g, int dx, int dy);

/* Ajoute du temps de jeu (le chronomètre est plafonné). */
void game_add_time(game_t *g, uint32_t ms);

/* Vrai si une partie a commencé et n'est pas terminée : c'est elle que l'on
 * sauvegarde et que l'on peut reprendre. */
bool game_in_progress(game_t const *g);

#endif /* GAME_H */
