/* hero.h - Hero creation, statistics and progression. */
#ifndef HERO_H
#define HERO_H

#include "types.h"

void init_hero(Hero *hero, HeroClass class_type);
void reset_combat_effects(Hero *hero);

int get_attack_attribute(const Hero *hero);
int get_hero_armor_rating(const Hero *hero);
DamagePool get_hero_damage_pool(const Hero *hero);

void damage_hero(Hero *hero, int damage);
void restore_hero_hearts(Hero *hero, int amount);
void improve_weapon(Hero *hero);
void improve_armor(Hero *hero);

void add_experience(Hero *hero, int amount);
void grant_random_stat_bonus(Hero *hero);

void print_character_sheet(const Hero *hero);

#endif