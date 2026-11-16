/* dice.c - Random number helpers. */
#include <stdlib.h>
#include "dice.h"

int roll_d6(void)
{
    return rand() % 6 + 1;
}

int roll_d20(void)
{
    return rand() % 20 + 1;
}

int roll_percent(void)
{
    return rand() % 100;
}

int roll_range(int min, int max)
{
    return min + rand() % (max - min + 1);
}

/*
 * roll_die — Returns a random value from 1 to sides.
 *
 * @param sides Number of sides (clamped to >= 2).
 * @return      Random value in [1, sides].
 */
int roll_die(int sides)
{
    if (sides < 2) {
        sides = 2;
    }
    return rand() % sides + 1;
}