/* game.c — État d'une partie. */
#include "game.h"
#include "levels.h"

void game_new(game_t *g, int level)
{
    if (level < 0 || level >= LEVEL_COUNT)
        level = LEVEL_EASY;

    level_t const *lv = &LEVELS[level];
    board_init(&g->board, lv->w, lv->h, lv->mines);
    g->level = level;
    g->cx = g->board.w / 2;
    g->cy = g->board.h / 2;
    g->time_ms = 0;
}

/* Reste de la division euclidienne (toujours positif). */
static int wrap(int value, int size)
{
    value %= size;
    return value < 0 ? value + size : value;
}

void game_move_cursor(game_t *g, int dx, int dy)
{
    g->cx = wrap(g->cx + dx, g->board.w);
    g->cy = wrap(g->cy + dy, g->board.h);
}

void game_add_time(game_t *g, uint32_t ms)
{
    if (ms > GAME_MAX_TIME_MS - g->time_ms)
        g->time_ms = GAME_MAX_TIME_MS;
    else
        g->time_ms += ms;
}

bool game_in_progress(game_t const *g)
{
    return g->board.state == BOARD_PLAYING;
}
