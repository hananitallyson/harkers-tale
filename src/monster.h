/* monster.h - Ordinary creatures and the four Harkers. */
#ifndef MONSTER_H
#define MONSTER_H

#include "types.h"

void init_creature(Monster *monster, CreatureId id, int scaling_percent);
void init_harker(Monster *monster, HarkerId id);
const char *get_harker_name(HarkerId id);

void apply_phase_scaling(Monster *monster, int scaling_percent);
void damage_monster(Monster *monster, int damage);

#endif