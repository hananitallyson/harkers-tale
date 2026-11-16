/* ui.h - Terminal screens: 50-column boxes, input and pauses. */
#ifndef UI_H
#define UI_H

#include <stdio.h>
#include "lang.h"
#include "types.h"

#define SCREEN_WIDTH 50
#define TEXT_WIDTH 46

/* Prints a text pair (second argument ignored, English-only). */
#define SAY(english, portuguese) print_text(translate(english, portuguese))

/* Same as SAY, but the text is a printf format. */
#define SAYF(english, portuguese, ...)                                       \
    do {                                                                     \
        char say_buffer[TEXT_LEN];                                           \
        snprintf(say_buffer, sizeof say_buffer, translate(english, portuguese), \
                 __VA_ARGS__);                                               \
        print_text(say_buffer);                                              \
    } while (0)

void clear_screen(void);
void print_border(void);
void print_blank(void);
void print_text(const char *text);
void print_title(const char *title);
void print_row(const char *left_text, const char *right_text);

int read_integer(const char *prompt, int min, int max);
void wait_for_enter(void);

#endif