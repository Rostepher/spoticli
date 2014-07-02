#include <stdlib.h>
#include <string.h>

#include "statusline.h"

void ui_statusline_init(ui_t *ui)
{
    ui->window = newwin(0, 0, 0, 0);
    ui->flags = 0;
    ui-> min_width = 0;
    ui-> min_height = 1;
}

void ui_statusline_draw()
{

}

void ui_statusline_release()
{

}
