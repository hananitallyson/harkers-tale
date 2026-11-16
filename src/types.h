/* types.h - Shared enumerations and structures for Harker's Tale. */
#ifndef TYPES_H
#define TYPES_H

#define NAME_LEN 50
#define DESC_LEN 256
#define TEXT_LEN 400
#define MAX_TILES 100
#define PHASE_COUNT 3
#define SECTION_COUNT 4
#define CREATURES_PER_SECTION 3

/* The three playable protagonists. */
typedef enum {
    CLASS_BRAVE,
    CLASS_WANDERER,
    CLASS_TRICKSTER,
    CLASS_COUNT
} HeroClass;

/* Board tile types. */
typedef enum {
    TILE_EMPTY,
    TILE_COMBAT,
    TILE_TREASURE,
    TILE_HEAL,
    TILE_TRAP,
    TILE_MISSION
} TileType;

/* Combat actions. Defend is now a reaction, not a menu choice. */
typedef enum {
    ACTION_ATTACK = 1,
    ACTION_SPECIAL_SKILL = 2,
    ACTION_FLEE = 3
} CombatAction;

/* The three ways a combat encounter can end. */
typedef enum {
    OUTCOME_VICTORY,
    OUTCOME_DEFEAT,
    OUTCOME_ESCAPE
} CombatOutcome;

/* The twelve ordinary creatures. */
typedef enum {
    CREATURE_FEAR_DEARG,
    CREATURE_PUCA,
    CREATURE_CORPSE_LIGHT,
    CREATURE_BEAN_NIGHE,
    CREATURE_FETCH,
    CREATURE_REDCAP,
    CREATURE_CAILLEACH,
    CREATURE_DEARG_DUE,
    CREATURE_FACHAN,
    CREATURE_DULLAHAN,
    CREATURE_ABHARTACH,
    CREATURE_NUCKELAVEE,
    CREATURE_COUNT
} CreatureId;

/* The four Harkers, in the order they are defeated. */
typedef enum {
    HARKER_STORYTELLER,
    HARKER_BELL_RINGER,
    HARKER_ENKINDLED,
    HARKER_CROON,
    HARKER_COUNT
} HarkerId;

/*
 * A damage pool: `count` dice of `sides` sides.
 * Position 0 (leftmost) is always the Primary Die.
 */
typedef struct {
    int count;
    int sides;
} DamagePool;

typedef struct {
    char name[NAME_LEN];
    char title[NAME_LEN];
    HeroClass class_type;

    int hearts;
    int max_hearts;

    int strength;
    int agility;
    int wits;
    int defense;

    int xp;
    int level;

    char weapon_name[NAME_LEN];
    char armor_name[NAME_LEN];
    int weapon_bonus;
    int armor_bonus;

    char ability_name[NAME_LEN];
    char ability_text[DESC_LEN];
    char skill_name[NAME_LEN];
    char skill_text[DESC_LEN];

    /* Temporary combat state, cleared after every encounter. */
    int defending;
    int strong_counter;
    int evasive_step;
    int ambush_ready;
} Hero;

typedef struct {
    char name[NAME_LEN];
    char description[DESC_LEN];

    int hearts;
    int max_hearts;

    DamagePool damage_pool;
    int armor_rating;

    int level;
    int xp_reward;
    int is_boss;
} Monster;

typedef struct {
    int tile_index;
    TileType type;
    int section;
    char description[DESC_LEN];
} Tile;

/* One of the four narrative sections of the Coilsíde forest. */
typedef struct {
    char name[NAME_LEN];
    char atmosphere[TEXT_LEN];
    CreatureId creatures[CREATURES_PER_SECTION];
    HarkerId harker;
    char harker_taunt[TEXT_LEN];
    int harker_defeated;
} ForestSection;

/* One of the three technical game phases. */
typedef struct {
    int phase_number;
    char phase_name[NAME_LEN];
    char description[TEXT_LEN];
    char mission_description[DESC_LEN];
    int is_mission_completed;
    int scaling_percent;
    int first_section;
    int last_section;
    Tile board[MAX_TILES];
    int board_size;
} GamePhase;

typedef struct {
    Hero hero;
    GamePhase phases[PHASE_COUNT];
    ForestSection sections[SECTION_COUNT];
    int current_phase;
    int current_section;
    int position;
    int is_over;
    int has_won;
} Game;

#endif