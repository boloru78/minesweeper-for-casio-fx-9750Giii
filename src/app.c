/* app.c — Enchaînement des écrans du jeu. */
#include "app.h"
#include "help.h"
#include "platform.h"
#include "ui.h"

/* Données sauvegardées, partagées par tous les écrans. */
static save_data_t data;

/* Sauvegarde d'urgence avant la touche MENU ou une extinction. */
static void save_hook(void)
{
    save_store(&data);
}

bool abandon_current_game(save_data_t *d)
{
    game_t *g = &d->game;
    if (!game_in_progress(g))
        return true;
    if (!ui_confirm("Abandonner ?", "La partie en cours", "comptera perdue.",
            "Oui, abandonner", "Non", NULL, NULL))
        return false;

    stats_record(&d->stats, g->level, false, g->time_ms);
    game_new(g, g->level);
    save_store(d);
    return true;
}

void app_run(void)
{
    save_load(&data);
    pf_set_save_hook(save_hook);

    screen_t screen = SCREEN_MAIN;
    while (screen != SCREEN_QUIT) {
        switch (screen) {
        case SCREEN_MAIN:     screen = screen_main(&data); break;
        case SCREEN_NEW_GAME: screen = screen_new_game(&data); break;
        case SCREEN_PLAY:     screen = screen_play(&data); break;
        case SCREEN_STATS:    screen = screen_stats(&data); break;
        case SCREEN_HELP:     help_show(); screen = SCREEN_MAIN; break;
        default:              screen = SCREEN_QUIT; break;
        }
    }

    save_store(&data);
    pf_set_save_hook(NULL);
}
