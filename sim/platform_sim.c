/* platform_sim.c — Implémentation de platform.h pour le simulateur PC.
 *
 * Le simulateur n'a pas de fenêtre : il lit une suite de touches dans un
 * script et enregistre les écrans demandés en images PBM (voir sim/README.md
 * pour la syntaxe des scripts). Le temps est simulé : chaque touche fait
 * avancer l'horloge, ce qui rend les captures parfaitement reproductibles. */
#include "platform.h"
#include "sim.h"
#include "gfx.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Image affichée en dernier. */
static uint32_t screen[GFX_HEIGHT * 4];

static sim_options_t opt;
static char *script;         /* contenu du script */
static char *cursor;         /* position de lecture */
static uint32_t now_ms;      /* horloge simulée */
static uint32_t key_delay_ms = 250;
static int wait_ticks;       /* tics d'attente restants (WAIT:n) */
static void (*save_hook)(void);

/* Enregistrement d'une animation (REC:on / REC:off). */
static bool recording;
static int frame_count;
static FILE *frame_index;

static void write_pbm(char const *path, uint32_t const *vram)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "sim : impossible d'écrire %s\n", path);
        exit(1);
    }
    fprintf(fp, "P4\n%d %d\n", GFX_WIDTH, GFX_HEIGHT);
    for (int i = 0; i < GFX_HEIGHT * 4; i++) {
        uint8_t bytes[4] = { vram[i] >> 24, vram[i] >> 16, vram[i] >> 8,
            vram[i] };
        fwrite(bytes, 1, 4, fp);
    }
    fclose(fp);
}

void sim_start(sim_options_t const *options)
{
    opt = *options;

    FILE *fp = fopen(opt.script_path, "rb");
    if (!fp) {
        fprintf(stderr, "sim : script introuvable : %s\n", opt.script_path);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    script = calloc(size + 1, 1);
    if (!script || fread(script, 1, size, fp) != (size_t)size) {
        fprintf(stderr, "sim : lecture impossible : %s\n", opt.script_path);
        exit(1);
    }
    fclose(fp);
    cursor = script;
}

int sim_finish(void)
{
    if (frame_index)
        fclose(frame_index);
    frame_index = NULL;
    free(script);
    script = cursor = NULL;

    /* Un texte trop long pour l'écran est une erreur. */
    if (gfx_text_clipped > 0) {
        fprintf(stderr, "sim : %d pixels de texte hors de l'écran\n",
            gfx_text_clipped);
        return 2;
    }
    return 0;
}

void pf_init(void) {}
void pf_quit(void) {}

void pf_present(uint32_t const *vram)
{
    memcpy(screen, vram, sizeof screen);
    if (recording) {
        char path[512];
        snprintf(path, sizeof path, "%s/frame-%04d.pbm", opt.out_dir,
            frame_count);
        write_pbm(path, screen);
        fprintf(frame_index, "frame-%04d.pbm %u\n", frame_count,
            (unsigned)now_ms);
        frame_count++;
    }
}

uint32_t pf_time_ms(void)
{
    return now_ms - now_ms % PF_TICK_MS;
}

uint32_t pf_entropy(void)
{
    return opt.seed;
}

void pf_set_save_hook(void (*hook)(void))
{
    save_hook = hook;
}

/* Lit le prochain mot du script (en sautant blancs et commentaires). */
static bool next_token(char *token, size_t size)
{
    for (;;) {
        while (isspace((unsigned char)*cursor))
            cursor++;
        if (*cursor == '#') {
            while (*cursor && *cursor != '\n')
                cursor++;
            continue;
        }
        break;
    }
    if (!*cursor)
        return false;

    size_t n = 0;
    while (*cursor && !isspace((unsigned char)*cursor)) {
        if (n + 1 < size)
            token[n++] = *cursor;
        cursor++;
    }
    token[n] = 0;
    return true;
}

static struct { char const *name; input_t key; } const KEYS[] = {
    { "UP", IN_UP }, { "DOWN", IN_DOWN }, { "LEFT", IN_LEFT },
    { "RIGHT", IN_RIGHT }, { "EXE", IN_EXE }, { "SHIFT", IN_SHIFT },
    { "ALPHA", IN_ALPHA }, { "F1", IN_F1 }, { "F2", IN_F2 },
    { "F3", IN_F3 }, { "F4", IN_F4 }, { "F5", IN_F5 }, { "F6", IN_F6 },
    { "EXIT", IN_EXIT }, { "DEL", IN_DEL }, { "OTHER", IN_OTHER },
};

input_t pf_getkey(bool timeout)
{
    char token[256];

    for (;;) {
        if (wait_ticks > 0) {
            wait_ticks--;
            now_ms += PF_TICK_MS;
            if (timeout)
                return IN_NONE;
            continue;
        }
        /* Fin du script : le simulateur s'arrête là. */
        if (!next_token(token, sizeof token))
            exit(sim_finish());

        if (!strncmp(token, "WAIT:", 5)) {
            wait_ticks = atoi(token + 5);
            continue;
        }
        if (!strncmp(token, "SHOT:", 5)) {
            char path[512];
            snprintf(path, sizeof path, "%s/%s.pbm", opt.out_dir, token + 5);
            write_pbm(path, screen);
            continue;
        }
        if (!strncmp(token, "SEED:", 5)) {
            opt.seed = strtoul(token + 5, NULL, 0);
            continue;
        }
        if (!strncmp(token, "DELAY:", 6)) {
            key_delay_ms = atoi(token + 6);
            continue;
        }
        if (!strcmp(token, "REC:on")) {
            char path[512];
            snprintf(path, sizeof path, "%s/frames.txt", opt.out_dir);
            if (!frame_index && !(frame_index = fopen(path, "w"))) {
                fprintf(stderr, "sim : impossible d'écrire %s\n", path);
                exit(1);
            }
            recording = true;
            pf_present(screen);
            continue;
        }
        if (!strcmp(token, "REC:off")) {
            /* Dernière image, pour que sa durée soit connue. */
            pf_present(screen);
            recording = false;
            continue;
        }
        if (!strcmp(token, "MENU")) {
            if (save_hook)
                save_hook();
            now_ms += key_delay_ms;
            return IN_RESUME;
        }

        for (size_t i = 0; i < sizeof KEYS / sizeof *KEYS; i++) {
            if (!strcmp(token, KEYS[i].name)) {
                now_ms += key_delay_ms;
                return KEYS[i].key;
            }
        }
        fprintf(stderr, "sim : instruction inconnue : %s\n", token);
        exit(1);
    }
}

int pf_save_read(void *buffer, size_t max_size)
{
    FILE *fp = fopen(opt.save_path, "rb");
    if (!fp)
        return -1;
    size_t size = fread(buffer, 1, max_size, fp);
    fclose(fp);
    return (int)size;
}

bool pf_save_write(void const *data, size_t size)
{
    FILE *fp = fopen(opt.save_path, "wb");
    if (!fp)
        return false;
    size_t written = fwrite(data, 1, size, fp);
    return (fclose(fp) == 0) && written == size;
}
