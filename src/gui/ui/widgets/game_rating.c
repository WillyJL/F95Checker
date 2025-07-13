#include "../ui_i.h"

#include <globals.h>

void gui_ui_game_rating(Gui* gui, Game* game) {
    ImVec2 prev_pos = ImGui_GetCursorPos();
    ImGui_BeginGroup();
    for(u8 i = 0; i < 5; i++) {
        if(game->rating >= i + 1) {
            ImGui_TextUnformatted(mdi_star);
        } else if(game->rating <= i) {
            ImGui_TextUnformatted(mdi_star_outline);
        } else {
            ImGui_TextUnformatted(mdi_star_half_full);
        }
        ImGui_SameLineEx(0, 0);
    }
    ImGui_EndGroup();
    ImVec2 rating_size = ImGui_GetItemRectSize();

    ImGui_SetCursorPos(prev_pos);
    ImGui_InvisibleButton("###game_rating", rating_size, ImGuiButtonFlags_None);
    if(ImGui_IsItemActive()) {
        f32 left_x = ImGui_GetItemRectMin().x;
        f32 mouse_x = ImGui_GetMousePos().x;
        f32 rounded_doubled = roundf((mouse_x - left_x) * 10 / rating_size.x);
        game->rating = CLAMP(rounded_doubled, 10.0f, 0.0f) / 2;
    } else if(ImGui_IsItemDeactivated()) {
        db_save_game(db, game, GamesColumn_rating);
        gui->ui_state.need_game_index_rebuild = true;
    }
}
