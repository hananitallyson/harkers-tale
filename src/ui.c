/* ui.c - Terminal screens: 50-column boxes, input and pauses. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"

#define INPUT_LEN 64
#define ROW_LEFT 24
#define ROW_RIGHT 19

static int is_utf8_continuation(unsigned char byte)
{
    return (byte & 0xC0) == 0x80;
}

static int count_characters(const char *text, int bytes)
{
    int count = 0;
    int i;

    for (i = 0; i < bytes; i++) {
        if (!is_utf8_continuation((unsigned char)text[i])) {
            count++;
        }
    }
    return count;
}

static int bytes_for_characters(const char *text, int max_chars)
{
    int bytes = 0;
    int characters = 0;

    while (text[bytes] != '\0') {
        if (!is_utf8_continuation((unsigned char)text[bytes])) {
            if (characters == max_chars) {
                break;
            }
            characters++;
        }
        bytes++;
    }
    return bytes;
}

void clear_screen(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_border(void)
{
    int i;

    putchar('+');
    for (i = 0; i < SCREEN_WIDTH - 2; i++) {
        putchar('-');
    }
    printf("+\n");
}

void print_blank(void)
{
    printf("| %*s |\n", TEXT_WIDTH, "");
}

static void print_box_line(const char *text, int bytes)
{
    printf("| %.*s%*s |\n", bytes, text, TEXT_WIDTH - count_characters(text, bytes), "");
}

static void print_wrapped_paragraph(const char *text, int length)
{
    int position = 0;
    int i;
    int character_count;
    int last_space_position;
    int line_end;
    unsigned char current_character;

    while (position < length) {
        while (position < length && text[position] == ' ') {
            position++;
        }
        if (position >= length) {
            break;
        }

        i = position;
        character_count = 0;
        last_space_position = -1;
        while (i < length) {
            current_character = (unsigned char)text[i];
            if (!is_utf8_continuation(current_character)) {
                if (character_count == TEXT_WIDTH) {
                    break;
                }
                if (current_character == ' ') {
                    last_space_position = i;
                }
                character_count++;
            }
            i++;
        }

        if (i >= length) {
            line_end = length;
        } else if (text[i] == ' ') {
            line_end = i;
        } else if (last_space_position > position) {
            line_end = last_space_position;
        } else {
            line_end = i;
        }
        print_box_line(text + position, line_end - position);
        position = line_end;
    }
}

void print_text(const char *text)
{
    const char *start = text;
    const char *newline;

    if (*text == '\0') {
        print_blank();
        return;
    }
    while (*start != '\0') {
        newline = strchr(start, '\n');
        if (newline == NULL) {
            print_wrapped_paragraph(start, (int)strlen(start));
            break;
        }
        if (newline == start) {
            print_blank();
        } else {
            print_wrapped_paragraph(start, (int)(newline - start));
        }
        start = newline + 1;
    }
}

void print_title(const char *title)
{
    int title_bytes = (int)strlen(title);
    int title_characters = count_characters(title, title_bytes);
    int left_padding;

    if (title_characters > TEXT_WIDTH) {
        print_text(title);
        return;
    }
    left_padding = (TEXT_WIDTH - title_characters) / 2;
    printf("| %*s%s%*s |\n", left_padding, "", title,
           TEXT_WIDTH - title_characters - left_padding, "");
}

void print_row(const char *left_text, const char *right_text)
{
    int left_bytes = bytes_for_characters(left_text, ROW_LEFT);
    int right_bytes = bytes_for_characters(right_text, ROW_RIGHT);

    printf("| %.*s%*s | %.*s%*s |\n",
           left_bytes, left_text, ROW_LEFT - count_characters(left_text, left_bytes), "",
           right_bytes, right_text, ROW_RIGHT - count_characters(right_text, right_bytes), "");
}

static void clear_input_buffer(void)
{
    int character;

    do {
        character = getchar();
    } while (character != '\n' && character != EOF);
}

static void handle_closed_input(void)
{
    printf("\n%s\n", translate("Input closed. Goodbye.", ""));
    exit(EXIT_SUCCESS);
}

int read_integer(const char *prompt, int min, int max)
{
    char input_line[INPUT_LEN];
    char *parse_end;
    long parsed_number;
    int input_too_long;

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(input_line, sizeof input_line, stdin) == NULL) {
            handle_closed_input();
        }

        input_too_long = (strchr(input_line, '\n') == NULL
                          && strlen(input_line) == sizeof input_line - 1);
        if (input_too_long) {
            clear_input_buffer();
        }

        parsed_number = strtol(input_line, &parse_end, 10);
        while (*parse_end == ' ' || *parse_end == '\t') {
            parse_end++;
        }

        if (input_too_long || parse_end == input_line
            || (*parse_end != '\n' && *parse_end != '\0')
            || parsed_number < min || parsed_number > max) {
            printf(translate("Invalid. Enter a number from %d to %d.\n", ""),
                   min, max);
        } else {
            return (int)parsed_number;
        }
    }
}

void wait_for_enter(void)
{
    char input_line[INPUT_LEN];

    printf("%s", translate("[Press Enter]", ""));
    fflush(stdout);

    if (fgets(input_line, sizeof input_line, stdin) == NULL) {
        handle_closed_input();
    }
    if (strchr(input_line, '\n') == NULL && strlen(input_line) == sizeof input_line - 1) {
        clear_input_buffer();
    }
}