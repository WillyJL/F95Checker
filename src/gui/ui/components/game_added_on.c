#include "../ui_i.h"

void gui_ui_game_added_on(Gui* gui, Game* game) {
    gui_ui_datestamp(gui, game->added_on, NULL);
}
