/* app.h — Enchaînement des écrans du jeu. */
#ifndef APP_H
#define APP_H

#include "save.h"

#define APP_VERSION "1.0.0"

/* Écrans du jeu. Chaque écran est une fonction qui renvoie l'écran suivant. */
typedef enum {
    SCREEN_MAIN,     /* menu principal */
    SCREEN_NEW_GAME, /* choix du niveau */
    SCREEN_PLAY,     /* partie */
    SCREEN_STATS,    /* statistiques */
    SCREEN_HELP,     /* aide */
    SCREEN_QUIT,     /* fin du programme */
} screen_t;

/* Boucle principale : charge la sauvegarde, affiche les écrans jusqu'à ce
 * que le joueur quitte, puis sauvegarde. */
void app_run(void);

/* Écrans (screens.c et play.c). */
screen_t screen_main(save_data_t *d);
screen_t screen_new_game(save_data_t *d);
screen_t screen_stats(save_data_t *d);
screen_t screen_play(save_data_t *d);

/* Demande s'il faut abandonner la partie en cours ; si oui, elle compte comme
 * perdue. Renvoie vrai s'il n'y a plus de partie en cours. */
bool abandon_current_game(save_data_t *d);

#endif /* APP_H */
