/* Fichier généré par tools/gen_assets.py à partir du dossier assets/.
 * Ne pas le modifier à la main : éditer les .txt puis lancer make assets. */
#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>

/* Hauteur d'une ligne de texte (les jambages débordent d'un pixel). */
#define FONT_HEIGHT 7
#define FONT_ROWS 8

/* Caractère de la police : chaque ligne est un octet, pixel de gauche
 * sur le bit de poids fort. */
typedef struct {
    uint32_t code;  /* point de code Unicode */
    uint8_t width;  /* largeur en pixels */
    uint8_t rows[FONT_ROWS];
} glyph_t;

/* Image : chaque ligne est un mot de 32 bits, pixel de gauche sur le
 * bit de poids fort. */
typedef struct {
    uint8_t w, h;
    uint32_t const *rows;
} sprite_t;

/* Les 95 premiers caractères sont l'ASCII imprimable (32 à 126) dans
 * l'ordre ; les suivants sont triés par point de code. */
extern glyph_t const FONT_GLYPHS[];
extern int const FONT_GLYPH_COUNT;

extern sprite_t const SPR_TILE_HIDDEN;
extern sprite_t const SPR_TILE_FLAG;
extern sprite_t const SPR_TILE_WRONG_FLAG;
extern sprite_t const SPR_TILE_OPEN;
extern sprite_t const SPR_TILE_1;
extern sprite_t const SPR_TILE_2;
extern sprite_t const SPR_TILE_3;
extern sprite_t const SPR_TILE_4;
extern sprite_t const SPR_TILE_5;
extern sprite_t const SPR_TILE_6;
extern sprite_t const SPR_TILE_7;
extern sprite_t const SPR_TILE_8;
extern sprite_t const SPR_TILE_MINE;
extern sprite_t const SPR_TILE_BOOM;
extern sprite_t const SPR_ICON_MINE;
extern sprite_t const SPR_ICON_CLOCK;
extern sprite_t const SPR_FACE_NORMAL;
extern sprite_t const SPR_FACE_WON;
extern sprite_t const SPR_FACE_LOST;
extern sprite_t const SPR_LOGO_MINE;

#endif /* ASSETS_H */
