/* save.c — Sauvegarde des statistiques et de la partie en cours.
 *
 * Format du fichier (version 1), entiers en petit-boutiste :
 *
 *   octets  contenu
 *   4       signature "MSWP"
 *   1       version du format (1)
 *   1       nombre de niveaux N
 *   12 × N  statistiques de chaque niveau :
 *             u16 parties, u16 victoires, u16 série, u16 meilleure série,
 *             u32 meilleur temps en ms (0xFFFFFFFF = aucun)
 *   1       dernier niveau choisi
 *   1       1 si une partie en cours suit, 0 sinon
 *   [partie en cours]
 *           u8 niveau, u8 curseur x, u8 curseur y, u32 temps en ms,
 *           u8 largeur, u8 hauteur, u8 explosion x, u8 explosion y,
 *           u16 mines, puis largeur × hauteur octets de cases (board.h)
 *   4       somme de contrôle FNV-1a de tous les octets précédents
 *
 * Les compteurs de cases ouvertes et de drapeaux ne sont pas stockés : ils
 * sont recalculés au chargement, ce qui évite toute incohérence. */
#include "save.h"
#include "platform.h"
#include <string.h>

#define SAVE_VERSION 1

static uint32_t fnv1a(uint8_t const *data, size_t size)
{
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < size; i++) {
        h ^= data[i];
        h *= 16777619u;
    }
    return h;
}

void save_defaults(save_data_t *d)
{
    memset(d, 0, sizeof *d);
    stats_reset(&d->stats);
    d->last_level = LEVEL_EASY;
    game_new(&d->game, LEVEL_EASY);
}

//---
// Écriture
//---

typedef struct {
    uint8_t *data;
    size_t size, max;
} writer_t;

static void put8(writer_t *w, uint32_t v)
{
    if (w->size < w->max)
        w->data[w->size] = v;
    w->size++;
}
static void put16(writer_t *w, uint32_t v)
{
    put8(w, v);
    put8(w, v >> 8);
}
static void put32(writer_t *w, uint32_t v)
{
    put16(w, v);
    put16(w, v >> 16);
}

size_t save_serialize(save_data_t const *d, uint8_t *buffer, size_t max)
{
    writer_t w = { buffer, 0, max };

    put8(&w, 'M'); put8(&w, 'S'); put8(&w, 'W'); put8(&w, 'P');
    put8(&w, SAVE_VERSION);
    put8(&w, LEVEL_COUNT);

    for (int i = 0; i < LEVEL_COUNT; i++) {
        level_stats_t const *ls = &d->stats.level[i];
        put16(&w, ls->played);
        put16(&w, ls->won);
        put16(&w, ls->streak);
        put16(&w, ls->best_streak);
        put32(&w, ls->best_time_ms);
    }
    put8(&w, d->last_level);

    game_t const *g = &d->game;
    bool has_game = game_in_progress(g);
    put8(&w, has_game);
    if (has_game) {
        board_t const *b = &g->board;
        put8(&w, g->level);
        put8(&w, g->cx);
        put8(&w, g->cy);
        put32(&w, g->time_ms);
        put8(&w, b->w);
        put8(&w, b->h);
        put8(&w, b->boom_x);
        put8(&w, b->boom_y);
        put16(&w, b->mines);
        for (int i = 0; i < b->w * b->h; i++)
            put8(&w, b->cells[i]);
    }

    if (w.size + 4 > max)
        return 0;
    put32(&w, fnv1a(buffer, w.size));
    return w.size;
}

//---
// Lecture
//---

typedef struct {
    uint8_t const *data;
    size_t size, pos;
    bool error;
} reader_t;

static uint32_t get8(reader_t *r)
{
    if (r->pos >= r->size) {
        r->error = true;
        return 0;
    }
    return r->data[r->pos++];
}
static uint32_t get16(reader_t *r)
{
    uint32_t lo = get8(r);
    return lo | (get8(r) << 8);
}
static uint32_t get32(reader_t *r)
{
    uint32_t lo = get16(r);
    return lo | (get16(r) << 16);
}

/* Lit et vérifie une partie en cours. Renvoie faux si elle est incohérente. */
static bool read_game(reader_t *r, game_t *g)
{
    int level = get8(r);
    int cx = get8(r), cy = get8(r);
    uint32_t time_ms = get32(r);
    int w = get8(r), h = get8(r);
    int boom_x = get8(r), boom_y = get8(r);
    int mines = get16(r);
    if (r->error || level >= LEVEL_COUNT)
        return false;
    if (w < 1 || h < 1 || w > BOARD_MAX_W || h > BOARD_MAX_H)
        return false;
    if (cx >= w || cy >= h || mines >= w * h)
        return false;

    game_new(g, level);
    board_t *b = &g->board;
    board_init(b, w, h, mines);
    b->boom_x = boom_x;
    b->boom_y = boom_y;
    g->cx = cx;
    g->cy = cy;
    g->time_ms = time_ms > GAME_MAX_TIME_MS ? GAME_MAX_TIME_MS : time_ms;

    int mine_count = 0;
    for (int i = 0; i < w * h; i++) {
        uint8_t c = get8(r);
        if ((c & 0x80) || (c & CELL_COUNT_MASK) > 8)
            return false;
        /* Une partie en cours n'a jamais de mine découverte. */
        if ((c & CELL_MINE) && (c & CELL_OPEN))
            return false;
        b->cells[i] = c;
        mine_count += (c & CELL_MINE) != 0;
        b->opened += (c & CELL_OPEN) != 0;
        b->flags += (c & CELL_FLAG) != 0;
    }
    if (r->error || mine_count != mines || b->opened >= w * h - mines)
        return false;

    b->state = BOARD_PLAYING;
    return true;
}

bool save_deserialize(save_data_t *d, uint8_t const *buffer, size_t size)
{
    save_defaults(d);

    if (size < 10 || memcmp(buffer, "MSWP", 4) != 0)
        return false;

    /* Somme de contrôle sur tout sauf ses propres 4 octets. */
    reader_t check = { buffer, size, size - 4, false };
    if (get32(&check) != fnv1a(buffer, size - 4))
        return false;

    reader_t r = { buffer, size - 4, 4, false };
    if (get8(&r) != SAVE_VERSION || get8(&r) != LEVEL_COUNT)
        return false;

    save_data_t loaded;
    save_defaults(&loaded);
    for (int i = 0; i < LEVEL_COUNT; i++) {
        level_stats_t *ls = &loaded.stats.level[i];
        ls->played = get16(&r);
        ls->won = get16(&r);
        ls->streak = get16(&r);
        ls->best_streak = get16(&r);
        ls->best_time_ms = get32(&r);
    }
    loaded.last_level = get8(&r);
    if (loaded.last_level >= LEVEL_COUNT)
        loaded.last_level = LEVEL_EASY;

    bool has_game = get8(&r);
    if (r.error)
        return false;

    /* Une partie illisible est ignorée, mais les statistiques sont gardées. */
    if (has_game && !read_game(&r, &loaded.game))
        game_new(&loaded.game, loaded.last_level);

    *d = loaded;
    return true;
}

//---
// Fichier
//---

bool save_load(save_data_t *d)
{
    uint8_t buffer[SAVE_MAX_SIZE];
    int size = pf_save_read(buffer, sizeof buffer);
    if (size <= 0) {
        save_defaults(d);
        return false;
    }
    return save_deserialize(d, buffer, size);
}

bool save_store(save_data_t const *d)
{
    uint8_t buffer[SAVE_MAX_SIZE];
    size_t size = save_serialize(d, buffer, sizeof buffer);
    return size > 0 && pf_save_write(buffer, size);
}
