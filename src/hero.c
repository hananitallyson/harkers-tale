/* hero.c - Hero creation, statistics and progression. */
#include <stdio.h>
#include "hero.h"
#include "dice.h"
#include "ui.h"

#define XP_LEVEL_FACTOR 25
#define LEVEL_HEARTS_BONUS 5
#define MAX_HERO_DICE 8
#define HERO_DIE_SIDES 6
#define DEFENSE_TO_ARMOR_DIVISOR 3

static int xp_needed_for_level(int level)
{
    return XP_LEVEL_FACTOR * level * level;
}

static void copy_text_to_field(char *destination, int buffer_size, const char *source_text)
{
    snprintf(destination, (size_t)buffer_size, "%s", source_text);
}

static void set_starting_stats(Hero *hero, int hearts, int strength, int agility,
                               int wits, int defense, int weapon_bonus, int armor_bonus)
{
    hero->max_hearts = hearts;
    hero->hearts = hearts;
    hero->strength = strength;
    hero->agility = agility;
    hero->wits = wits;
    hero->defense = defense;
    hero->weapon_bonus = weapon_bonus;
    hero->armor_bonus = armor_bonus;
}

void init_hero(Hero *hero, HeroClass class_type)
{
    hero->class_type = class_type;
    hero->level = 1;
    hero->xp = 0;

    switch (class_type) {
    case CLASS_BRAVE:
        copy_text_to_field(hero->name, NAME_LEN, "Bran");
        copy_text_to_field(hero->title, NAME_LEN, translate("The Brave", ""));
        set_starting_stats(hero, 50, 6, 3, 2, 1, 3, 2);
        copy_text_to_field(hero->weapon_name, NAME_LEN,
                           translate("Hazel Cudgel", ""));
        copy_text_to_field(hero->armor_name, NAME_LEN,
                           translate("Wool Jerkin", ""));
        copy_text_to_field(hero->ability_name, NAME_LEN,
                           translate("Counterattack", ""));
        copy_text_to_field(hero->ability_text, DESC_LEN,
                           translate("When you Defend against a hit, you strike back automatically.",
                                     ""));
        copy_text_to_field(hero->skill_name, NAME_LEN,
                           translate("Unyielding Stand", ""));
        copy_text_to_field(hero->skill_text, DESC_LEN,
                           translate("Your next Defend counter deals double damage.",
                                     ""));
        break;
    case CLASS_WANDERER:
        set_starting_stats(hero, 52, 3, 6, 3, 7, 3, 2);
        copy_text_to_field(hero->name, NAME_LEN, "Siofra");
        copy_text_to_field(hero->title, NAME_LEN, translate("The Wanderer", ""));
        copy_text_to_field(hero->weapon_name, NAME_LEN,
                           translate("Hunting Knife", ""));
        copy_text_to_field(hero->armor_name, NAME_LEN,
                           translate("Travelling Cloak", ""));
        copy_text_to_field(hero->ability_name, NAME_LEN, translate("Ambush", ""));
        copy_text_to_field(hero->ability_text, DESC_LEN,
                           translate("Your first attack of a fight has Advantage.",
                                     ""));
        copy_text_to_field(hero->skill_name, NAME_LEN,
                           translate("Lie in Wait", ""));
        copy_text_to_field(hero->skill_text, DESC_LEN,
                           translate("Re-arm your Ambush for your next attack.",
                                     ""));
        break;
    default:
        set_starting_stats(hero, 50, 2, 5, 6, 6, 3, 2);
        copy_text_to_field(hero->name, NAME_LEN, "Donn");
        copy_text_to_field(hero->title, NAME_LEN, translate("The Trickster", ""));
        copy_text_to_field(hero->weapon_name, NAME_LEN,
                           translate("Sling and Stones", ""));
        copy_text_to_field(hero->armor_name, NAME_LEN,
                           translate("Patched Coat", ""));
        copy_text_to_field(hero->ability_name, NAME_LEN,
                           translate("Evasive Step", ""));
        copy_text_to_field(hero->ability_text, DESC_LEN,
                           translate("Your next incoming attack has Disadvantage.",
                                     ""));
        copy_text_to_field(hero->skill_name, NAME_LEN,
                           translate("Evasive Step", ""));
        copy_text_to_field(hero->skill_text, DESC_LEN,
                           translate("Activate your ability on demand.", ""));
        break;
    }

    reset_combat_effects(hero);
}

void reset_combat_effects(Hero *hero)
{
    hero->defending = 0;
    hero->strong_counter = 0;
    hero->evasive_step = 0;
    hero->ambush_ready = (hero->class_type == CLASS_WANDERER);
}

int get_attack_attribute(const Hero *hero)
{
    switch (hero->class_type) {
    case CLASS_BRAVE:
        return hero->strength;
    case CLASS_WANDERER:
        return hero->agility;
    default:
        return hero->wits;
    }
}

/*
 * get_hero_armor_rating — Passive damage reduction.
 * Armor bonus plus a small share of the Defense stat.
 */
int get_hero_armor_rating(const Hero *hero)
{
    return hero->armor_bonus + hero->defense / DEFENSE_TO_ARMOR_DIVISOR;
}

/*
 * get_hero_damage_pool — Dice pool built from attack attribute and
 * weapon bonus. Every three combined points grants one d6, capped.
 */
DamagePool get_hero_damage_pool(const Hero *hero)
{
    DamagePool pool;
    int total = get_attack_attribute(hero) + hero->weapon_bonus;
    int dice = total / 3;

    if (dice < 1) {
        dice = 1;
    }
    if (dice > MAX_HERO_DICE) {
        dice = MAX_HERO_DICE;
    }
    pool.count = dice;
    pool.sides = HERO_DIE_SIDES;
    return pool;
}

void damage_hero(Hero *hero, int damage)
{
    hero->hearts -= damage;
    if (hero->hearts < 0) {
        hero->hearts = 0;
    }
}

void restore_hero_hearts(Hero *hero, int amount)
{
    hero->hearts += amount;
    if (hero->hearts > hero->max_hearts) {
        hero->hearts = hero->max_hearts;
    }
}

void improve_weapon(Hero *hero)
{
    hero->weapon_bonus++;
}

void improve_armor(Hero *hero)
{
    hero->armor_bonus++;
}

static void apply_level_up(Hero *hero)
{
    hero->level++;
    hero->max_hearts += LEVEL_HEARTS_BONUS;
    hero->hearts = hero->max_hearts;
    hero->strength++;
    hero->agility++;
    hero->wits++;
    hero->defense++;
    SAYF("Level %d! All stats +1, Hearts restored.", "", hero->level);
}

void add_experience(Hero *hero, int amount)
{
    hero->xp += amount;

    while (hero->xp >= xp_needed_for_level(hero->level)) {
        apply_level_up(hero);
    }
}

void grant_random_stat_bonus(Hero *hero)
{
    switch (roll_range(0, 3)) {
    case 0:
        hero->strength++;
        SAY("Bonus: Strength +1.", "");
        break;
    case 1:
        hero->agility++;
        SAY("Bonus: Agility +1.", "");
        break;
    case 2:
        hero->wits++;
        SAY("Bonus: Wits +1.", "");
        break;
    default:
        hero->defense++;
        SAY("Bonus: Defense +1.", "");
        break;
    }
}

void print_character_sheet(const Hero *hero)
{
    char left_column[DESC_LEN];
    char right_column[DESC_LEN];
    DamagePool pool = get_hero_damage_pool(hero);

    print_border();
    snprintf(left_column, sizeof left_column, "%s, %s", hero->name, hero->title);
    print_title(left_column);
    print_border();

    snprintf(left_column, sizeof left_column,
             translate("Hearts %d/%d", ""), hero->hearts, hero->max_hearts);
    snprintf(right_column, sizeof right_column,
             translate("Level %d", ""), hero->level);
    print_row(left_column, right_column);

    snprintf(left_column, sizeof left_column,
             translate("Strength %d", ""), hero->strength);
    snprintf(right_column, sizeof right_column,
             translate("Agility %d", ""), hero->agility);
    print_row(left_column, right_column);

    snprintf(left_column, sizeof left_column,
             translate("Wits %d", ""), hero->wits);
    snprintf(right_column, sizeof right_column,
             translate("Defense %d", ""), hero->defense);
    print_row(left_column, right_column);
    print_border();

    SAYF("Weapon | %s +%d", "", hero->weapon_name, hero->weapon_bonus);
    SAYF("Armor  | %s +%d (AR %d)", "",
         hero->armor_name, hero->armor_bonus, get_hero_armor_rating(hero));
    SAYF("Damage | %dd%d", "", pool.count, pool.sides);
    SAYF("Ability | %s", "", hero->ability_name);
    print_text(hero->ability_text);
    SAYF("Special | %s", "", hero->skill_name);
    print_text(hero->skill_text);
    print_border();
}