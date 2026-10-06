/* board.c — Règles du démineur. */
#include "board.h"
#include <string.h>

/* Décalages vers les 8 cases voisines. */
static int8_t const DX[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
static int8_t const DY[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

static inline bool inside(board_t const *b, int x, int y)
{
    return x >= 0 && y >= 0 && x < b->w && y < b->h;
}

void board_init(board_t *b, int w, int h, int mines)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (w > BOARD_MAX_W) w = BOARD_MAX_W;
    if (h > BOARD_MAX_H) h = BOARD_MAX_H;
    if (mines < 0) mines = 0;
    if (mines > w * h - 1) mines = w * h - 1;

    memset(b, 0, sizeof *b);
    b->w = w;
    b->h = h;
    b->mines = mines;
    b->state = BOARD_READY;
}

/* Vrai si (x, y) est dans le carré 3×3 centré sur (sx, sy). */
static bool near(int x, int y, int sx, int sy)
{
    return x >= sx - 1 && x <= sx + 1 && y >= sy - 1 && y <= sy + 1;
}

void board_place_mines(board_t *b, int sx, int sy, rng_t *rng)
{
    int const total = b->w * b->h;
    uint8_t candidates[BOARD_MAX_CELLS];
    int n = 0;

    /* Zone protégée : la case jouée et ses voisines, sauf si cela ne laisse
     * pas assez de place pour toutes les mines. */
    int protected_cells = 0;
    for (int i = 0; i < total; i++)
        protected_cells += near(i % b->w, i / b->w, sx, sy);
    bool protect_neighbors = (total - protected_cells >= b->mines);

    for (int i = 0; i < total; i++) {
        int x = i % b->w, y = i / b->w;
        bool safe = protect_neighbors ? near(x, y, sx, sy)
                                      : (x == sx && y == sy);
        if (!safe)
            candidates[n++] = i;
    }

    /* Mélange de Fisher-Yates partiel : les [mines] premières cases du
     * tableau sont tirées uniformément parmi les candidates. */
    for (int i = 0; i < b->mines && i < n; i++) {
        int j = i + rng_below(rng, n - i);
        uint8_t tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
        b->cells[candidates[i]] |= CELL_MINE;
    }

    /* Compte des mines voisines de chaque case. */
    for (int y = 0; y < b->h; y++)
    for (int x = 0; x < b->w; x++) {
        int count = 0;
        for (int k = 0; k < 8; k++) {
            int nx = x + DX[k], ny = y + DY[k];
            if (inside(b, nx, ny) && (board_cell(b, nx, ny) & CELL_MINE))
                count++;
        }
        b->cells[y * b->w + x] |= count;
    }

    b->state = BOARD_PLAYING;
}

/* Découvre une case sans mine. Renvoie vrai si elle était encore cachée. */
static bool open_cell(board_t *b, int i)
{
    uint8_t c = b->cells[i];
    if (c & (CELL_OPEN | CELL_FLAG | CELL_MINE))
        return false;
    b->cells[i] = c | CELL_OPEN;
    b->opened++;
    return true;
}

/* Découverte en cascade à partir d'une case sans mine voisine. On utilise une
 * pile explicite plutôt que la récursivité : la pile d'appels d'une
 * calculatrice est petite. Chaque case n'est empilée qu'une fois. */
static void flood(board_t *b, int start)
{
    uint8_t stack[BOARD_MAX_CELLS];
    int top = 0;
    stack[top++] = start;

    while (top > 0) {
        int i = stack[--top];
        int x = i % b->w, y = i / b->w;
        for (int k = 0; k < 8; k++) {
            int nx = x + DX[k], ny = y + DY[k];
            if (!inside(b, nx, ny))
                continue;
            int j = ny * b->w + nx;
            if (open_cell(b, j) && (b->cells[j] & CELL_COUNT_MASK) == 0)
                stack[top++] = j;
        }
    }
}

reveal_result_t board_reveal(board_t *b, int x, int y, rng_t *rng)
{
    if (!inside(b, x, y) || board_is_over(b))
        return REVEAL_NOTHING;

    int i = y * b->w + x;
    if (b->cells[i] & (CELL_OPEN | CELL_FLAG))
        return REVEAL_NOTHING;

    if (b->state == BOARD_READY)
        board_place_mines(b, x, y, rng);

    if (b->cells[i] & CELL_MINE) {
        b->cells[i] |= CELL_OPEN;
        b->state = BOARD_LOST;
        b->boom_x = x;
        b->boom_y = y;
        return REVEAL_BOOM;
    }

    open_cell(b, i);
    if ((b->cells[i] & CELL_COUNT_MASK) == 0)
        flood(b, i);

    if (b->opened == b->w * b->h - b->mines) {
        /* Victoire : comme sous Windows, toutes les mines reçoivent un
         * drapeau. */
        b->state = BOARD_WON;
        for (int j = 0; j < b->w * b->h; j++) {
            if (b->cells[j] & CELL_MINE)
                b->cells[j] |= CELL_FLAG;
        }
        b->flags = b->mines;
        return REVEAL_WIN;
    }
    return REVEAL_OK;
}

bool board_toggle_flag(board_t *b, int x, int y)
{
    if (!inside(b, x, y) || board_is_over(b))
        return false;

    int i = y * b->w + x;
    if (b->cells[i] & CELL_OPEN)
        return false;

    b->cells[i] ^= CELL_FLAG;
    if (b->cells[i] & CELL_FLAG)
        b->flags++;
    else
        b->flags--;
    return true;
}

int board_mines_left(board_t const *b)
{
    return (int)b->mines - (int)b->flags;
}

bool board_is_over(board_t const *b)
{
    return b->state == BOARD_WON || b->state == BOARD_LOST;
}
