#pragma once

#include "components/components.h" // IWYU pragma: export
#include "widgets/widgets.h" // IWYU pragma: export

#include <std.h>

f32 gui_ui_size(Gui* gui, f32 size);
ImColor4 gui_ui_color_alpha(Gui* gui, ImColor4 color, f32 alpha);
ImColor4 gui_ui_color_text(Gui* gui, ImColor4 background_color);
bool gui_ui_is_topmost(Gui* gui);
bool gui_ui_should_close_weak_modal(Gui* gui);
