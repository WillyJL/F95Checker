#include "../ui_i.h"

void gui_ui_game_last_launched(Gui* gui, Game* game) {
    gui_ui_datestamp(gui, game->last_launched, "Never");
}
