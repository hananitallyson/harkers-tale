/* game.c - Game flow, narrative and mission progression. */
#include <stdio.h>
#include "game.h"
#include "board.h"
#include "combat.h"
#include "dice.h"
#include "hero.h"
#include "lang.h"
#include "monster.h"
#include "ui.h"

/* ---------- Setup ---------- */

static void init_section(ForestSection *section, const char *name, const char *atmosphere,
                         const CreatureId section_creatures[], HarkerId harker,
                         const char *taunt)
{
    int i;

    snprintf(section->name, NAME_LEN, "%s", name);
    snprintf(section->atmosphere, TEXT_LEN, "%s", atmosphere);
    for (i = 0; i < CREATURES_PER_SECTION; i++) {
        section->creatures[i] = section_creatures[i];
    }
    section->harker = harker;
    snprintf(section->harker_taunt, TEXT_LEN, "%s", taunt);
    section->harker_defeated = 0;
}

static void init_all_sections(ForestSection *sections)
{
    const CreatureId section_one_creatures[] = {
        CREATURE_FEAR_DEARG, CREATURE_PUCA, CREATURE_CORPSE_LIGHT
    };
    const CreatureId section_two_creatures[] = {
        CREATURE_BEAN_NIGHE, CREATURE_FETCH, CREATURE_REDCAP
    };
    const CreatureId section_three_creatures[] = {
        CREATURE_CAILLEACH, CREATURE_DEARG_DUE, CREATURE_FACHAN
    };
    const CreatureId section_four_creatures[] = {
        CREATURE_DULLAHAN, CREATURE_ABHARTACH, CREATURE_NUCKELAVEE
    };

    init_section(&sections[0],
                 translate("The Strawfield Verge", ""),
                 translate("Untended fields meet the first ancient trees. Straw crunches underfoot.", ""),
                 section_one_creatures, HARKER_STORYTELLER,
                 translate("\"Sit, little one. Let me tell you how your tale ends.\"", ""));
    init_section(&sections[1],
                 translate("The Glade of Bells", ""),
                 translate("Bells hang from every bough, and the chiming never quite stops.", ""),
                 section_two_creatures, HARKER_BELL_RINGER,
                 translate("\"Ding, dong! I have already rung your hour. Shall I ring it again?\"", ""));
    init_section(&sections[2],
                 translate("The Barkwood", ""),
                 translate("Bark peels from the trunks like discarded masks. Embers drift between them.", ""),
                 section_three_creatures, HARKER_ENKINDLED,
                 translate("A low growl comes from behind a mask of bark. \"Every flame needs its shadow.\"", ""));
    init_section(&sections[3],
                 translate("The Antler Deep", ""),
                 translate("The oldest heart of Coilsíde. Antlered bones lie in the moss. Somewhere ahead, Caoimhe waits.", ""),
                 section_four_creatures, HARKER_CROON,
                 translate("A hollow lullaby: \"Hush now, little one. Come away.\" You heard that voice the day Caoimhe vanished.", ""));
}

static void init_phase(GamePhase *phase, int number, const char *name, const char *description,
                       const char *mission, int scaling_percent,
                       int first_section, int last_section)
{
    phase->phase_number = number;
    snprintf(phase->phase_name, NAME_LEN, "%s", name);
    snprintf(phase->description, TEXT_LEN, "%s", description);
    snprintf(phase->mission_description, DESC_LEN, "%s", mission);
    phase->is_mission_completed = 0;
    phase->scaling_percent = scaling_percent;
    phase->first_section = first_section;
    phase->last_section = last_section;
}

static void init_all_phases(Game *game)
{
    int i;

    init_phase(&game->phases[0], 1,
               translate("The Edge of Coilsíde", ""),
               translate("You leave Caer Seilbhe and enter the forest.", ""),
               translate("Defeat The Storyteller.", ""),
               100, 0, 0);
    init_phase(&game->phases[1], 2,
               translate("The Wildwood Heart", ""),
               translate("The forest grows stranger. Two Harkers guard the way.", ""),
               translate("Defeat The Bell-Ringer, then The Enkindled.", ""),
               125, 1, 2);
    init_phase(&game->phases[2], 3,
               translate("The Hollow Crown", ""),
               translate("The deepest part of Coilsíde, where the trees bow over a single path.", ""),
               translate("Defeat The Croon and rescue Caoimhe.", ""),
               150, 3, 3);

    for (i = 0; i < PHASE_COUNT; i++) {
        build_board(&game->phases[i], game->sections);
    }
}

static void init_game(Game *game)
{
    game->current_phase = 0;
    game->current_section = 0;
    game->position = 0;
    game->is_over = 0;
    game->has_won = 0;
    init_all_sections(game->sections);
    init_all_phases(game);
}

/* ---------- Story pages ---------- */

static void begin_page(void)
{
    clear_screen();
    print_border();
}

static void end_page(void)
{
    print_border();
    wait_for_enter();
}

static void choose_hero(Hero *hero)
{
    Hero preview;
    char menu_line[TEXT_LEN];
    int i;

    begin_page();
    print_title(translate("Choose your protagonist", ""));
    print_border();
    for (i = 0; i < CLASS_COUNT; i++) {
        init_hero(&preview, (HeroClass)i);
        snprintf(menu_line, sizeof menu_line, "%d | %s, %s",
                 i + 1, preview.name, preview.title);
        print_row(menu_line, preview.ability_name);
        print_border();
    }
    init_hero(hero, (HeroClass)(read_integer("> ", 1, CLASS_COUNT) - 1));
}

static void show_prologue(const Hero *hero)
{
    begin_page();
    print_title(translate("Prologue", ""));
    print_border();
    SAY("Caer Seilbhe is a small town built on the ruins of an old fortress, beside the mysterious forest of Coilsíde. You are twelve and live there with your six-year-old sister, Caoimhe.", "");
    print_blank();
    SAY("One day, at the forest's edge, a tall skeletal creature with a moose skull and a crown of pointed strands steps from the trees: The Croon.", "");
    end_page();

    begin_page();
    SAYF("%s tries to protect Caoimhe, but The Croon sings, low and wrong, and brushes you aside like a leaf.",
         "", hero->name);
    print_blank();
    SAY("When you can move again, Caoimhe is gone. The four Harkers took her deep into Coilsíde.", "");
    print_blank();
    SAY("Defeat them all (The Storyteller, The Bell-Ringer, The Enkindled and, last, The Croon) and bring her home.", "");
    end_page();
}

static void show_defeat_screen(void)
{
    begin_page();
    print_title("GAME OVER");
    print_border();
    SAY("The forest closes over you. Caoimhe is still out there, waiting.", "");
    print_border();
}

static void show_victory_screen(const Hero *hero)
{
    begin_page();
    print_title(translate("VICTORY", ""));
    print_border();
    SAYF("The Croon falls. Caoimhe is curled in a bed of moss, frightened but unharmed. %s takes her hand and you walk home to Caer Seilbhe together.",
         "", hero->name);
    print_border();
}

/* ---------- Phases and missions ---------- */

static void begin_phase(Game *game)
{
    const GamePhase *phase = &game->phases[game->current_phase];
    char title_text[DESC_LEN];

    snprintf(title_text, sizeof title_text,
             translate("Phase %d: %s", ""),
             phase->phase_number, phase->phase_name);
    begin_page();
    print_title(title_text);
    print_border();
    print_text(phase->description);
    SAYF("Mission | %s", "", phase->mission_description);
    print_border();

    game->position = 0;
    game->current_section = phase->first_section;
    announce_new_section(&game->sections[phase->first_section]);
    end_page();
}

static void mark_harker_defeated(Game *game, int section_index)
{
    GamePhase *phase = &game->phases[game->current_phase];

    game->sections[section_index].harker_defeated = 1;
    if (section_index == phase->last_section) {
        phase->is_mission_completed = 1;
    }
}

void handle_harker_encounter(Game *game)
{
    const GamePhase *phase = &game->phases[game->current_phase];
    int section_index = phase->board[game->position].section;
    const ForestSection *section = &game->sections[section_index];
    Monster harker;

    if (section->harker_defeated) {
        return;
    }

    init_harker(&harker, section->harker);
    begin_page();
    print_title(harker.name);
    print_border();
    print_text(harker.description);
    print_blank();
    print_text(section->harker_taunt);
    end_page();

    if (run_combat(&game->hero, &harker) == OUTCOME_DEFEAT) {
        game->is_over = 1;
    } else {
        mark_harker_defeated(game, section_index);
    }
}

static void advance_phase_if_ready(Game *game)
{
    if (game->is_over || !game->phases[game->current_phase].is_mission_completed) {
        return;
    }
    if (game->current_phase == PHASE_COUNT - 1) {
        game->has_won = 1;
        return;
    }
    game->current_phase++;
    begin_phase(game);
}

/* ---------- Main loop ---------- */

static void show_turn_menu(const Game *game)
{
    const GamePhase *phase = &game->phases[game->current_phase];
    const Hero *hero = &game->hero;

    begin_page();
    SAYF("Phase %d | %s", "",
         phase->phase_number, game->sections[game->current_section].name);
    SAYF("%s | Hearts %d/%d | Lv %d | Tile %d/%d", "",
         hero->name, hero->hearts, hero->max_hearts,
         hero->level, game->position, phase->board_size - 1);
    print_border();
    SAY("1 | Roll the die", "");
    SAY("2 | View character", "");
    SAY("3 | Quit", "");
    print_border();
}

static void roll_and_move(Game *game)
{
    int dice_roll = roll_d6();
    int previous_section = game->current_section;

    begin_page();
    SAYF("You roll a %d.", "", dice_roll);
    advance_hero(game, dice_roll);
    if (game->current_section != previous_section) {
        end_page();
        clear_screen();
    }
    handle_tile_event(game);
    advance_phase_if_ready(game);
}

static void run_turn(Game *game)
{
    show_turn_menu(game);
    switch (read_integer("> ", 1, 3)) {
    case 1:
        roll_and_move(game);
        break;
    case 2:
        clear_screen();
        print_character_sheet(&game->hero);
        wait_for_enter();
        break;
    default:
        game->is_over = 1;
        break;
    }
}

void start_game(void)
{
    Game game;

    init_game(&game);
    choose_hero(&game.hero);
    show_prologue(&game.hero);
    begin_phase(&game);

    while (!game.is_over && !game.has_won) {
        run_turn(&game);
    }

    if (game.has_won) {
        show_victory_screen(&game.hero);
    } else if (game.hero.hearts <= 0) {
        show_defeat_screen();
    } else {
        clear_screen();
        printf("%s\n", translate("You abandon the search. Farewell.", ""));
    }
}