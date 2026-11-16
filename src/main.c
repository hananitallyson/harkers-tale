/* main.c - Entry point of Harker's Tale. */
#include <stdlib.h>
#include <time.h>
#include "game.h"

/*
 * main — Seeds the RNG and runs the game.
 */
int main(void)
{
    srand((unsigned int)time(NULL));
    start_game();
    return 0;
}