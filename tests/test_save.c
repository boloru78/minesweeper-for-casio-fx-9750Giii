/* test_save.c — Format du fichier de sauvegarde (save.c). */
#include "test.h"
#include "save.h"
#include <string.h>

/* Recalcule la somme de contrôle après avoir modifié un fichier à la main. */
static void fix_checksum(uint8_t *buf, size_t size)
{
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < size - 4; i++) {
        h ^= buf[i];
        h *= 16777619u;
    }
    for (int i = 0; i < 4; i++)
        buf[size - 4 + i] = h >> (8 * i);
}

/* Prépare des données avec des statistiques et une partie en cours. */
static void sample(save_data_t *d)
{
    save_defaults(d);
    stats_record(&d->stats, LEVEL_MEDIUM, true, 61200);
    stats_record(&d->stats, LEVEL_MEDIUM, false, 3000);
    stats_record(&d->stats, LEVEL_HARD, true, 123400);
    d->last_level = LEVEL_HARD;

    rng_t rng;
    rng_seed(&rng, 99);
    game_new(&d->game, LEVEL_HARD);
    board_reveal(&d->game.board, 8, 3, &rng);
    board_toggle_flag(&d->game.board, 0, 0);
    board_toggle_flag(&d->game.board, 15, 6);
    d->game.cx = 5;
    d->game.cy = 2;
    d->game.time_ms = 4200;
}

static void check_round_trip(void)
{
    save_data_t d, loaded;
    uint8_t buf[SAVE_MAX_SIZE];
    sample(&d);
    CHECK(game_in_progress(&d.game));

    size_t size = save_serialize(&d, buf, sizeof buf);
    CHECK(size > 0);
    CHECK(save_deserialize(&loaded, buf, size));

    CHECK(memcmp(&loaded.stats, &d.stats, sizeof d.stats) == 0);
    CHECK(loaded.last_level == LEVEL_HARD);
    game_t const *a = &d.game, *b = &loaded.game;
    CHECK(game_in_progress(b));
    CHECK(b->level == a->level && b->cx == a->cx && b->cy == a->cy);
    CHECK(b->time_ms == a->time_ms);
    CHECK(b->board.w == a->board.w && b->board.h == a->board.h);
    CHECK(b->board.mines == a->board.mines);
    CHECK(b->board.opened == a->board.opened);
    CHECK(b->board.flags == a->board.flags);
    CHECK(memcmp(b->board.cells, a->board.cells, a->board.w * a->board.h)
        == 0);

    /* Taille maximale : la plus grande grille tient dans le tampon. */
    board_init(&d.game.board, BOARD_MAX_W, BOARD_MAX_H, 10);
    d.game.board.state = BOARD_PLAYING;
    CHECK(save_serialize(&d, buf, sizeof buf) > 0);
    CHECK(save_serialize(&d, buf, 20) == 0);
}

static void check_finished_game_not_saved(void)
{
    save_data_t d, loaded;
    uint8_t buf[SAVE_MAX_SIZE];
    sample(&d);
    d.game.board.state = BOARD_LOST;

    size_t size = save_serialize(&d, buf, sizeof buf);
    CHECK(save_deserialize(&loaded, buf, size));
    CHECK(!game_in_progress(&loaded.game));
    CHECK(loaded.stats.level[LEVEL_HARD].won == 1);
}

static void check_corruption(void)
{
    save_data_t d, loaded;
    uint8_t buf[SAVE_MAX_SIZE], bad[SAVE_MAX_SIZE];
    sample(&d);
    size_t size = save_serialize(&d, buf, sizeof buf);

    /* Un octet modifié : tout est rejeté, on repart des valeurs par défaut. */
    memcpy(bad, buf, size);
    bad[10] ^= 0x01;
    CHECK(!save_deserialize(&loaded, bad, size));
    CHECK(loaded.stats.level[LEVEL_MEDIUM].played == 0);
    CHECK(!game_in_progress(&loaded.game));

    /* Fichier tronqué, vide ou étranger. */
    CHECK(!save_deserialize(&loaded, buf, size - 1));
    CHECK(!save_deserialize(&loaded, buf, 0));
    CHECK(!save_deserialize(&loaded, (uint8_t const *)"hello world", 11));

    /* Version inconnue (avec une somme de contrôle valide). */
    memcpy(bad, buf, size);
    bad[4] = 99;
    fix_checksum(bad, size);
    CHECK(!save_deserialize(&loaded, bad, size));

    /* Partie incohérente : une mine en moins. Les statistiques sont gardées
     * mais la partie est abandonnée. */
    memcpy(bad, buf, size);
    size_t cells = size - 4 - d.game.board.w * d.game.board.h;
    for (size_t i = cells; i < size - 4; i++) {
        if (bad[i] & CELL_MINE) {
            bad[i] &= ~CELL_MINE;
            break;
        }
    }
    fix_checksum(bad, size);
    CHECK(save_deserialize(&loaded, bad, size));
    CHECK(loaded.stats.level[LEVEL_MEDIUM].played == 2);
    CHECK(!game_in_progress(&loaded.game));
}

void test_save(void)
{
    check_round_trip();
    check_finished_game_not_saved();
    check_corruption();
}
