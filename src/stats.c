/* stats.c — Statistiques et meilleurs temps. */
#include "stats.h"

/* Incrémente un compteur sans dépasser sa valeur maximale. */
static void inc16(uint16_t *counter)
{
    if (*counter < UINT16_MAX)
        (*counter)++;
}

void stats_reset(stats_t *s)
{
    for (int i = 0; i < LEVEL_COUNT; i++) {
        s->level[i] = (level_stats_t){ .best_time_ms = STATS_NO_TIME };
    }
}

bool stats_record(stats_t *s, int level, bool won, uint32_t time_ms)
{
    if (level < 0 || level >= LEVEL_COUNT)
        return false;

    level_stats_t *ls = &s->level[level];
    inc16(&ls->played);

    if (!won) {
        ls->streak = 0;
        return false;
    }

    inc16(&ls->won);
    inc16(&ls->streak);
    if (ls->streak > ls->best_streak)
        ls->best_streak = ls->streak;

    if (time_ms < ls->best_time_ms) {
        ls->best_time_ms = time_ms;
        return true;
    }
    return false;
}

int stats_win_percent(level_stats_t const *ls)
{
    if (ls->played == 0)
        return 0;
    return (ls->won * 100 + ls->played / 2) / ls->played;
}
