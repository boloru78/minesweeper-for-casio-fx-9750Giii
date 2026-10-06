/* test_main.c — Lance tous les tests de logique sur PC : make test */
#include "test.h"

int tests_run = 0;
int tests_failed = 0;

int main(void)
{
    test_board();
    test_game_and_stats();
    test_save();
    test_gfx();

    if (tests_failed) {
        printf("%d vérification(s) ratée(s) sur %d\n", tests_failed,
            tests_run);
        return 1;
    }
    printf("%d vérifications réussies\n", tests_run);
    return 0;
}
