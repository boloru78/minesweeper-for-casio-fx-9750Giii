/* platform.h — Interface entre le jeu et la machine qui l'exécute.
 *
 * Tout le code du jeu est du C portable. Les seules fonctions qui dépendent
 * de la machine sont déclarées ici et implémentées deux fois :
 *   - src/platform_gint.c : la vraie calculatrice (gint / fxSDK) ;
 *   - sim/platform_sim.c  : le simulateur PC (tests, captures d'écran).
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Touches reconnues par le jeu (indépendantes des codes gint). */
typedef enum {
    IN_NONE = 0, /* aucune touche : délai écoulé (voir pf_getkey) */
    IN_RESUME,   /* retour du menu Casio ou rallumage : il faut redessiner */
    IN_UP,
    IN_DOWN,
    IN_LEFT,
    IN_RIGHT,
    IN_EXE,
    IN_SHIFT,
    IN_ALPHA,
    IN_F1,
    IN_F2,
    IN_F3,
    IN_F4,
    IN_F5,
    IN_F6,
    IN_EXIT,
    IN_DEL,
    IN_OTHER,    /* n'importe quelle autre touche */
} input_t;

/* Durée d'un tic d'horloge, en millisecondes. */
#define PF_TICK_MS 100

/* Initialise la plateforme (minuterie, clavier). */
void pf_init(void);

/* Libère les ressources avant de quitter. */
void pf_quit(void);

/* Affiche à l'écran une image 128×64 au format de gint : 4 mots de 32 bits
 * par ligne, bit de poids fort = pixel le plus à gauche, 1 = noir. */
void pf_present(uint32_t const *vram);

/* Attend une touche.
 * Si [timeout] est vrai, la fonction rend IN_NONE au prochain tic (toutes les
 * PF_TICK_MS ms) quand aucune touche n'est pressée : pratique pour faire
 * avancer le chronomètre.
 * Les touches MENU, SHIFT+AC/ON et la mise en veille automatique sont gérées
 * ici : la fonction appelle alors le crochet de sauvegarde puis rend IN_RESUME
 * quand le joueur revient dans le jeu. */
input_t pf_getkey(bool timeout);

/* Temps écoulé depuis pf_init(), en millisecondes (précision : un tic). */
uint32_t pf_time_ms(void);

/* Quelques bits imprévisibles pour initialiser le générateur aléatoire. */
uint32_t pf_entropy(void);

/* Fonction appelée juste avant de quitter le jeu de façon imprévue (touche
 * MENU, extinction) pour sauvegarder la progression. */
void pf_set_save_hook(void (*hook)(void));

/* Lecture / écriture du fichier de sauvegarde dans la mémoire de stockage.
 * pf_save_read() renvoie le nombre d'octets lus, ou -1 en cas d'erreur
 * (fichier absent par exemple). */
int pf_save_read(void *buffer, size_t max_size);
bool pf_save_write(void const *data, size_t size);

#endif /* PLATFORM_H */
