/* render.h — Dessin de l'écran de jeu : bandeau et grille. */
#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>
#include <stdint.h>
#include "game.h"

/* Taille d'une case et hauteur du bandeau, en pixels. */
#define TILE_SIZE 8
#define HUD_HEIGHT 8

/* Bandeau du haut : mines restantes, visage, chronomètre. */
void render_hud(game_t const *g);

/* Grille ; le curseur est dessiné si [cursor] est vrai. */
void render_board(game_t const *g, bool cursor);

/* Écran de jeu complet (efface l'écran, ne l'affiche pas). */
void render_game(game_t const *g, bool cursor);

/* Écrit un temps sous la forme « 12,3 s » dans un tampon d'au moins
 * FORMAT_TIME_SIZE octets. */
#define FORMAT_TIME_SIZE 16
void format_time(char *buffer, uint32_t ms);

#endif /* RENDER_H */
