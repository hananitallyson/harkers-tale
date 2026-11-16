/* board.c - Phase boards, movement and tile events. */
#include <stdio.h>
#include "board.h"
#include "combat.h"
#include "dice.h"
#include "game.h"
#include "hero.h"
#include "monster.h"
#include "ui.h"

/* One character per tile: E empty, C combat, T treasure, H heal, X trap, M mission. */
static const char *BOARD_LAYOUTS[SECTION_COUNT] = {
    "ECCXCTCHCCM",
    "ECCXCTCHCCM",
    "ECCXCTCHCCM",
    "ECCXCTCHCCM"
};

static TileType tile_type_from_symbol(char symbol)
{
    switch (symbol) {
    case 'C':
        return TILE_COMBAT;
    case 'T':
        return TILE_TREASURE;
    case 'H':
        return TILE_HEAL;
    case 'X':
        return TILE_TRAP;
    case 'M':
        return TILE_MISSION;
    default:
        return TILE_EMPTY;
    }
}

static const char *get_trap_description(int section)
{
    switch (section) {
    case 0:
        return translate("A silent grey stranger (a Bodach) watches you. Every path leads you in circles.", "");
    case 1:
        return translate("The Sluagh, a restless host of spirits, shrieks through the branches.", "");
    case 2:
        return translate("A beautiful voice sings in the shadows. The Leanan Sídhe drains your strength.", "");
    default:
        return translate("An abandoned cradle wails in a voice far too old. A Changeling's trick.", "");
    }
}

static void set_tile_description(Tile *tile, const ForestSection *sections)
{
    const char *description;

    switch (tile->type) {
    case TILE_COMBAT:
        description = translate("Something stirs between the trees.", "");
        break;
    case TILE_TREASURE:
        description = translate("A bundle left by a lost traveller.", "");
        break;
    case TILE_HEAL:
        description = translate("A quiet ring of rowan trees. You rest.", "");
        break;
    case TILE_TRAP:
        description = get_trap_description(tile->section);
        break;
    case TILE_MISSION:
        snprintf(tile->description, DESC_LEN,
                 translate("The path ends at the domain of %s.", ""),
                 get_harker_name(sections[tile->section].harker));
        return;
    default:
        description = translate("The forest is quiet. Nothing happens.", "");
        break;
    }
    snprintf(tile->description, DESC_LEN, "%s", description);
}

void build_board(GamePhase *phase, const ForestSection *sections)
{
    int section;
    int i;
    Tile *current_tile;

    phase->board_size = 0;
    for (section = phase->first_section; section <= phase->last_section; section++) {
        for (i = 0; BOARD_LAYOUTS[section][i] != '\0' && phase->board_size < MAX_TILES; i++) {
            current_tile = &phase->board[phase->board_size];
            current_tile->tile_index = phase->board_size;
            current_tile->type = tile_type_from_symbol(BOARD_LAYOUTS[section][i]);
            current_tile->section = section;
            set_tile_description(current_tile, sections);
            phase->board_size++;
        }
    }
}

void announce_new_section(const ForestSection *section)
{
    print_title(section->name);
    print_text(section->atmosphere);
}

void advance_hero(Game *game, int roll)
{
    const GamePhase *phase = &game->phases[game->current_phase];
    int new_position = game->position + roll;
    int i;

    for (i = game->position + 1; i <= new_position && i < phase->board_size; i++) {
        if (phase->board[i].type == TILE_MISSION) {
            new_position = i;
            break;
        }
    }
    if (new_position > phase->board_size - 1) {
        new_position = phase->board_size - 1;
    }

    game->position = new_position;
    SAYF("You advance to tile %d/%d.", "", new_position, phase->board_size - 1);

    if (phase->board[new_position].section != game->current_section) {
        game->current_section = phase->board[new_position].section;
        print_border();
        announce_new_section(&game->sections[game->current_section]);
    }
}

static void handle_combat_tile(Game *game, const Tile *tile)
{
    const ForestSection *section = &game->sections[tile->section];
    Monster monster;
    CreatureId creature_id = section->creatures[roll_range(0, CREATURES_PER_SECTION - 1)];

    init_creature(&monster, creature_id,
                  game->phases[game->current_phase].scaling_percent);
    if (run_combat(&game->hero, &monster) == OUTCOME_DEFEAT) {
        game->is_over = 1;
    }
}

static void handle_treasure_tile(Hero *hero)
{
    if (roll_range(0, 1) == 0) {
        improve_weapon(hero);
        SAYF("A whetstone! Weapon bonus is now +%d.", "", hero->weapon_bonus);
    } else {
        improve_armor(hero);
        SAYF("A padded lining! Armor bonus is now +%d.", "", hero->armor_bonus);
    }
}

static void handle_heal_tile(Hero *hero)
{
    int hearts_before = hero->hearts;

    restore_hero_hearts(hero, hero->max_hearts / 2);
    SAYF("You recover %d Hearts (%d/%d).", "",
         hero->hearts - hearts_before, hero->hearts, hero->max_hearts);
}

static void handle_trap_tile(Hero *hero, int section)
{
    int damage = 8 + section * 3 - hero->wits;

    if (damage < 1) {
        damage = 1;
    }
    if (damage > hero->hearts - 1) {
        damage = hero->hearts - 1;
    }
    damage_hero(hero, damage);
    SAYF("Your Wits soften the blow. You lose %d Hearts (%d/%d).", "",
         damage, hero->hearts, hero->max_hearts);
}

void handle_tile_event(Game *game)
{
    const GamePhase *phase = &game->phases[game->current_phase];
    const Tile *current_tile = &phase->board[game->position];

    if (current_tile->type == TILE_COMBAT) {
        handle_combat_tile(game, current_tile);
        return;
    }
    if (current_tile->type == TILE_MISSION) {
        handle_harker_encounter(game);
        return;
    }

    print_border();
    print_text(current_tile->description);

    switch (current_tile->type) {
    case TILE_TREASURE:
        handle_treasure_tile(&game->hero);
        break;
    case TILE_HEAL:
        handle_heal_tile(&game->hero);
        break;
    case TILE_TRAP:
        handle_trap_tile(&game->hero, current_tile->section);
        break;
    default:
        break;
    }
    print_border();
    wait_for_enter();
}