/* levels.c — Niveaux de difficulté.
 *
 * Les densités de mines (13 %, 17 % et 21 % des cases) sont proches de celles
 * du démineur de Windows (12 %, 16 % et 21 %). */
#include "levels.h"

level_t const LEVELS[LEVEL_COUNT] = {
    [LEVEL_EASY]   = { "Facile",     9, 7,  8 },
    [LEVEL_MEDIUM] = { "Moyen",     12, 7, 14 },
    [LEVEL_HARD]   = { "Difficile", 16, 7, 23 },
};
