#ifndef SPOTICLI_UI_H
#define SPOTICLI_UI_H

#include <stdbool.h>
#include <ncurses.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define KEY_ESC 0x1b

typedef enum ui_flags {
    UI_FLAG_FOCUS = 1 << 0,
    UI_FLAG_DIRTY = 1 << 1
} ui_flags_t;

typedef enum ui_elem {
    UI_STATUSLINE = 0,
    UI_SIDEBAR,
    UI_PLAYER,
    UI_END
} ui_elem_t;

struct ui_s;
typedef void (*ui_draw_cb_t)(struct *ui_s);

typedef struct ui_s {
    WINDOW *window;
    ui_flags_t flags;

    unsigned int width;
    unsigned int height;
    unsigned int min_width;     // min width for ui element
    unsigned int min_height;    // min height for ui element

    ui_draw_cb_t ui_draw_cb;
} ui_t;

void stdscr_init();
void stdscr_release();

void ui_init();
void ui_release();

void ui_balance();
void ui_update(bool redraw);

#endif
