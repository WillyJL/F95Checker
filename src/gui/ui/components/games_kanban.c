#include "../ui_i.h"

#include "games_list_i.h"

#include <globals.h>

void gui_ui_games_kanban(Gui* gui) {
    gui_ui_games_list_tick_columns(gui, settings->table_header_outside_list);
    // FIXME: implement kanban view
}
