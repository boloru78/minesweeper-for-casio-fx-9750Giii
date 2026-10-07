/* test_board.c — Règles du démineur (board.c), état d'une partie (game.c) et
 * statistiques (stats.c). */
#include "test.h"
#include "board.h"
#include "game.h"
#include "levels.h"
#include "stats.h"

static int count_mines(board_t const *b)
{
    int n = 0;
    for (int i = 0; i < b->w * b->h; i++)
        n += (b->cells[i] & CELL_MINE) != 0;
    return n;
}

static int neighbor_mines(board_t const *b, int x, int y)
{
    int n = 0;
    for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++) {
        int nx = x + dx, ny = y + dy;
        if ((dx || dy) && nx >= 0 && ny >= 0 && nx < b->w && ny < b->h)
            n += (board_cell(b, nx, ny) & CELL_MINE) != 0;
    }
    return n;
}

/* Vérifie tous les invariants d'une grille après un premier coup. */
static void check_first_reveal(int level, uint32_t seed)
{
    level_t const *lv = &LEVELS[level];
    rng_t rng;
    rng_seed(&rng, seed);
    int sx = rng_below(&rng, lv->w), sy = rng_below(&rng, lv->h);

    board_t b;
    board_init(&b, lv->w, lv->h, lv->mines);
    reveal_result_t r = board_reveal(&b, sx, sy, &rng);

    /* Le premier coup ne perd jamais et ouvre une zone. */
    CHECK(r == REVEAL_OK || r == REVEAL_WIN);
    CHECK(count_mines(&b) == lv->mines);
    for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++) {
        int x = sx + dx, y = sy + dy;
        if (x >= 0 && y >= 0 && x < b.w && y < b.h)
            CHECK(!(board_cell(&b, x, y) & CELL_MINE));
    }

    int opened = 0;
    for (int y = 0; y < b.h; y++)
    for (int x = 0; x < b.w; x++) {
        uint8_t c = board_cell(&b, x, y);
        /* Les compteurs correspondent aux mines voisines. */
        CHECK((c & CELL_COUNT_MASK) == neighbor_mines(&b, x, y));
        if (!(c & CELL_OPEN))
            continue;
        opened++;
        CHECK(!(c & CELL_MINE));
        /* Une case ouverte sans mine voisine a ouvert toutes ses voisines. */
        if ((c & CELL_COUNT_MASK) == 0) {
            for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && ny >= 0 && nx < b.w && ny < b.h)
                    CHECK(board_cell(&b, nx, ny) & CELL_OPEN);
            }
        }
    }
    CHECK(opened == b.opened);
    CHECK(board_cell(&b, sx, sy) & CELL_OPEN);
    CHECK((board_cell(&b, sx, sy) & CELL_COUNT_MASK) == 0);
}

/* Découvre toutes les cases sans mine : la partie doit être gagnée. */
static void check_win(void)
{
    board_t b;
    rng_t rng;
    rng_seed(&rng, 42);
    board_init(&b, 9, 7, 8);
    board_reveal(&b, 4, 3, &rng);

    reveal_result_t last = REVEAL_NOTHING;
    for (int y = 0; y < b.h; y++)
    for (int x = 0; x < b.w; x++) {
        uint8_t c = board_cell(&b, x, y);
        if (!(c & (CELL_MINE | CELL_OPEN)))
            last = board_reveal(&b, x, y, &rng);
    }
    CHECK(last == REVEAL_WIN);
    CHECK(b.state == BOARD_WON);
    CHECK(board_is_over(&b));
    CHECK(b.flags == b.mines);
    CHECK(board_mines_left(&b) == 0);
    for (int i = 0; i < b.w * b.h; i++) {
        if (b.cells[i] & CELL_MINE)
            CHECK(b.cells[i] & CELL_FLAG);
    }
    /* Plus rien ne bouge une fois la partie finie. */
    CHECK(board_reveal(&b, 0, 0, &rng) == REVEAL_NOTHING);
    CHECK(!board_toggle_flag(&b, 0, 0));
}

static void check_loss_and_flags(void)
{
    board_t b;
    rng_t rng;
    rng_seed(&rng, 7);
    board_init(&b, 16, 7, 23);
    board_reveal(&b, 0, 0, &rng);

    /* Trouve une mine et une case cachée sans mine. */
    int mx = -1, my = -1, hx = -1, hy = -1;
    for (int y = 0; y < b.h; y++)
    for (int x = 0; x < b.w; x++) {
        uint8_t c = board_cell(&b, x, y);
        if ((c & CELL_MINE) && mx < 0) { mx = x; my = y; }
        if (!(c & (CELL_MINE | CELL_OPEN)) && hx < 0) { hx = x; hy = y; }
    }
    CHECK(mx >= 0 && hx >= 0);

    /* Drapeaux : posés sur une case cachée seulement, ils protègent la
     * case et changent le compteur. */
    CHECK(!board_toggle_flag(&b, 0, 0)); /* case déjà ouverte */
    CHECK(board_toggle_flag(&b, mx, my));
    CHECK(b.flags == 1 && board_mines_left(&b) == 22);
    CHECK(board_reveal(&b, mx, my, &rng) == REVEAL_NOTHING);
    CHECK(board_toggle_flag(&b, mx, my));
    CHECK(b.flags == 0);
    CHECK(!board_toggle_flag(&b, -1, 0));
    CHECK(board_reveal(&b, 16, 0, &rng) == REVEAL_NOTHING);

    /* Le compteur de mines restantes peut devenir négatif. */
    board_t many = b;
    int placed = 0;
    for (int i = 0; i < many.w * many.h; i++) {
        if (!(many.cells[i] & CELL_OPEN)) {
            board_toggle_flag(&many, i % many.w, i / many.w);
            placed++;
        }
    }
    CHECK(board_mines_left(&many) == 23 - placed);
    CHECK(board_mines_left(&many) < 0);

    /* Découvrir une mine fait perdre. */
    CHECK(board_reveal(&b, hx, hy, &rng) != REVEAL_BOOM);
    CHECK(board_reveal(&b, mx, my, &rng) == REVEAL_BOOM);
    CHECK(b.state == BOARD_LOST);
    CHECK(b.boom_x == mx && b.boom_y == my);
    CHECK(board_is_over(&b));
}

static void check_limits(void)
{
    board_t b;
    rng_t rng;
    rng_seed(&rng, 3);

    /* Dimensions et nombre de mines ramenés dans les limites. */
    board_init(&b, 99, 99, 9999);
    CHECK(b.w == BOARD_MAX_W && b.h == BOARD_MAX_H);
    CHECK(b.mines == BOARD_MAX_CELLS - 1);
    board_init(&b, 0, -3, -1);
    CHECK(b.w == 1 && b.h == 1 && b.mines == 0);

    /* Grille trop pleine pour protéger les voisines : seule la case jouée
     * est épargnée, et la découvrir suffit à gagner. */
    board_init(&b, 3, 3, 8);
    CHECK(board_reveal(&b, 1, 1, &rng) == REVEAL_WIN);
    CHECK(count_mines(&b) == 8);
    CHECK((board_cell(&b, 1, 1) & CELL_COUNT_MASK) == 8);

    /* Grille sans mine : le premier coup ouvre tout. */
    board_init(&b, 5, 4, 0);
    CHECK(board_reveal(&b, 0, 0, &rng) == REVEAL_WIN);
    CHECK(b.opened == 20);
}

static void check_rng(void)
{
    rng_t a, b;
    rng_seed(&a, 123);
    rng_seed(&b, 123);
    for (int i = 0; i < 100; i++)
        CHECK(rng_next(&a) == rng_next(&b));

    /* Graine nulle acceptée, valeurs toujours dans l'intervalle. */
    rng_seed(&a, 0);
    int seen[7] = { 0 };
    for (int i = 0; i < 7000; i++) {
        uint32_t v = rng_below(&a, 7);
        CHECK(v < 7);
        if (v < 7)
            seen[v]++;
    }
    /* Répartition grossièrement uniforme (1000 attendus par valeur). */
    for (int i = 0; i < 7; i++)
        CHECK(seen[i] > 850 && seen[i] < 1150);
}

void test_board(void)
{
    for (int level = 0; level < LEVEL_COUNT; level++)
        for (uint32_t seed = 1; seed <= 300; seed++)
            check_first_reveal(level, seed);
    check_win();
    check_loss_and_flags();
    check_limits();
    check_rng();
}

void test_game_and_stats(void)
{
    game_t g;
    game_new(&g, LEVEL_MEDIUM);
    CHECK(g.board.w == 12 && g.board.h == 7 && g.board.mines == 14);
    CHECK(g.cx == 6 && g.cy == 3);
    CHECK(!game_in_progress(&g));

    /* Le curseur fait le tour de la grille. */
    game_new(&g, LEVEL_EASY);
    g.cx = 0;
    g.cy = 0;
    game_move_cursor(&g, -1, -1);
    CHECK(g.cx == 8 && g.cy == 6);
    game_move_cursor(&g, 1, 1);
    CHECK(g.cx == 0 && g.cy == 0);

    /* Le chronomètre est plafonné. */
    game_add_time(&g, 1500);
    CHECK(g.time_ms == 1500);
    game_add_time(&g, 0xffffffffu);
    CHECK(g.time_ms == GAME_MAX_TIME_MS);

    /* Un niveau invalide revient au niveau facile. */
    game_new(&g, 42);
    CHECK(g.level == LEVEL_EASY);

    stats_t s;
    stats_reset(&s);
    level_stats_t const *ls = &s.level[LEVEL_HARD];
    CHECK(ls->best_time_ms == STATS_NO_TIME);
    CHECK(stats_win_percent(ls) == 0);

    CHECK(stats_record(&s, LEVEL_HARD, true, 50000));  /* premier record */
    CHECK(!stats_record(&s, LEVEL_HARD, true, 60000)); /* plus lent */
    CHECK(stats_record(&s, LEVEL_HARD, true, 40000));  /* plus rapide */
    CHECK(!stats_record(&s, LEVEL_HARD, false, 1000)); /* défaite */
    CHECK(stats_record(&s, LEVEL_HARD, true, 39900));
    CHECK(ls->played == 5 && ls->won == 4);
    CHECK(ls->streak == 1 && ls->best_streak == 3);
    CHECK(ls->best_time_ms == 39900);
    CHECK(stats_win_percent(ls) == 80);
    CHECK(s.level[LEVEL_EASY].played == 0);
    CHECK(!stats_record(&s, LEVEL_COUNT, true, 1));

    /* 2 victoires sur 3 : 66,7 % arrondi à 67. */
    stats_reset(&s);
    stats_record(&s, LEVEL_EASY, true, 1);
    stats_record(&s, LEVEL_EASY, false, 1);
    stats_record(&s, LEVEL_EASY, true, 1);
    CHECK(stats_win_percent(&s.level[LEVEL_EASY]) == 67);

    /* Les compteurs ne débordent pas. */
    s.level[LEVEL_EASY].played = UINT16_MAX;
    stats_record(&s, LEVEL_EASY, false, 1);
    CHECK(s.level[LEVEL_EASY].played == UINT16_MAX);
}
