#ifndef _UI_H_
#define _UI_H_

#define _XOPEN_SOURCE_EXTENDED  // wchar_t support in ncurses
#include <wchar.h>
#include <stdbool.h>
#include <ncurses.h>

#define KEY_ESC 0x1b

typedef enum ui_flags {
    UI_FLAG_FOCUS = 1 << 0,
    UI_FLAG_DIRTY = 1 << 1
} ui_flags_t;

typedef struct ui_s {
    WINDOW *window;
    ui_flags_t flags;

    unsigned int width;
    unsigned int height;
    unsigned int min_width;     // min width for ui element
    unsigned int min_height;    // min height for ui element
} ui_t;

void stdscr_init();
void stdscr_release();

void ui_init();
void ui_release();

void ui_balance();
void ui_update(bool redraw);

#endif
