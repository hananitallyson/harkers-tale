<p align="center">
  <img src="HARKERS%20TALE%20LOGO.jpg" alt="Harker's Tale" />
</p>

## About

**Harker's Tale** is a terminal-based RPG written in C. It combines exploration, turn-based combat, character progression, and encounters with dangerous creatures.

The game takes place in a dark fantasy world where every journey brings new dangers. Explore the map, fight monsters, collect equipment, develop your character, and face the Harkers.

## Story

In the land of Caer Seilbhe, a small settlement lies beside the mysterious region of Coilsíde.

When the sinister creature known as **The Croon** enables the Harkers to abduct children from the settlement, a six-year-old girl named **Caoimhe** is taken.

Her twelve-year-old sibling — **Bran, Siofra, or Donn** — sets out into Coilsíde to find her.

The journey leads through four dangerous sections, each ruled by one of the Harkers:

- **The Storyteller**
- **The Bell-Ringer**
- **The Enkindled**
- **The Croon**

To rescue Caoimhe, the hero must survive the creatures of Coilsíde and defeat all four Harkers.

## Getting Started

### Requirements

- GCC

### Compilation

Clone or download the repository and navigate to its root directory.

Compile the game using:

```bash
gcc -std=c99 -Wall -Wextra -pedantic src/*.c -o game
```

### Running

Start the game with:

```bash
./game
```

## How to Play

### Exploration

Explore the map by rolling a d6. Each movement takes you to a tile that may contain a different event:

- **Empty:** Nothing happens.
- **Combat:** Fight a monster.
- **Treasure:** Find a weapon (+1 damage) or armor (+1 AR).
- **Heal:** Restore half your maximum Hearts.
- **Trap:** Take damage based on the current section and your Wits. Traps can never kill you.
- **Mission:** Face the Harker of the current section.

### Combat

Combat is turn-based and uses Damage Pools instead of separate attack rolls. Roll all the dice assigned to your attack.

#### Attacking

The leftmost die is the **Primary Die** and determines the attack's outcome:

- **1:** Miss. Deal no damage.
- **Maximum result:** Critical Hit. Roll the Primary Die again and add the result to your damage. Keep rolling while you get the maximum result.
- **Any other result:** Normal hit.

All other dice contribute to damage but cannot trigger misses or Critical Hits.

**Critical Hits ignore armor**, bypassing the target's Armor Rating.

#### Advantage and Disadvantage

- **Advantage:** Roll an extra die for each level and discard the lowest result.
- **Disadvantage:** Roll an extra die for each level and discard the highest result.
- Advantage and Disadvantage cancel each other out one-for-one.

#### Armor

Armor reduces incoming damage by its Armor Rating (AR). Subtract the AR from the incoming damage. Damage cannot go below 0.

Heavier armor provides greater protection.

Critical Hits ignore this reduction.

#### Defend

After a monster attacks and its damage is determined, you may choose to Defend.

- Halves the incoming damage.
- Costs your next action.

#### Flee

Attempt to escape by rolling:

```text
d20 + Agility >= 12
```

You cannot flee from a Harker.

### Character Abilities

Each playable character has unique abilities.

**Bran**
- **Counterattack:** Defending triggers a counterattack against the enemy.
- **Unyielding Stand:** His next counterattack deals double damage.

**Siofra**
- **Ambush:** Her first attack in combat has Advantage.
- **Lie in Wait:** Re-arms Ambush for another use.

**Donn**
- **Evasive Step:** The next incoming attack against him has Disadvantage.

### Experience and Leveling

Defeating monsters grants XP and increases one random attribute by 1.

You gain a level whenever your XP reaches:

```text
25 × level²
```

Each level grants:

- +5 maximum Hearts.
- Full Hearts restoration.
- +1 to every attribute.

### Progression and Victory

The adventure takes place across four sections of Coilsíde. Each section culminates in a battle against one of the Harkers.

Defeat all four Harkers, including The Croon, to complete the adventure and rescue Caoimhe.

### Death

If your Hearts reach 0, your adventure ends.

## How the Code Works

Harker's Tale is written in C and organized into separate source and header files. Each part of the code is responsible for a specific aspect of the game.

The main systems work together as follows:

- **Game:** Controls the main game flow, initializes the adventure, and manages progression.
- **Board:** Handles map exploration, movement, and tile events.
- **Hero:** Stores the player's attributes, Hearts, equipment, experience, and abilities.
- **Monster:** Handles enemy data, attributes, and combat encounters.
- **Combat:** Resolves attacks, damage, armor, defending, and fleeing.
- **Dice:** Provides the dice-rolling mechanics used throughout the game.
- **UI:** Handles terminal output and interaction with the player.

The game begins by initializing the player and the world. The exploration system determines which event occurs on each tile. Combat is handled separately, using the dice system to resolve attacks and the character systems to update Hearts, attributes, and progression.

The game continues until the player completes the adventure or their Hearts reach 0.
