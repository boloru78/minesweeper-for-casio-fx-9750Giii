/* screens.c — Menu principal, choix du niveau et statistiques. */
#include "app.h"
#include "gfx.h"
#include "levels.h"
#include "platform.h"
#include "render.h"
#include "ui.h"
#include <stdio.h>

//---
// Menu principal
//---

screen_t screen_main(save_data_t *d)
{
    enum { RESUME, NEW_GAME, STATS, HELP, QUIT, ACTION_COUNT };
    static char const *const labels[ACTION_COUNT] = {
        "Reprendre la partie", "Nouvelle partie", "Statistiques", "Aide",
        "Quitter",
    };

    /* « Reprendre » n'apparaît que s'il y a une partie en cours. */
    char const *items[ACTION_COUNT];
    int actions[ACTION_COUNT];
    int count = 0;
    for (int a = game_in_progress(&d->game) ? RESUME : NEW_GAME;
            a < ACTION_COUNT; a++) {
        items[count] = labels[a];
        actions[count++] = a;
    }

    int selected = 0;
    for (;;) {
        gfx_clear();
        gfx_sprite(2, 1, &SPR_LOGO_MINE, GFX_BLACK);
        gfx_text_bold_at(19, 4, "Minesweeper", GFX_BLACK, ALIGN_LEFT);
        gfx_text_at(GFX_WIDTH - 2, 4, "v" APP_VERSION, GFX_BLACK, ALIGN_RIGHT);
        gfx_rect(0, 15, GFX_WIDTH - 1, 15, GFX_BLACK);
        /* Liste centrée dans la place restante sous le titre. */
        int y = 19 + (ACTION_COUNT - count) * UI_ITEM_HEIGHT / 2;
        ui_items(2, GFX_WIDTH - 3, y, items, count, selected);
        gfx_present();

        if (ui_list_input(pf_getkey(false), &selected, count) != UI_CHOOSE)
            continue;
        switch (actions[selected]) {
        case RESUME:   return SCREEN_PLAY;
        case NEW_GAME: return SCREEN_NEW_GAME;
        case STATS:    return SCREEN_STATS;
        case HELP:     return SCREEN_HELP;
        default:       return SCREEN_QUIT;
        }
    }
}

//---
// Choix du niveau
//---

screen_t screen_new_game(save_data_t *d)
{
    int selected = d->last_level;
    char text[40];

    for (;;) {
        gfx_clear();
        ui_title_bar("Nouvelle partie");

        for (int i = 0; i < LEVEL_COUNT; i++) {
            level_t const *lv = &LEVELS[i];
            int y = 13 + i * (UI_ITEM_HEIGHT + 1);
            gfx_text(6, y, lv->name, GFX_BLACK);
            snprintf(text, sizeof text, "%d×%d", lv->w, lv->h);
            gfx_text_at(86, y, text, GFX_BLACK, ALIGN_RIGHT);
            snprintf(text, sizeof text, "%d", lv->mines);
            gfx_text_at(111, y, text, GFX_BLACK, ALIGN_RIGHT);
            gfx_sprite(115, y, &SPR_ICON_MINE, GFX_BLACK);
            if (i == selected)
                gfx_rect(2, y - 1, GFX_WIDTH - 3, y + FONT_HEIGHT, GFX_INVERT);
        }

        /* Rappel du record et des victoires du niveau sélectionné. */
        level_stats_t const *ls = &d->stats.level[selected];
        gfx_rect(0, 44, GFX_WIDTH - 1, 44, GFX_BLACK);
        if (ls->best_time_ms == STATS_NO_TIME) {
            snprintf(text, sizeof text, "Record : aucun");
        } else {
            char time[FORMAT_TIME_SIZE];
            format_time(time, ls->best_time_ms);
            snprintf(text, sizeof text, "Record : %s", time);
        }
        gfx_text_at(GFX_WIDTH / 2, 47, text, GFX_BLACK, ALIGN_CENTER);
        snprintf(text, sizeof text, "Victoires : %u / %u",
            (unsigned)ls->won, (unsigned)ls->played);
        gfx_text_at(GFX_WIDTH / 2, 56, text, GFX_BLACK, ALIGN_CENTER);
        gfx_present();

        switch (ui_list_input(pf_getkey(false), &selected, LEVEL_COUNT)) {
        case UI_CHOOSE:
            /* Une partie en cours doit d'abord être abandonnée. */
            if (!abandon_current_game(d))
                break;
            d->last_level = selected;
            game_new(&d->game, selected);
            return SCREEN_PLAY;
        case UI_CANCEL:
            return SCREEN_MAIN;
        default:
            break;
        }
    }
}

//---
// Statistiques
//---

/* Une ligne « intitulé ........ valeur ». */
static void stat_line(int y, char const *label, char const *value)
{
    gfx_text(2, y, label, GFX_BLACK);
    gfx_text_at(GFX_WIDTH - 3, y, value, GFX_BLACK, ALIGN_RIGHT);
}

screen_t screen_stats(save_data_t *d)
{
    int level = d->last_level;
    char text[40];

    for (;;) {
        level_stats_t const *ls = &d->stats.level[level];
        gfx_clear();
        ui_title_bar("Statistiques");
        snprintf(text, sizeof text, "◀ %s ▶", LEVELS[level].name);
        gfx_text_at(GFX_WIDTH / 2, 11, text, GFX_BLACK, ALIGN_CENTER);
        gfx_rect(0, 19, GFX_WIDTH - 1, 19, GFX_BLACK);

        snprintf(text, sizeof text, "%u", (unsigned)ls->played);
        stat_line(21, "Parties jouées", text);
        snprintf(text, sizeof text, "%u (%d %%)", (unsigned)ls->won,
            stats_win_percent(ls));
        stat_line(30, "Victoires", text);
        snprintf(text, sizeof text, "%u / %u", (unsigned)ls->streak,
            (unsigned)ls->best_streak);
        stat_line(39, "Série / max", text);
        if (ls->best_time_ms == STATS_NO_TIME)
            snprintf(text, sizeof text, "-");
        else
            format_time(text, ls->best_time_ms);
        stat_line(48, "Meilleur temps", text);
        gfx_text_at(GFX_WIDTH / 2, 57, "F6 : tout effacer", GFX_BLACK,
            ALIGN_CENTER);
        gfx_present();

        switch (pf_getkey(false)) {
        case IN_LEFT:
            level = (level + LEVEL_COUNT - 1) % LEVEL_COUNT;
            break;
        case IN_RIGHT:
            level = (level + 1) % LEVEL_COUNT;
            break;
        case IN_F6:
        case IN_DEL:
            if (ui_confirm("Tout effacer ?", "Les statistiques de",
                    "tous les niveaux.", "Oui, effacer", "Non", NULL,
                    NULL)) {
                stats_reset(&d->stats);
                save_store(d);
            }
            break;
        case IN_EXIT:
        case IN_EXE:
        case IN_SHIFT:
            return SCREEN_MAIN;
        default:
            break;
        }
    }
}
