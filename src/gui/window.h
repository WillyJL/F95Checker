#pragma once

#include "gui.h"

bool gui_window_init(Gui* gui);

void gui_window_show(Gui* gui);
void gui_window_hide(Gui* gui);

void gui_window_process_event(Gui* gui, SDL_Event* event);
void gui_window_new_frame(Gui* gui);
void gui_window_render(Gui* gui);

void gui_window_free(Gui* gui);
