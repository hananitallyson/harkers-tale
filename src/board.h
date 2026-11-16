/* board.h - Phase boards, movement and tile events. */
#ifndef BOARD_H
#define BOARD_H

#include "types.h"

void build_board(GamePhase *phase, const ForestSection *sections);
void announce_new_section(const ForestSection *section);
void advance_hero(Game *game, int roll);
void handle_tile_event(Game *game);

#endif