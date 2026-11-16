/* combat.h - Turn-based combat using Nimble 2E damage pools. */
#ifndef COMBAT_H
#define COMBAT_H

#include "types.h"

/* Result of resolving one damage pool. */
typedef struct {
    int total_damage;   /* Sum of kept dice, before armor. */
    int is_miss;        /* Primary die rolled 1 and the caller allows misses. */
    int is_critical;    /* Primary die rolled its maximum (explodes). */
    int primary_kept;   /* Whether the Primary Die survived discarding. */
    int primary_roll;   /* Raw result of the Primary Die. */
} AttackResult;

/*
 * resolve_attack — Rolls a damage pool using Nimble 2E rules.
 *
 * @param pool                Damage pool. Position 0 is the Primary Die.
 * @param advantage           Levels of Advantage (adds dice, keeps lowest out).
 * @param disadvantage        Levels of Disadvantage (adds dice, keeps highest out).
 * @param primary_one_misses  If non-zero, a natural 1 on the Primary Die
 *                            is a miss. If zero, the 1 is a normal result
 *                            and still adds to the damage total.
 * @return                    Resolved AttackResult.
 */
AttackResult resolve_attack(DamagePool pool, int advantage, int disadvantage,
                            int primary_one_misses);

CombatOutcome run_combat(Hero *hero, Monster *monster);

#endif