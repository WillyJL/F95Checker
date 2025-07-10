#include "../ui_i.h"

void gui_ui_game_developer(Gui* gui, Game* game) {
    UNUSED(gui);

    ImGui_TextUnformatted(
        m_string_empty_p(game->developer) ? "Unknown" : m_string_get_cstr(game->developer));
}
