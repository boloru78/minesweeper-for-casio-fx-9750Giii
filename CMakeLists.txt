# Compilation de l'add-in pour la calculatrice, avec le fxSDK :
#   fxsdk build-fx
# Le fichier Mines.g1a est produit à la racine du projet.

cmake_minimum_required(VERSION 3.15)
project(Minesweeper VERSION 1.0.0 LANGUAGES C)

include(GenerateG1A)
find_package(Gint 2.9 REQUIRED)

# Tout le jeu est dans src/ ; le simulateur PC (sim/) n'est pas inclus.
set(SOURCES
  src/main.c
  src/platform_gint.c
  src/app.c
  src/assets.c
  src/board.c
  src/game.c
  src/gfx.c
  src/help.c
  src/levels.c
  src/play.c
  src/render.c
  src/rng.c
  src/save.c
  src/screens.c
  src/stats.c
  src/ui.c
)

add_executable(mines ${SOURCES})
target_compile_options(mines PRIVATE -Wall -Wextra -Werror -Os -flto)
target_link_options(mines PRIVATE -flto)
target_link_libraries(mines Gint::Gint)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G)
  # Le format g1a limite le nom affiché à 8 caractères.
  generate_g1a(TARGET mines OUTPUT "Mines.g1a"
    NAME "Mines" INTERNAL "@MINES" VERSION "01.00.0000"
    ICON assets-fx/icon.png)
else()
  message(FATAL_ERROR "Ce jeu cible la fx-9750GIII : utiliser fxsdk build-fx")
endif()
