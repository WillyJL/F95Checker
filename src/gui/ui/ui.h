#pragma once

#include "../gui.h"

void gui_ui_init(Gui* gui);

f32 gui_ui_size(Gui* gui, f32 size);
ImColor gui_ui_color_alpha(Gui* gui, ImColor color, f32 alpha);
ImColor gui_ui_color_text(Gui* gui, ImColor background_color);
bool gui_ui_is_topmost(Gui* gui);
bool gui_ui_should_close_weak_modal(Gui* gui);

void gui_ui_apply_styles(Gui* gui);
void gui_ui_draw(Gui* gui);

void gui_ui_free(Gui* gui);
