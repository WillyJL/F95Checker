#include "../ui_i.h"

#include "games_list_i.h"

#include <globals.h>

void gui_ui_game_row(Gui* gui, Game* game) {
    // Base row height with a buttom to align the following text calls to center vertically
    ImGui_TableSetColumnIndex(GamesListColumn_Separator);
    ImGui_ButtonEx("", (ImVec2){FLT_MIN, 0});

    for(GamesListColumn col = GamesListColumn_min(); col <= GamesListColumn_max(); col++) {
        if(!gui->ui_state.columns_enabled[col]) continue;
        ImGui_TableSetColumnIndex(col);

        switch(col) {
        case GamesListColumn_COUNT:
        case GamesListColumn_ManualSort:
        case GamesListColumn_Version:
        case GamesListColumn_FinishedVersion:
        case GamesListColumn_InstalledVersion:
        case GamesListColumn_StatusAfterName:
        case GamesListColumn_Separator:
            // Ghost columns
            break;
        case GamesListColumn_LaunchButton:
            gui_ui_game_launch_button(gui, game);
            break;
        case GamesListColumn_Type:
            gui_ui_game_type(gui, game);
            break;
        case GamesListColumn_Name:
            if(settings->show_remove_btn) {
                gui_ui_game_remove_button(gui, game);
                ImGui_SameLine();
            }
            gui_ui_game_icons(gui, game);
            gui_ui_game_name(gui, game);
            break;
        case GamesListColumn_Developer:
            gui_ui_game_developer(gui, game);
            break;
        case GamesListColumn_LastUpdated:
            gui_ui_game_last_updated(gui, game);
            break;
        case GamesListColumn_LastLaunched:
            gui_ui_game_last_launched(gui, game);
            break;
        case GamesListColumn_AddedOn:
            gui_ui_game_added_on(gui, game);
            break;
        case GamesListColumn_FinishedCheckbox:
            gui_ui_game_finished_checkbox(gui, game);
            break;
        case GamesListColumn_InstalledCheckbox:
            gui_ui_game_installed_checkbox(gui, game);
            break;
        case GamesListColumn_Rating:
            gui_ui_game_rating(gui, game);
            break;
        case GamesListColumn_Notes:
            gui_ui_game_notes(gui, game);
            break;
        case GamesListColumn_OpenUrlButton:
            gui_ui_game_open_url_button(gui, game);
            break;
        case GamesListColumn_CopyUrlButton:
            gui_ui_game_copy_url_button(gui, game);
            break;
        case GamesListColumn_OpenFolderButton:
            gui_ui_game_open_folder_button(gui, game);
            break;
        case GamesListColumn_StatusOwnColumn:
            gui_ui_game_status(gui, game);
            break;
        case GamesListColumn_Score:
            gui_ui_game_score(gui, game);
            break;
        }
    }

    // FIXME: implement row hitbox and click events
}
