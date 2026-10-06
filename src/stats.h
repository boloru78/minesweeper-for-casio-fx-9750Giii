/* stats.h — Statistiques et meilleurs temps, niveau par niveau. */
#ifndef STATS_H
#define STATS_H

#include <stdbool.h>
#include <stdint.h>
#include "levels.h"

/* Valeur de best_time_ms quand aucune partie n'a encore été gagnée. */
#define STATS_NO_TIME UINT32_MAX

typedef struct {
    uint16_t played;       /* parties terminées ou abandonnées */
    uint16_t won;          /* parties gagnées */
    uint16_t streak;       /* victoires consécutives en cours */
    uint16_t best_streak;  /* plus longue série de victoires */
    uint32_t best_time_ms; /* meilleur temps, ou STATS_NO_TIME */
} level_stats_t;

typedef struct {
    level_stats_t level[LEVEL_COUNT];
} stats_t;

/* Remet toutes les statistiques à zéro. */
void stats_reset(stats_t *s);

/* Enregistre le résultat d'une partie (une partie abandonnée compte comme
 * perdue). Renvoie vrai si le temps est un nouveau record. */
bool stats_record(stats_t *s, int level, bool won, uint32_t time_ms);

/* Pourcentage de victoires arrondi à l'entier le plus proche (0 si aucune
 * partie). */
int stats_win_percent(level_stats_t const *ls);

#endif /* STATS_H */
