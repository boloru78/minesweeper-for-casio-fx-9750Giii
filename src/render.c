/* render.c — Dessin de l'écran de jeu. */
#include "render.h"
#include "gfx.h"
#include <stdio.h>

static sprite_t const *const NUMBER_TILES[9] = {
    &SPR_TILE_OPEN, &SPR_TILE_1, &SPR_TILE_2, &SPR_TILE_3, &SPR_TILE_4,
    &SPR_TILE_5, &SPR_TILE_6, &SPR_TILE_7, &SPR_TILE_8,
};

void format_time(char *buffer, uint32_t ms)
{
    unsigned long tenths = ms / 100;
    snprintf(buffer, FORMAT_TIME_SIZE, "%lu,%lu s", tenths / 10, tenths % 10);
}

void render_hud(game_t const *g)
{
    board_t const *b = &g->board;
    char text[16];

    /* Mines restantes, sur trois chiffres comme sous Windows. */
    int left = board_mines_left(b);
    if (left < -99)
        left = -99;
    if (left >= 0)
        snprintf(text, sizeof text, "%03d", left);
    else
        snprintf(text, sizeof text, "-%02d", -left);
    gfx_sprite(0, 0, &SPR_ICON_MINE, GFX_BLACK);
    gfx_text(9, 0, text, GFX_BLACK);

    /* Visage au centre. */
    sprite_t const *face = &SPR_FACE_NORMAL;
    if (b->state == BOARD_WON)
        face = &SPR_FACE_WON;
    else if (b->state == BOARD_LOST)
        face = &SPR_FACE_LOST;
    gfx_sprite((GFX_WIDTH - face->w) / 2, 0, face, GFX_BLACK);

    /* Chronomètre en secondes entières. */
    snprintf(text, sizeof text, "%03lu", (unsigned long)(g->time_ms / 1000));
    int x = GFX_WIDTH - gfx_text_width(text);
    gfx_text(x, 0, text, GFX_BLACK);
    gfx_sprite(x - SPR_ICON_CLOCK.w - 2, 0, &SPR_ICON_CLOCK, GFX_BLACK);
}

/* Image d'une case selon son état et celui de la partie. */
static sprite_t const *tile_sprite(board_t const *b, int x, int y)
{
    uint8_t c = board_cell(b, x, y);
    bool mine = c & CELL_MINE, flag = c & CELL_FLAG, open = c & CELL_OPEN;

    if (b->state == BOARD_LOST) {
        /* En fin de partie perdue, on montre les mines et les erreurs. */
        if (x == b->boom_x && y == b->boom_y)
            return &SPR_TILE_BOOM;
        if (mine && !flag)
            return &SPR_TILE_MINE;
        if (flag && !mine)
            return &SPR_TILE_WRONG_FLAG;
    }
    if (flag)
        return &SPR_TILE_FLAG;
    if (open)
        return NUMBER_TILES[c & CELL_COUNT_MASK];
    return &SPR_TILE_HIDDEN;
}

void render_board(game_t const *g, bool cursor)
{
    board_t const *b = &g->board;
    int x0 = (GFX_WIDTH - b->w * TILE_SIZE) / 2;
    int y0 = HUD_HEIGHT;

    for (int y = 0; y < b->h; y++)
    for (int x = 0; x < b->w; x++) {
        int px = x0 + x * TILE_SIZE, py = y0 + y * TILE_SIZE;
        gfx_sprite(px, py, tile_sprite(b, x, y), GFX_BLACK);
    }

    if (cursor) {
        /* Le curseur inverse la partie visible de la case (7×7 pixels). */
        int px = x0 + g->cx * TILE_SIZE, py = y0 + g->cy * TILE_SIZE;
        gfx_rect(px, py, px + TILE_SIZE - 2, py + TILE_SIZE - 2, GFX_INVERT);
    }
}

void render_game(game_t const *g, bool cursor)
{
    gfx_clear();
    render_hud(g);
    render_board(g, cursor);
}
