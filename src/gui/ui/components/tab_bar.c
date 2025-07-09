#include "../ui_i.h"

#include "games_list_i.h"

#include <globals.h>

void gui_ui_tab_bar(Gui* gui) {
    UNUSED(gui);
    // Don't show tab bar
    if(tab_list_empty_p(tabs) || (settings->filter_all_tabs && gui->ui_state.is_filtering)) {
        return;
    }

    bool select_display_tab = gui->ui_state.current_tab != settings->display_tab;
    bool save_selected_tab = true;
    if(gui->ui_state.dragging_tab != NULL) {
        // Avoid ImGui interfering with forced reordering while dragging
        select_display_tab = true;
        save_selected_tab = false;
    } else if(game_index_empty_p(gui->ui_state.game_index)) {
        // First draw since launching, need to update index early
        // Also ImGui doesn't know yet what tab is selected
        gui->ui_state.current_tab = settings->display_tab;
        gui_ui_games_list_tick_columns(gui, false);
        select_display_tab = true;
        save_selected_tab = false;
    }

    Tab_ptr selected_tab = NULL;
    m_string_t str;
    m_string_init(str);

    m_string_init_set(str, "###tabbar");
    for each(Tab_ptr, tab, TabList, tabs) {
        // ID string changes with tab ordering, forces ImGui to reorder manually when dragging
        m_string_cat_printf(str, "_%d", tab->id);
    }
    if(ImGui_BeginTabBar(m_string_get_cstr(str), ImGuiTabBarFlags_FittingPolicyScroll)) {
        size_t games_count;

        // First (Default/New) tab
        games_count = game_array_size(*game_index_get(gui->ui_state.game_index, TAB_ID_NULL));
        if(games_count != 0 || !settings->hide_empty_tabs) {
            m_string_printf(
                str,
                "%s (%d)###tab_" XSTR(TAB_ID_NULL),
                settings->default_tab_is_new ? mdi_alert_decagram " New" :
                                               mdi_heart_box " Default",
                games_count);
            if(ImGui_BeginTabItem(
                   m_string_get_cstr(str),
                   NULL,
                   (select_display_tab && settings->display_tab == NULL) ?
                       ImGuiTabItemFlags_SetSelected :
                       ImGuiTabItemFlags_None)) {
                selected_tab = NULL;
                ImGui_EndTabItem();
            }
        }

        // Custom tabs
        TabList_it swap_tabs[2] = {};
        TabList_it prev_tab_it;
        for each(Tab_ptr, tab, TabList, tabs) {
            games_count = game_array_size(*game_index_get(gui->ui_state.game_index, tab->id));
            if(games_count == 0 && settings->hide_empty_tabs) {
                continue;
            }

            // Draw the tab
            if(tab->color.w != 0) {
                ImGui_PushStyleColor4(ImGuiCol_Tab, gui_ui_color_alpha(gui, tab->color, 0.5f));
                ImGui_PushStyleColor4(ImGuiCol_TabActive, tab->color);
                ImGui_PushStyleColor4(ImGuiCol_TabHovered, tab->color);
                ImGui_PushStyleColor4(ImGuiCol_Text, gui_ui_color_text(gui, tab->color));
            }
            m_string_printf(
                str,
                "%s %s (%d)###tab_%d",
                m_string_get_cstr(tab->icon),
                m_string_get_cstr(tab->name),
                games_count,
                tab->id);
            if(ImGui_BeginTabItem(
                   m_string_get_cstr(str),
                   NULL,
                   (select_display_tab && settings->display_tab == tab) ?
                       ImGuiTabItemFlags_SetSelected :
                       ImGuiTabItemFlags_None)) {
                selected_tab = tab;
                ImGui_EndTabItem();
            }
            if(tab->color.w != 0) {
                ImGui_PopStyleColorEx(4);
            }

            // Drag reordering
            if(gui->ui_state.dragging_tab == tab && ImGui_IsMouseReleased(ImGuiMouseButton_Left)) {
                gui->ui_state.dragging_tab = NULL;
            } else if(ImGui_IsItemActive() || gui->ui_state.dragging_tab == tab) {
                f32 mouse_x = ImGui_GetMousePos().x;
                f32 tab_left_x = ImGui_GetItemRectMin().x;
                f32 tab_right_x = ImGui_GetItemRectMax().x;
                if(tab != *tab_list_front(tabs) && tab_left_x > 0 && mouse_x < tab_left_x) {
                    if(ImGui_GetMouseDragDelta(ImGuiMouseButton_Left, -1).x < 0) {
                        gui->ui_state.dragging_tab = tab;
                        *swap_tabs[0] = *prev_tab_it;
                        *swap_tabs[1] = *tab_it;
                    }
                    ImGui_ResetMouseDragDelta();
                } else if(tab != *tab_list_back(tabs) && tab_right_x > 0 && mouse_x > tab_right_x) {
                    if(ImGui_GetMouseDragDelta(ImGuiMouseButton_Left, -1).x > 0) {
                        gui->ui_state.dragging_tab = tab;
                        TabList_it next_tab_it = {*tab_it};
                        tab_list_next(next_tab_it);
                        *swap_tabs[0] = *tab_it;
                        *swap_tabs[1] = *next_tab_it;
                    }
                    ImGui_ResetMouseDragDelta();
                }
            }

            // Context menu
            bool set_name_focus = ImGui_IsPopupOpen(m_string_get_cstr(str), ImGuiPopupFlags_None);
            if(ImGui_BeginPopupContextItem()) {
                // ImGui_SetNextItemWidth(ImGui_GetContentRegionAvail().x);
                if(set_name_focus) {
                    ImGui_SetKeyboardFocusHere();
                }
                gui_ui_input_text(gui, "###fixme", tab->name, ImGuiInputTextFlags_None);
                ImGui_EndPopup();
            }

            *prev_tab_it = *tab_it;
        }

        // Swap tabs if they were reordered by dragging
        if((*swap_tabs[0]).current != NULL && (*swap_tabs[1]).current != NULL) {
            tab_list_splice_at(tabs, swap_tabs[1], tabs, swap_tabs[0]);
            tab_list_update_positions(tabs);
        }

        ImGui_EndTabBar();
    }

    m_string_clear(str);

    // Save the currently selected tab
    if(save_selected_tab && selected_tab != gui->ui_state.current_tab) {
        // FIXME: deselect games
        gui->ui_state.current_tab = selected_tab;
        gui->ui_state.need_game_index_rebuild = true;
        settings->display_tab = selected_tab;
        db_save_setting(db, settings, SettingsColumn_display_tab);
    }
}
