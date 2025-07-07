#pragma once

#include "gui.h"

#include <std.h>

bool gui_backend_init(Gui* gui);

void gui_backend_process_events(Gui* gui);
void gui_backend_wait_idle_time(Gui* gui);

void gui_backend_free(Gui* gui);
