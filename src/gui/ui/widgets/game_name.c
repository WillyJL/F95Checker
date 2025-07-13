#include "../ui_i.h"

#include <globals.h>

void gui_ui_game_name(Gui* gui, Game* game) {
    UNUSED(gui);

    if(game->archived) {
        ImGui_TextDisabled("%s", m_string_get_cstr(game->name));
    } else if(m_string_equal_p(game->finished, game->version)) {
        ImGui_TextUnformatted(m_string_get_cstr(game->name));
    } else {
        ImGui_TextColored(settings->style_accent, "%s", m_string_get_cstr(game->name));
    }

    if(ImGui_IsItemClickedEx(ImGuiMouseButton_Middle)) {
        ImGui_SetClipboardText(m_string_get_cstr(game->name));
    }
}
