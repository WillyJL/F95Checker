#pragma once

#include "../gui.h"

f32 gui_ui_scaled(Gui* gui, f32 size);
bool gui_ui_is_topmost(Gui* gui);
bool gui_ui_should_close_weak_modal(Gui* gui);

void gui_ui_apply_styles(Gui* gui);
void gui_ui_draw(Gui* gui);
