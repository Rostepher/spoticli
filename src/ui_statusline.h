#ifndef SPOTICLI_UI_STATUSLINE_H
#define SPOTICLI_UI_STATUSLINE_H

#include "ui.h"

typedef enum input_type {
    INPUT_NONE,
    INUPT_COMMAND,
    INPUT_SEARCH,
    INPUT_USERNAME,
    INPUT_PASSWORD
} input_type_t;

void ui_statusline_init();
void ui_statusline_draw();
void ui_statusline_release();

#endif
