/* dice.h - Random number helpers. */
#ifndef DICE_H
#define DICE_H

int roll_d6(void);
int roll_d20(void);
int roll_percent(void);
int roll_range(int min, int max);

/* Rolls one die with the given number of sides (returns 1..sides). */
int roll_die(int sides);

#endif