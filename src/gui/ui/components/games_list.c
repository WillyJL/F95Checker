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
    [GamesListColumn_LaunchButton] =
        {
            .icon = mdi_play,
            .label = mdi_play " Launch Button",
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
            .flags = ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch |
                     ImGuiTableColumnFlags_DefaultSort,
        },
    [GamesListColumn_Developer] =
        {
            .icon = mdi_account,
            .label = mdi_account " Developer",
            .header = "Developer",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_WidthStretch,
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
    [GamesListColumn_FinishedCheckbox] =
        {
            .icon = mdi_flag_checkered,
            .label = mdi_flag_checkered " Finished Checkbox",
            .header = mdi_flag_checkered,
            .flags = ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_InstalledCheckbox] =
        {
            .icon = mdi_download,
            .label = mdi_download " Installed Checkbox",
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
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_WidthStretch,
        },
    [GamesListColumn_OpenUrlButton] =
        {
            .icon = mdi_open_in_new,
            .label = mdi_open_in_new " Open URL Button",
            .header = "",
            .flags = ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_CopyUrlButton] =
        {
            .icon = mdi_content_copy,
            .label = mdi_content_copy " Copy URL Button",
            .header = "",
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort |
                     ImGuiTableColumnFlags_NoResize,
        },
    [GamesListColumn_OpenFolderButton] =
        {
            .icon = mdi_folder_open_outline,
            .label = mdi_folder_open_outline " Open Folder Button",
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
    [GamesListColumn_Score] =
        {
            .icon = mdi_message_star,
            .label = mdi_message_star " Forum Score",
            .header = mdi_message_star,
            .flags = ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoResize,
        },
};

static i32 placeholder_sort(Game* const* a, Game* const* b) {
    // FIXME: proper sorting
    return ((*a)->id < (*b)->id) ? -1 : ((*a)->id > (*b)->id);
}

static void gui_ui_games_list_rebuild_index(Gui* gui, ImGuiTableSortSpecs* sort_specs) {
    GameIndex game_index;
    game_index_init(game_index);

    GameArray game_array;
    game_array_init(game_array);

    for each(GameDict_pair, pair, GameDict, games) {
        Game* game = pair.value;
        if(game->tab == NULL) {
            game_array_push_back(game_array, game);
        }
    }
    game_array_special_sort(game_array, &placeholder_sort);
    game_index_set_at(game_index, TAB_ID_NULL, game_array);

    for each(Tab_ptr, tab, TabList, tabs) {
        game_array_reset(game_array);
        for each(GameDict_pair, pair, GameDict, games) {
            Game* game = pair.value;
            if(game->tab != NULL && game->tab->id == tab->id) {
                game_array_push_back(game_array, game);
            }
        }
        game_array_special_sort(game_array, &placeholder_sort);
        game_index_set_at(game_index, tab->id, game_array);
    }

    game_array_clear(game_array);

    // FIXME: actually sort and filter
    UNUSED(sort_specs);

    game_index_move(gui->ui_state.game_index, game_index);
}

static void gui_ui_games_list_tick_index(Gui* gui, ImGuiTableSortSpecs* sort_specs) {
    const bool is_manual_sort = gui->ui_state.columns_enabled[GamesListColumn_ManualSort];
    if(gui->ui_state.prev_manual_sort != is_manual_sort) {
        gui->ui_state.prev_manual_sort = is_manual_sort;
        gui->ui_state.need_game_index_rebuild = true;
    }

    // FIXME: check if filters changed and mark for game index rebuild

    if(sort_specs->SpecsDirty) {
        sort_specs->SpecsDirty = false;
        gui->ui_state.need_game_index_rebuild = true;
    }

    if(gui->ui_state.need_game_index_rebuild) {
        gui->ui_state.need_game_index_rebuild = false;
        gui_ui_games_list_rebuild_index(gui, sort_specs);
    }
}

static bool gui_ui_games_list_begin(Gui* gui, bool draw_header, bool can_reorder, f32 height) {
    m_string_t table_id;
    if(settings->independent_tab_views) {
        m_string_init_printf(
            table_id,
            "###game_list%d",
            gui->ui_state.current_tab == NULL ? TAB_ID_NULL : gui->ui_state.current_tab->id);
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

        // Column headers
        if(draw_header) {
            ImGui_TableSetupScrollFreeze(0, 1);
            ImGui_TableNextRowEx(ImGuiTableRowFlags_Headers, 0);
            for(GamesListColumn col = GamesListColumn_min(); col <= GamesListColumn_max(); col++) {
                ImGui_TableSetColumnIndex(col);
                ImGui_TableHeader(games_list_column[col].header);
            }
        }

        // Rebuild game index if necessary
        gui_ui_games_list_tick_index(gui, ImGui_TableGetSortSpecs());
    }

    m_string_clear(table_id);
    return ret;
}

static void gui_ui_games_list_draw_row(Gui* gui, Game* game) {
    // Base row height with a buttom to align the following text calls to center vertically
    ImGui_TableSetColumnIndex(GamesListColumn_Separator);
    ImGui_ButtonEx("", (ImVec2){FLT_MIN, 0});

    for(GamesListColumn col = GamesListColumn_min(); col <= GamesListColumn_max(); col++) {
        if(!gui->ui_state.columns_enabled[col]) continue;
        ImGui_TableSetColumnIndex(col);

        switch(col) {
        default:
            // FIXME: implement other columns
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

void gui_ui_games_list(Gui* gui) {
    const f32 bottom_bar_height = ImGui_GetFrameHeightWithSpacing();
    if(gui_ui_games_list_begin(gui, true, true, -bottom_bar_height)) {
        // FIXME: sync scroll with other view modes

        TabId tab_id = gui->ui_state.current_tab == NULL ? TAB_ID_NULL :
                                                           gui->ui_state.current_tab->id;
        GameArray_ptr game_array = *game_index_get(gui->ui_state.game_index, tab_id);

        ImGuiListClipper clipper = {0};
        ImGuiListClipper_Begin(
            &clipper,
            game_array_size(game_array),
            ImGui_GetFrameHeightWithSpacing());

        while(ImGuiListClipper_Step(&clipper)) {
            for(i32 clip_i = clipper.DisplayStart;
                clip_i < clipper.DisplayEnd && clip_i < (i32)game_array_size(game_array);
                clip_i++) {
                ImGui_TableNextRow();
                gui_ui_games_list_draw_row(gui, *game_array_get(game_array, clip_i));
            }
        }

        ImGui_EndTable();
    }
}

void gui_ui_games_list_tick_columns(Gui* gui, bool draw_header) {
    // HACK: get sort and column specs for list mode in grid and kanban mode
    const f32 prev_pos_y = ImGui_GetCursorPosY();
    const f32 header_height = draw_header ? ImGui_GetTextLineHeightWithSpacing() : -FLT_MIN;
    if(gui_ui_games_list_begin(gui, draw_header, false, header_height)) {
        ImGui_EndTable();
    }
    if(!draw_header) {
        ImGui_SetCursorPosY(prev_pos_y);
    }
}
