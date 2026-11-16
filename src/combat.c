/* combat.c - Turn-based combat using Nimble 2E damage pools. */
#include <stdio.h>
#include "combat.h"
#include "dice.h"
#include "hero.h"
#include "monster.h"
#include "ui.h"

#define MAX_POOL_DICE 20
#define FLEE_TARGET 12

/*
 * resolve_attack — Nimble 2E damage pool resolution.
 *
 *  - Leftmost die (index 0) is the Primary Die. Its position is preserved.
 *  - Primary Die = 1       => miss, only if `primary_one_misses` is set.
 *                             Monsters pass 0: a 1 on their Primary Die is
 *                             a normal result and still adds to damage.
 *  - Primary Die = maximum => Critical Hit; the die explodes and keeps
 *                             exploding while it rolls its maximum.
 *  - Other dice only add damage.
 *  - Advantage adds extra dice and discards the lowest results.
 *  - Disadvantage adds extra dice and discards the highest results.
 *  - Advantage and Disadvantage cancel one-for-one.
 */
AttackResult resolve_attack(DamagePool pool, int advantage, int disadvantage,
                            int primary_one_misses)
{
    AttackResult result;
    int net = advantage - disadvantage;
    int extra = net < 0 ? -net : net;
    int total = pool.count + extra;
    int rolls[MAX_POOL_DICE];
    int values[MAX_POOL_DICE];
    int keep[MAX_POOL_DICE];
    int i, j;

    result.total_damage = 0;
    result.is_miss = 0;
    result.is_critical = 0;
    result.primary_kept = 1;
    result.primary_roll = 0;

    if (total > MAX_POOL_DICE) total = MAX_POOL_DICE;
    if (total < 1) total = 1;

    for (i = 0; i < total; i++) {
        rolls[i] = roll_die(pool.sides);
        values[i] = rolls[i];
        keep[i] = 1;
    }

    result.primary_roll = rolls[0];

    /* Miss: only when the caller allows Primary Die 1 to be a miss. */
    if (rolls[0] == 1 && primary_one_misses) {
        result.is_miss = 1;
        return result;
    }

    /* Critical: Primary Die rolled its maximum; it explodes. */
    if (rolls[0] == pool.sides) {
        int explode;
        result.is_critical = 1;
        do {
            explode = roll_die(pool.sides);
            values[0] += explode;
        } while (explode == pool.sides);
    }

    /* Discard `extra` dice: lowest for advantage, highest for disadvantage. */
    for (i = 0; i < extra; i++) {
        int target = -1;
        int target_val = 0;
        int first = 1;

        for (j = 0; j < total; j++) {
            if (!keep[j]) continue;
            if (first) {
                target = j;
                target_val = values[j];
                first = 0;
            } else if (net > 0 && values[j] < target_val) {
                target = j;
                target_val = values[j];
            } else if (net < 0 && values[j] > target_val) {
                target = j;
                target_val = values[j];
            }
        }
        if (target >= 0) {
            keep[target] = 0;
        }
    }

    result.primary_kept = keep[0];

    for (i = 0; i < total; i++) {
        if (keep[i]) {
            result.total_damage += values[i];
        }
    }

    return result;
}

/* ---------- Combat display ---------- */

static void show_combat_status(const Hero *hero, const Monster *monster, int first_round)
{
    char right_column[DESC_LEN];

    print_border();
    snprintf(right_column, sizeof right_column,
             translate("Hearts %d/%d", ""), hero->hearts, hero->max_hearts);
    print_row(hero->name, right_column);
    snprintf(right_column, sizeof right_column,
             translate("Hearts %d/%d", ""), monster->hearts, monster->max_hearts);
    print_row(monster->name, right_column);
    print_border();

    if (first_round && !monster->is_boss) {
        SAYF("%s (Lv %d) confronts you!", "", monster->name, monster->level);
        print_text(monster->description);
        print_border();
    }
}

static int choose_combat_action(const Hero *hero)
{
    SAY("1 | Attack", "");
    SAYF("2 | Special: %s", "", hero->skill_name);
    SAY("3 | Flee", "");
    print_border();
    return read_integer("> ", ACTION_ATTACK, ACTION_FLEE);
}

static int attempt_flee(const Hero *hero)
{
    int dice_roll = roll_d20();

    SAYF("You try to flee: d20 %d + Agi %d vs %d.",
         "", dice_roll, hero->agility, FLEE_TARGET);
    return dice_roll + hero->agility >= FLEE_TARGET;
}

/* ---------- Hero turn ---------- */

static void perform_hero_attack(Hero *hero, Monster *monster)
{
    int adv = hero->ambush_ready ? 1 : 0;
    /* Heroes still miss on a natural 1 of the Primary Die. */
    AttackResult ar = resolve_attack(get_hero_damage_pool(hero), adv, 0, 1);
    int damage;

    hero->ambush_ready = 0;

    if (adv) {
        SAY("Ambush! Advantage applies.", "");
    }

    if (ar.is_miss) {
        SAY("Your attack misses!", "");
        return;
    }

    damage = ar.total_damage;
    if (!ar.is_critical) {
        damage -= monster->armor_rating;
    }
    if (damage < 0) {
        damage = 0;
    }
    damage_monster(monster, damage);

    if (ar.is_critical) {
        SAYF("Critical hit! Armor ignored. %d damage.", "", damage);
    } else {
        SAYF("You deal %d damage.", "", damage);
    }
}

static void activate_special_skill(Hero *hero)
{
    switch (hero->class_type) {
    case CLASS_BRAVE:
        hero->strong_counter = 1;
        SAY("Unyielding Stand! Your next Defend counter deals double damage.",
            "");
        break;
    case CLASS_WANDERER:
        hero->ambush_ready = 1;
        SAY("Lie in Wait! Ambush re-armed.", "");
        break;
    default:
        hero->evasive_step = 1;
        SAY("Evasive Step! The next attack against you has Disadvantage.",
            "");
        break;
    }
}

/* Bran's automatic counterattack after a successful Defend. */
static void perform_counterattack(Hero *hero, Monster *monster)
{
    /* Heroes still miss on a natural 1 of the Primary Die. */
    AttackResult ar = resolve_attack(get_hero_damage_pool(hero), 0, 0, 1);
    int damage;
    int doubled = hero->strong_counter;

    hero->strong_counter = 0;

    if (ar.is_miss) {
        SAY("Your counterattack misses.", "");
        return;
    }

    damage = ar.total_damage;
    if (!ar.is_critical) {
        damage -= monster->armor_rating;
    }
    if (damage < 0) {
        damage = 0;
    }
    if (doubled) {
        damage *= 2;
    }
    damage_monster(monster, damage);

    if (ar.is_critical) {
        SAYF("Critical counterattack! Armor ignored. %d damage.", "", damage);
    } else if (doubled) {
        SAYF("Unyielding counter! You deal %d damage.", "", damage);
    } else {
        SAYF("Counterattack! You deal %d damage.", "", damage);
    }
}

/* ---------- Monster turn ---------- */

/*
 * perform_monster_attack — Rolls the monster's damage pool, subtracts
 * the hero's Armor Rating (ignored on Critical Hits), then offers the
 * Defend reaction. Monsters never miss: a Primary Die result of 1 is
 * treated as an ordinary 1 and still adds to the damage total.
 * Returns 1 if the hero forfeits the next turn.
 */
static int perform_monster_attack(Hero *hero, Monster *monster)
{
    int disadv = hero->evasive_step ? 1 : 0;
    /* Monsters never miss: pass 0 for primary_one_misses. */
    AttackResult ar = resolve_attack(monster->damage_pool, 0, disadv, 0);
    int damage;
    int forfeits_turn = 0;

    hero->evasive_step = 0;

    if (disadv) {
        SAY("Your Evasive Step gives the attack Disadvantage.", "");
    }

    damage = ar.total_damage;
    if (!ar.is_critical) {
        damage -= get_hero_armor_rating(hero);
    }
    if (damage < 0) {
        damage = 0;
    }

    if (ar.is_critical) {
        SAYF("%s lands a critical hit! Armor ignored. %d damage.",
             "", monster->name, damage);
    } else {
        SAYF("%s attacks for %d damage.", "", monster->name, damage);
    }

    if (damage > 0) {
        SAY("Defend? 1=Yes (half damage, lose next turn)  2=No", "");
        if (read_integer("> ", 1, 2) == 1) {
            damage /= 2;
            forfeits_turn = 1;
            SAYF("You defend. Damage reduced to %d.", "", damage);
            if (hero->class_type == CLASS_BRAVE) {
                perform_counterattack(hero, monster);
            }
        }
    }

    damage_hero(hero, damage);
    return forfeits_turn;
}

/* ---------- Rewards ---------- */

static void give_victory_rewards(Hero *hero, const Monster *monster)
{
    SAYF("%s is defeated! +%d XP.", "", monster->name, monster->xp_reward);
    add_experience(hero, monster->xp_reward);
    grant_random_stat_bonus(hero);
}

static void finish_round(void)
{
    print_border();
    wait_for_enter();
}

/* ---------- Main combat loop ---------- */

CombatOutcome run_combat(Hero *hero, Monster *monster)
{
    int first_round = 1;
    int skip_hero_turn = 0;

    reset_combat_effects(hero);

    for (;;) {
        clear_screen();
        show_combat_status(hero, monster, first_round);
        first_round = 0;

        if (skip_hero_turn) {
            skip_hero_turn = 0;
            SAY("You are recovering from defending and lose your turn.", "");
        } else {
            CombatAction action = (CombatAction)choose_combat_action(hero);
            print_border();

            if (action == ACTION_ATTACK) {
                perform_hero_attack(hero, monster);
            } else if (action == ACTION_SPECIAL_SKILL) {
                activate_special_skill(hero);
            } else {
                if (monster->is_boss) {
                    SAY("No one escapes a Harker.", "");
                } else if (attempt_flee(hero)) {
                    SAY("You escape. No XP or rewards.", "");
                    reset_combat_effects(hero);
                    finish_round();
                    return OUTCOME_ESCAPE;
                } else {
                    SAY("You cannot get away!", "");
                }
            }
        }

        if (monster->hearts <= 0) {
            give_victory_rewards(hero, monster);
            reset_combat_effects(hero);
            finish_round();
            return OUTCOME_VICTORY;
        }

        if (perform_monster_attack(hero, monster)) {
            skip_hero_turn = 1;
        }

        if (hero->hearts <= 0) {
            finish_round();
            return OUTCOME_DEFEAT;
        }

        /* Bran's counterattack may have finished the monster. */
        if (monster->hearts <= 0) {
            give_victory_rewards(hero, monster);
            reset_combat_effects(hero);
            finish_round();
            return OUTCOME_VICTORY;
        }

        finish_round();
    }
}