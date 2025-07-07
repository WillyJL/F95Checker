#include "../ui_i.h"

#include "games_list_i.h"

#include <globals.h>

static const ImGuiTableColumnFlags games_list_ghost_column_flags =
    ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_NoReorder |
    ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;

SMARTENUM_DEFINE(_GamesListColumn, GamesListColumn)
const GamesListColumnInfo games_list_column[GamesListColumn_COUNT] = {
    [GamesListColumn_ManualSort] =
        {
            .icon = mdi_cursor_move,
            .label = mdi_cursor_move " Manual Sort",
            .header = "",
            .flags = games_list_ghost_column_flags | ImGuiTableColumnFlags_DefaultHide,
            .ghost = true,
        },
    [GamesListColumn_Version] =
        {
            .icon = mdi_star_shooting,
            .label = mdi_star_shooting " Version",
            .header = "",
            .flags = games_list_ghost_column_flags,
            .ghost = true,
        },
    [GamesListColumn_FinishedVersion] =
        {
            .icon = mdi_flag_checkered,
            .label = mdi_flag_checkered " Finished Version",
            .header = "",
            .flags = games_list_ghost_column_flags | ImGuiTableColumnFlags_DefaultHide,
            .ghost = true,
        },
    [GamesListColumn_InstalledVersion] =
        {
            .icon = mdi_download,
            .label = mdi_download " Installed Version",
            .header = "",
            .flags = games_list_ghost_column_flags | ImGuiTableColumnFlags_DefaultHide,
            .ghost = true,
        },
    [GamesListColumn_StatusAfterName] =
        {
            .icon = mdi_checkbox_marked_circle,
            .label = mdi_checkbox_marked_circle " Status (after name)",
            .header = "",
            .flags = games_list_ghost_column_flags,
            .ghost = true,
        },
    [GamesListColumn_Separator] =
        {
            .icon = "",
            .label = "-----------------------------------",
            .header = "",
            .flags = games_list_ghost_column_flags | ImGuiTableColumnFlags_NoHide,
            .ghost = true,
        },
    [GamesListColumn_PlayButton] =
        {
            .icon = mdi_play,
            .label = mdi_play " Play Button",
            .header = "",
            .flags = ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_Type] =
        {
            .icon = mdi_book_information_variant,
            .label = mdi_book_information_variant " Type",
            .header = "Type",
            .flags = ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_Name] =
        {
            .icon = mdi_gamepad_variant,
            .label = mdi_gamepad_variant " Name",
            .header = "Name",
            .flags = ImGuiTableColumnFlags_NoHide,
        },
    [GamesListColumn_Developer] =
        {
            .icon = mdi_account,
            .label = mdi_account " Developer",
            .header = "Developer",
            .flags = ImGuiTableColumnFlags_DefaultHide,
        },
    [GamesListColumn_LastUpdated] =
        {
            .icon = mdi_update,
            .label = mdi_update " Last Updated",
            .header = "Last Updated",
            .flags = ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_LastLaunched] =
        {
            .icon = mdi_play,
            .label = mdi_play " Last Launched",
            .header = "Last Launched",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_AddedOn] =
        {
            .icon = mdi_plus,
            .label = mdi_plus " Added On",
            .header = "Added On",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_Finished] =
        {
            .icon = mdi_flag_checkered,
            .label = mdi_flag_checkered " Finished",
            .header = mdi_flag_checkered,
            .flags = ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_Installed] =
        {
            .icon = mdi_download,
            .label = mdi_download " Installed",
            .header = mdi_download,
            .flags = ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_Rating] =
        {
            .icon = mdi_star,
            .label = mdi_star " Rating",
            .header = "Rating",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_Notes] =
        {
            .icon = mdi_draw_pen,
            .label = mdi_draw_pen " Notes",
            .header = "Notes",
            .flags = ImGuiTableColumnFlags_DefaultHide,
        },
    [GamesListColumn_OpenThread] =
        {
            .icon = mdi_open_in_new,
            .label = mdi_open_in_new " Open Thread",
            .header = "",
            .flags = ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_CopyLink] =
        {
            .icon = mdi_content_copy,
            .label = mdi_content_copy " Copy Link",
            .header = "",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort |
                     ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_OpenFolder] =
        {
            .icon = mdi_folder_open_outline,
            .label = mdi_folder_open_outline " Open Folder",
            .header = "",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort |
                     ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_StatusOwnColumn] =
        {
            .icon = mdi_checkbox_marked_circle,
            .label = mdi_checkbox_marked_circle " Status (own column)",
            .header = "",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_ForumScore] =
        {
            .icon = mdi_message_star,
            .label = mdi_message_star " Forum Score",
            .header = mdi_message_star,
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoResize,
        },
};

static bool gui_ui_games_list_begin(Gui* gui, bool draw_header, bool can_reorder, f32 height) {
    m_string_t table_id;
    if(settings->independent_tab_views) {
        m_string_init_printf(
            table_id,
            "###game_list%d",
            gui->ui_state.current_tab == NULL ? -1 : gui->ui_state.current_tab->id);
    } else {
        m_string_init_set(table_id, "###game_list");
    }

    // HACK: custom toggles in table header right click menu by adding tiny empty "ghost" columns
    // and hiding them by starting the table render before the content region.
    f32 ghost_column_size = gui->style->FramePadding.x + gui->style->CellPadding.x * 2;
    f32 table_offset = ghost_column_size * gui->ui_state.ghost_columns_enabled_count;
    ImGui_SetCursorPosX(ImGui_GetCursorPosX() - table_offset);

    bool ret = ImGui_BeginTableEx(
        m_string_get_cstr(table_id),
        GamesListColumn_COUNT,
        ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBodyUntilResize |
            ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti,
        (ImVec2){0, height},
        0);
    if(ret) {
        // Setup columns
        u8 ghost_columns_enabled_count = 0;
        ImGuiTableColumnFlags extra_flags = can_reorder ? ImGuiTableColumnFlags_None :
                                                          ImGuiTableColumnFlags_NoReorder;
        for(GamesListColumn col = GamesListColumn_min(); col <= GamesListColumn_max(); col++) {
            const GamesListColumnInfo* column = &games_list_column[col];
            ImGui_TableSetupColumn(column->label, column->flags | extra_flags);
            const bool is_enabled = ImGui_TableGetColumnFlags(col) &
                                    ImGuiTableColumnFlags_IsEnabled;
            gui->ui_state.columns_enabled[col] = is_enabled;
            if(is_enabled && column->ghost) {
                ghost_columns_enabled_count += 1;
                if(col == GamesListColumn_ManualSort) {
                    // Manual sorting enabled, disable sorting for all other columns
                    extra_flags |= ImGuiTableColumnFlags_NoSort;
                }
            }
        }
        gui->ui_state.ghost_columns_enabled_count = ghost_columns_enabled_count;

        // FIXME: update game index
        // ImGui_TableGetSortSpecs();

        // Column headers
        if(draw_header) {
            ImGui_TableSetupScrollFreeze(0, 1);
            ImGui_TableNextRowEx(ImGuiTableRowFlags_Headers, 0);
            for(GamesListColumn col = GamesListColumn_min(); col <= GamesListColumn_max(); col++) {
                ImGui_TableSetColumnIndex(col);
                ImGui_TableHeader(games_list_column[col].header);
            }
        }
    }

    m_string_clear(table_id);
    return ret;
}

void gui_ui_games_list_tick_columns(Gui* gui, bool draw_header) {
    // HACK: get sort and column specs for list mode in grid and kanban mode
    const f32 prev_pos_y = ImGui_GetCursorPosY();
    const f32 header_height = draw_header ? ImGui_GetTextLineHeightWithSpacing() : FLT_MIN;
    if(gui_ui_games_list_begin(gui, draw_header, false, header_height)) {
        ImGui_EndTable();
    }
    if(!draw_header) {
        ImGui_SetCursorPosY(prev_pos_y);
    }
}

void gui_ui_games_list(Gui* gui) {
    const f32 bottom_bar_height = ImGui_GetFrameHeightWithSpacing();
    if(gui_ui_games_list_begin(gui, true, true, -bottom_bar_height)) {
        // FIXME: implement list view
        ImGui_EndTable();
    }
}
