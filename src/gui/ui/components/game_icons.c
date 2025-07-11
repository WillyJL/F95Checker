#include "../ui_i.h"

#include <globals.h>

void gui_ui_game_icons(Gui* gui, Game* game) {
    UNUSED(gui);

    if(game->archived) {
        ImGui_TextDisabled(mdi_archive);
        if(ImGui_IsItemClickedEx(ImGuiMouseButton_Middle) ||
           (ImGui_IsKeyDown(ImGuiKey_LeftAlt) && ImGui_IsItemClicked())) {
            game->archived = false;
            db_save_game(db, game, GamesColumn_archived);
            gui->ui_state.need_game_index_rebuild = true;
        } else if(settings->quick_filters && ImGui_IsItemClicked()) {
            // TODO: add archived filter
        }
        if(ImGui_BeginItemTooltip()) {
            ImGui_TextUnformatted("This game is archived!");
            ImGui_TextDisabled(
                "In this state you won't receive update notifications for\n"
                "this game and it will stay at the bottom of the list.");
            ImGui_TextUnformatted(
                "To unarchive it:\n"
                "" mdi_menu_right " Middle click\n"
                "" mdi_menu_right " Alt + Left click");
            ImGui_EndTooltip();
        }
        ImGui_SameLine();

    } else if(game->updated) {
        ImGui_TextColored((ImColor4){0.85f, 0.85f, 0.00f, 1}, mdi_star_circle);
        if(ImGui_IsItemClickedEx(ImGuiMouseButton_Middle) ||
           (ImGui_IsKeyDown(ImGuiKey_LeftAlt) && ImGui_IsItemClicked())) {
            game->updated = false;
            db_save_game(db, game, GamesColumn_updated);
            gui->ui_state.need_game_index_rebuild = true;
        } else if(settings->quick_filters && ImGui_IsItemClicked()) {
            // TODO: add updated filter
        }
        if(ImGui_BeginItemTooltip()) {
            ImGui_TextUnformatted("This game has been updated!");
            ImGui_TextDisabled("Installed:");
            ImGui_SameLine();
            ImGui_TextUnformatted(
                m_string_empty_p(game->installed) ? "N/A" : m_string_get_cstr(game->installed));
            ImGui_TextDisabled("Latest:");
            ImGui_SameLine();
            ImGui_TextUnformatted(m_string_get_cstr(game->version));
            ImGui_TextUnformatted(
                "To remove this update marker:\n"
                "" mdi_menu_right " Middle click\n"
                "" mdi_menu_right " Alt + Left click\n"
                "" mdi_menu_right " Mark game as installed");
            ImGui_EndTooltip();
        }
        ImGui_SameLine();
    }

    if(game->unknown_tags_flag) {
        ImGui_TextColored((ImColor4){1.00f, 0.65f, 0.00f, 1}, mdi_progress_tag);
        if(ImGui_IsItemClickedEx(ImGuiMouseButton_Middle) ||
           (ImGui_IsKeyDown(ImGuiKey_LeftAlt) && ImGui_IsItemClicked())) {
            game->unknown_tags_flag = false;
            db_save_game(db, game, GamesColumn_unknown_tags_flag);
            gui->ui_state.need_game_index_rebuild = true;
        } else if(ImGui_IsKeyDown(ImGuiKey_LeftShift) && ImGui_IsItemClicked()) {
            m_string_t unknown_tags;
            m_string_init(unknown_tags);
            for each(m_string_ptr, unknown_tag, m_string_list_t, game->unknown_tags) {
                if(!m_string_empty_p(unknown_tags)) {
                    m_string_cat(unknown_tags, ", ");
                }
                m_string_cat(unknown_tags, unknown_tag);
            }
            ImGui_SetClipboardText(m_string_get_cstr(unknown_tags));
            m_string_clear(unknown_tags);
        } else if(settings->quick_filters && ImGui_IsItemClicked()) {
            // TODO: add unknown tags filter
        }
        if(ImGui_BeginItemTooltip()) {
            ImGui_PushStyleVarImVec2(ImGuiStyleVar_ItemSpacing, (ImVec2){0, 0});
            ImGui_TextUnformatted("This game has new tags F95Checker did not recognize:");
            for each(m_string_ptr, unknown_tag, m_string_list_t, game->unknown_tags) {
                ImGui_TextDisabled(" - %s", m_string_get_cstr(unknown_tag));
            }
            ImGui_PopStyleVar();
            ImGui_Spacing();
            ImGui_TextUnformatted(
                "To copy these new tags:\n"
                "" mdi_menu_right " Shift + Left click\n"
                "" mdi_menu_right " Use Copy button in Tags section");
            ImGui_TextUnformatted(
                "To hide this unknown tags marker:\n"
                "" mdi_menu_right " Middle click\n"
                "" mdi_menu_right " Alt + Left click");
            ImGui_EndTooltip();
        }
        ImGui_SameLine();
    }

    if(!m_string_empty_p(game->notes)) {
        ImGui_TextColored((ImColor4){0.85f, 0.20f, 0.85f, 1}, mdi_draw_pen);
        if(ImGui_BeginItemTooltip()) {
            ImGui_TextDisabled("Notes:");
            ImGui_TextUnformatted(m_string_get_cstr(game->notes));
            ImGui_EndTooltip();
        }
        // TODO: quick filter for this too?
        ImGui_SameLine();
    }
}
