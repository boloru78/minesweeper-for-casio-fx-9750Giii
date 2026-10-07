/* board.h — Règles du démineur : grille, mines, découverte, drapeaux.
 *
 * Ce module ne dessine rien et ne lit pas le clavier : c'est de la logique
 * pure, testée automatiquement sur PC (voir tests/test_board.c). */
#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "rng.h"

/* Taille maximale d'une grille (16 × 8 cases de 8 pixels = l'écran entier). */
#define BOARD_MAX_W 16
#define BOARD_MAX_H 8
#define BOARD_MAX_CELLS (BOARD_MAX_W * BOARD_MAX_H)

/* Chaque case tient dans un octet :
 *   bits 0 à 3 : nombre de mines parmi les 8 cases voisines (0 à 8) ;
 *   bit 4      : la case contient une mine ;
 *   bit 5      : la case est découverte ;
 *   bit 6      : le joueur a posé un drapeau. */
#define CELL_COUNT_MASK 0x0f
#define CELL_MINE       0x10
#define CELL_OPEN       0x20
#define CELL_FLAG       0x40

typedef enum {
    BOARD_READY = 0, /* aucune case découverte : les mines ne sont pas posées */
    BOARD_PLAYING,   /* partie en cours */
    BOARD_WON,       /* toutes les cases sans mine sont découvertes */
    BOARD_LOST,      /* une mine a été découverte */
} board_state_t;

typedef enum {
    REVEAL_NOTHING = 0, /* rien ne change (case déjà ouverte, drapeau…) */
    REVEAL_OK,          /* une ou plusieurs cases ont été découvertes */
    REVEAL_BOOM,        /* le joueur a découvert une mine : perdu */
    REVEAL_WIN,         /* dernière case sûre découverte : gagné */
} reveal_result_t;

typedef struct {
    uint8_t w, h;           /* dimensions en cases */
    uint8_t state;          /* un board_state_t */
    uint8_t boom_x, boom_y; /* mine qui a explosé (si state == BOARD_LOST) */
    uint16_t mines;         /* nombre total de mines */
    uint16_t opened;        /* nombre de cases découvertes */
    uint16_t flags;         /* nombre de drapeaux posés */
    uint8_t cells[BOARD_MAX_CELLS]; /* ligne par ligne, w cases par ligne */
} board_t;

/* Prépare une grille vierge. Les dimensions sont ramenées dans les limites
 * autorisées et il reste toujours au moins une case sans mine. */
void board_init(board_t *b, int w, int h, int mines);

/* Pose les mines au hasard en épargnant la case (sx, sy) et, si la place le
 * permet, ses 8 voisines : le premier coup ouvre donc toujours une zone.
 * Appelée automatiquement par le premier board_reveal(). */
void board_place_mines(board_t *b, int sx, int sy, rng_t *rng);

/* Découvre la case (x, y). Au premier appel, les mines sont posées avec
 * [rng]. Une case sans mine voisine découvre aussi ses voisines, en cascade. */
reveal_result_t board_reveal(board_t *b, int x, int y, rng_t *rng);

/* Pose ou retire un drapeau sur une case encore cachée.
 * Renvoie vrai si la grille a changé. */
bool board_toggle_flag(board_t *b, int x, int y);

/* Mines restantes selon les drapeaux posés (peut être négatif). */
int board_mines_left(board_t const *b);

/* Vrai si la partie est terminée (gagnée ou perdue). */
bool board_is_over(board_t const *b);

/* Accès à une case ; (x, y) doit être dans la grille. */
static inline uint8_t board_cell(board_t const *b, int x, int y)
{
    return b->cells[y * b->w + x];
}

#endif /* BOARD_H */
