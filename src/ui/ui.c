#include "../spotify/session.h"
#include "ui.h"

#define UI_COLORS 8

static bool g_stdscr_initialized = false;
static short g_colors[UI_COLORS][3];

extern sp_session *g_session;

void stdscr_init()
{
    if (g_stdscr_initialized)
        return;

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    g_stdscr_initialized = true;
}

void stdscr_release()
{
    if (!g_stdscr_initialized)
        return;

    noraw();
    endwin();

    g_stdscr_initialized = false;
}

void ui_init()
{
    //stdscr_init();
}

void ui_release()
{
    //stdscr_release();
}

void ui_balance()
{

}

void ui_update(bool redraw)
{
    if (redraw)
        redrawwin(stdscr);
}
