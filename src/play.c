/* play.c — Écran de jeu : la boucle interactive d'une partie. */
#include "app.h"
#include "gfx.h"
#include "help.h"
#include "platform.h"
#include "render.h"
#include "ui.h"
#include <stdio.h>

/* Durée pendant laquelle la grille finale reste visible avant le bilan. */
#define FINAL_BOARD_MS 1000

/* Fond du bilan de fin de partie : la grille, sans curseur. */
static void board_background(void const *context)
{
    render_hud(context);
    render_board(context, false);
}

/* Fond de la pause : le bandeau seul, la grille est cachée pour que le
 * chronomètre arrêté ne permette pas de réfléchir gratuitement. */
static void pause_background(void const *context)
{
    render_hud(context);
}

/* Montre la grille finale un instant (une touche abrège l'attente ; elle
 * n'est pas transmise au bilan pour éviter un choix par erreur). */
static void show_final_board(game_t const *g)
{
    render_game(g, false);
    gfx_present();
    uint32_t start = pf_time_ms();
    while (pf_time_ms() - start < FINAL_BOARD_MS) {
        input_t key = pf_getkey(true);
        if (key != IN_NONE && key != IN_RESUME)
            break;
    }
}

/* « Voir la grille » : affiche la grille jusqu'à la prochaine touche. */
static void view_board(game_t const *g)
{
    for (;;) {
        render_game(g, false);
        gfx_present();
        input_t key = pf_getkey(false);
        if (key != IN_NONE && key != IN_RESUME)
            return;
    }
}

/* Fin de partie : statistiques, sauvegarde puis bilan. */
static screen_t end_of_game(save_data_t *d, bool won)
{
    game_t *g = &d->game;
    bool record = stats_record(&d->stats, g->level, won, g->time_ms);
    save_store(d);
    show_final_board(g);

    char time[FORMAT_TIME_SIZE], line1[32], line2[32];
    format_time(time, g->time_ms);
    snprintf(line1, sizeof line1, "Temps : %s", time);
    if (won && record) {
        snprintf(line2, sizeof line2, "Nouveau record !");
    } else if (won) {
        format_time(time, d->stats.level[g->level].best_time_ms);
        snprintf(line2, sizeof line2, "Record : %s", time);
    } else {
        board_t const *b = &g->board;
        int safe = b->w * b->h - b->mines;
        snprintf(line2, sizeof line2, "Découvert : %d %%",
            b->opened * 100 / safe);
    }

    char const *lines[] = { line1, line2 };
    char const *items[] = { "Rejouer", "Voir la grille", "Menu principal" };
    int selected = 0;
    for (;;) {
        int choice = ui_dialog(won ? "Gagné !" : "Perdu !", lines, 2, items,
            3, selected, board_background, g);
        if (choice == 1) {
            view_board(g);
            selected = 1;
            continue;
        }
        if (choice == 0) {
            game_new(g, g->level);
            return SCREEN_PLAY;
        }
        return SCREEN_MAIN;
    }
}

/* Menu de pause (touche EXIT). */
static screen_t pause_menu(save_data_t *d)
{
    game_t *g = &d->game;
    char const *items[] = { "Reprendre", "Recommencer", "Aide",
        "Menu principal" };
    int selected = 0;

    for (;;) {
        switch (ui_dialog("Pause", NULL, 0, items, 4, selected,
                pause_background, g)) {
        case 1:
            if (abandon_current_game(d)) {
                game_new(g, g->level);
                return SCREEN_PLAY;
            }
            selected = 1;
            break;
        case 2:
            help_show();
            selected = 2;
            break;
        case 3:
            /* La partie reste en mémoire : « Reprendre la partie ». */
            save_store(d);
            return SCREEN_MAIN;
        default:
            return SCREEN_PLAY;
        }
    }
}

screen_t screen_play(save_data_t *d)
{
    game_t *g = &d->game;
    rng_t rng = { 0 };
    uint32_t last = pf_time_ms();

    for (;;) {
        render_game(g, true);
        gfx_present();

        /* Pendant la partie, pf_getkey() rend la main à chaque tic pour que
         * le chronomètre avance à l'écran. */
        input_t key = pf_getkey(game_in_progress(g));
        uint32_t now = pf_time_ms();
        /* Le temps passé dans le menu Casio ou éteint ne compte pas. */
        if (game_in_progress(g) && key != IN_RESUME)
            game_add_time(g, now - last);
        last = now;

        switch (key) {
        case IN_UP:    game_move_cursor(g, 0, -1); break;
        case IN_DOWN:  game_move_cursor(g, 0, 1); break;
        case IN_LEFT:  game_move_cursor(g, -1, 0); break;
        case IN_RIGHT: game_move_cursor(g, 1, 0); break;

        case IN_EXE:
        case IN_SHIFT: {
            /* Les mines sont posées au premier coup, avec une graine qui
             * dépend du moment où le joueur appuie. */
            if (g->board.state == BOARD_READY)
                rng_seed(&rng, pf_entropy());
            reveal_result_t r = board_reveal(&g->board, g->cx, g->cy, &rng);
            if (r == REVEAL_BOOM || r == REVEAL_WIN) {
                screen_t next = end_of_game(d, r == REVEAL_WIN);
                if (next != SCREEN_PLAY)
                    return next;
                last = pf_time_ms();
            }
            break;
        }

        case IN_F1:
        case IN_ALPHA:
            board_toggle_flag(&g->board, g->cx, g->cy);
            break;

        case IN_EXIT: {
            screen_t next = pause_menu(d);
            if (next != SCREEN_PLAY)
                return next;
            last = pf_time_ms();
            break;
        }

        default:
            break;
        }
    }
}
