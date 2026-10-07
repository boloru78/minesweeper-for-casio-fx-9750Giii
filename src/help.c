/* help.c — Pages d'aide. */
#include "help.h"
#include "app.h"
#include "gfx.h"
#include "platform.h"
#include "ui.h"
#include <string.h>

help_page_t const HELP_PAGES[] = {
    { "But du jeu", {
        "Découvrir toutes les",
        "cases sans mine.",
        "Chaque chiffre donne",
        "le nombre de mines",
        "cachées dans les 8",
        "cases qui l'entourent.",
    } },
    { "Commandes", {
        "Flèches\tDéplacer",
        "EXE, SHIFT\tDécouvrir",
        "F1, ALPHA\tDrapeau",
        "EXIT\tPause",
        "MENU\tMenu Casio",
        "SHIFT AC/ON\tÉteindre",
    } },
    { "Astuces", {
        "Le premier coup ne",
        "tombe jamais sur une",
        "mine et ouvre une zone.",
        "Le compteur de gauche",
        "indique les mines",
        "restantes.",
    } },
    { "Sauvegarde", {
        "Statistiques et partie",
        "en cours sont gardées",
        "dans MINES.sav.",
        "Après MENU, la partie",
        "reprend là où elle",
        "s'était arrêtée.",
    } },
    { "Statistiques", {
        "Une partie abandonnée",
        "compte comme perdue.",
        "",
        "Dans l'écran des",
        "statistiques, F6",
        "efface tout.",
    } },
    { "À propos", {
        "Minesweeper " APP_VERSION,
        "par boloru78",
        "Licence MIT",
        "Créé avec gint/fxSDK",
        "Code source :",
        "github.com/boloru78",
    } },
};

int const HELP_PAGE_COUNT = sizeof HELP_PAGES / sizeof *HELP_PAGES;

/* Dessine une ligne, avec une éventuelle seconde colonne après « \t ». */
static void draw_line(int y, char const *line)
{
    char left[32];
    char const *tab = strchr(line, '\t');
    if (!tab) {
        gfx_text(2, y, line, GFX_BLACK);
        return;
    }
    size_t n = tab - line;
    if (n >= sizeof left)
        n = sizeof left - 1;
    memcpy(left, line, n);
    left[n] = 0;
    gfx_text(2, y, left, GFX_BLACK);
    gfx_text(HELP_COLUMN_X, y, tab + 1, GFX_BLACK);
}

void help_show(void)
{
    int page = 0;
    for (;;) {
        help_page_t const *p = &HELP_PAGES[page];
        gfx_clear();
        ui_title_bar(p->title);
        if (page > 0)
            gfx_text(1, 1, "◀", GFX_WHITE);
        if (page < HELP_PAGE_COUNT - 1)
            gfx_text_at(GFX_WIDTH - 2, 1, "▶", GFX_WHITE, ALIGN_RIGHT);
        for (int i = 0; i < HELP_LINES && p->lines[i]; i++)
            draw_line(11 + i * UI_ITEM_HEIGHT, p->lines[i]);
        gfx_present();

        switch (pf_getkey(false)) {
        case IN_LEFT:
        case IN_UP:
            if (page > 0)
                page--;
            break;
        case IN_RIGHT:
        case IN_DOWN:
            if (page < HELP_PAGE_COUNT - 1)
                page++;
            break;
        case IN_EXE:
        case IN_SHIFT:
            if (page == HELP_PAGE_COUNT - 1)
                return;
            page++;
            break;
        case IN_EXIT:
            return;
        default:
            break;
        }
    }
}
