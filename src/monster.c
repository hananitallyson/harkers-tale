/* monster.c - Ordinary creatures and the four Harkers. */
#include <stdio.h>
#include "monster.h"
#include "lang.h"

typedef struct {
    const char *name;
    const char *description_en;
    int hearts;
    DamagePool damage_pool;
    int armor_rating;
    int level;
    int xp_reward;
} MonsterTemplate;

/*
 * Base statistics balanced against the Nimble 2E guidelines.
 *
 * Damage pools stay within 1-4 dice (Creator's Kit recommends at most
 * 3-4 small dice or 2-3 large dice per roll). Die sizes grow with
 * threat level: d6 for weak fey, d8 for standard threats, d10 for
 * elites and the final Harker.
 *
 * Armor Rating follows the three Nimble armor tiers mapped to a
 * numeric scale: light (5-7), medium (8-12), heavy (13-20).
 * Critical hits ignore Armor Rating, as per Nimble core rules.
 */
static const MonsterTemplate CREATURE_TEMPLATES[CREATURE_COUNT] = {
    /* Level 1 - light armor, modest pools */
    { "Fear Dearg", "A small fey in red who torments travellers.",
      16, { 2, 6 }, 5, 1, 20 },
    { "Púca", "A shapeshifter that lures the lost with false help.",
      18, { 2, 6 }, 5, 1, 20 },
    { "Corpse Light", "A pale light that leads travellers astray.",
      14, { 2, 6 }, 5, 1, 20 },

    /* Level 3-4 - medium-light armor */
    { "Bean Nighe", "A spectral washer of bloodied clothes.",
      20, { 2, 8 }, 7, 3, 24 },
    { "Fetch", "A silent double wearing your face.",
      21, { 2, 8 }, 7, 3, 24 },
    { "Redcap", "A vicious fey with an iron weapon.",
      22, { 3, 6 }, 8, 4, 26 },

    /* Level 5 - medium armor */
    { "Cailleach", "The Hag of Winter. Plants wither where she walks.",
      24, { 3, 6 }, 10, 5, 30 },
    { "Dearg Due", "A pale undead noblewoman who feeds on blood.",
      24, { 2, 10 }, 10, 5, 30 },
    { "Fachan", "A fiend with one eye, one arm and one leg.",
      26, { 3, 6 }, 10, 5, 30 },

    /* Level 7-8 - medium-heavy armor */
    { "Dullahan", "A headless rider who calls the doomed.",
      30, { 3, 8 }, 12, 7, 36 },
    { "Abhartach", "A dead chieftain who will not stay buried.",
      32, { 3, 8 }, 13, 7, 36 },
    { "Nuckelavee", "A skinless, horse-like horror from the sea.",
      32, { 4, 8 }, 14, 8, 40 }
};

/* Harkers: hand-tuned, not phase-scaled. Armor climbs to 20. */
static const MonsterTemplate HARKER_TEMPLATES[HARKER_COUNT] = {
    { "The Storyteller",
      "A scarecrow of straw and hay, trimmed with bright cloth and yarrow. A huge straw hat hides its face; long blond hair curls beneath.",
      38, { 3, 8 }, 10, 3, 60 },
    { "The Bell-Ringer",
      "A tall figure with a goat's head and legs, blue doll-patch cheeks and two pairs of horns. Flowers grow from the lower pair; bells and crow skulls hang from its pigtails.",
      53, { 4, 8 }, 14, 6, 80 },
    { "The Enkindled",
      "A mysterious entity of many forms. Its best-known shape is a hound, bound to a mask of tree bark.",
      65, { 4, 8 }, 17, 8, 100 },
    { "The Croon",
      "A tall, skeletal creature with an elongated moose skull, hollow eyes, great curved horns and a crown of long, pointed strands.",
      82, { 4, 10 }, 20, 12, 150 }
};

static void copy_template_to_monster(Monster *monster,
                                     const MonsterTemplate *base_template,
                                     int is_boss)
{
    snprintf(monster->name, NAME_LEN, "%s", base_template->name);
    snprintf(monster->description, DESC_LEN, "%s",
             translate(base_template->description_en, ""));
    monster->max_hearts = base_template->hearts;
    monster->hearts = base_template->hearts;
    monster->damage_pool = base_template->damage_pool;
    monster->armor_rating = base_template->armor_rating;
    monster->level = base_template->level;
    monster->xp_reward = base_template->xp_reward;
    monster->is_boss = is_boss;
}

static int scale_by_percent(int value, int percent)
{
    return (value * percent + 50) / 100;
}

void init_creature(Monster *monster, CreatureId id, int scaling_percent)
{
    copy_template_to_monster(monster, &CREATURE_TEMPLATES[id], 0);
    apply_phase_scaling(monster, scaling_percent);
}

void init_harker(Monster *monster, HarkerId id)
{
    copy_template_to_monster(monster, &HARKER_TEMPLATES[id], 1);
}

const char *get_harker_name(HarkerId id)
{
    return HARKER_TEMPLATES[id].name;
}

/*
 * apply_phase_scaling — Scales Hearts and XP; adds one damage die per
 * 25% above baseline. Armor Rating is never scaled.
 */
void apply_phase_scaling(Monster *monster, int scaling_percent)
{
    int extra_dice = (scaling_percent - 100) / 25;

    monster->max_hearts = scale_by_percent(monster->max_hearts, scaling_percent);
    monster->hearts = monster->max_hearts;
    if (extra_dice > 0) {
        monster->damage_pool.count += extra_dice;
    }
    monster->xp_reward = scale_by_percent(monster->xp_reward, scaling_percent);
    monster->level += extra_dice;
}

void damage_monster(Monster *monster, int damage)
{
    monster->hearts -= damage;
    if (monster->hearts < 0) {
        monster->hearts = 0;
    }
}