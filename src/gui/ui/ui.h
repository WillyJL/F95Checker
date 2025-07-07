#pragma once

#include "../gui.h"

#include <std.h>

void gui_ui_init(Gui* gui);

void gui_ui_apply_styles(Gui* gui);
void gui_ui_draw(Gui* gui);

void gui_ui_free(Gui* gui);
