#include "ui.h"

#include "../window.h"
#include "components/components.h"
#include "widgets/widgets.h"

#include <globals.h>

void gui_ui_init(Gui* gui) {
    gui->ui_state.current_tab = NULL;
    gui->ui_state.dragging_tab = NULL;
    // FIXME: init sorted+filtered games
}

f32 gui_ui_size(Gui* gui, f32 size) {
    UNUSED(gui);

    return size * settings->interface_scaling;
}

ImColor gui_ui_color_alpha(Gui* gui, ImColor color, f32 alpha) {
    UNUSED(gui);

    ImColor color_alpha = color;
    color_alpha.Value.w = alpha;
    return color_alpha;
}

ImColor gui_ui_color_text(Gui* gui, ImColor background_color) {
    UNUSED(gui);

    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    static const f32 gamma = 2.2f;
    static const f32 threshold = 0.2176376f; // powf(0.5f, gamma)
    const f32 luma = 0.2126f * powf(background_color.Value.x, gamma) +
                     0.7152f * powf(background_color.Value.y, gamma) +
                     0.0722f * powf(background_color.Value.z, gamma);
    return luma > threshold ? (ImColor){{0.04f, 0.04f, 0.04f, 1.0f}} :
                              (ImColor){{1.0f, 1.0f, 1.0f, 1.0f}};
}

bool gui_ui_is_topmost(Gui* gui) {
    UNUSED(gui);

    return !ImGui_IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
}

bool gui_ui_should_close_weak_modal(Gui* gui) {
    if(!gui_ui_is_topmost(gui)) {
        return false;
    }
    if(ImGui_Shortcut(ImGuiKey_Escape, ImGuiInputFlags_None)) {
        return true;
    }
    if(ImGui_IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 window_pos = ImGui_GetWindowPos();
        ImVec2 window_size = ImGui_GetWindowSize();
        if(!ImGui_IsMouseHoveringRectEx(
               window_pos,
               (ImVec2){window_pos.x + window_size.x, window_pos.y + window_size.y},
               false)) {
            return true;
        }
    }
    return false;
}

void gui_ui_apply_styles(Gui* gui) {
    gui->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    gui->io->ConfigDragClickToInputText = true;
    gui->io->ConfigScrollbarScrollByPage = false;

    gui->style->ItemSpacing.x = gui->style->ItemSpacing.y;
    gui->style->ScrollbarSize = gui_ui_size(gui, 10.0f);
    gui->style->FrameBorderSize = gui_ui_size(gui, 1.0f);
    gui->style->Colors[ImGuiCol_ModalWindowDimBg] = (ImVec4){0.0f, 0.0f, 0.0f, 0.5f};
    gui->style->Colors[ImGuiCol_TableBorderStrong] = (ImVec4){0.0f, 0.0f, 0.0f, 0.0f};

    // clang-format off
        gui->style->Colors[ImGuiCol_ButtonActive] =
        gui->style->Colors[ImGuiCol_ButtonHovered] =
        gui->style->Colors[ImGuiCol_CheckMark] =
        gui->style->Colors[ImGuiCol_FrameBgActive] =
        gui->style->Colors[ImGuiCol_HeaderActive] =
        gui->style->Colors[ImGuiCol_HeaderHovered] =
        gui->style->Colors[ImGuiCol_NavHighlight] =
        gui->style->Colors[ImGuiCol_PlotHistogram] =
        gui->style->Colors[ImGuiCol_ResizeGripActive] =
        gui->style->Colors[ImGuiCol_ResizeGripHovered] =
        gui->style->Colors[ImGuiCol_ScrollbarGrabActive] =
        gui->style->Colors[ImGuiCol_SeparatorActive] =
        gui->style->Colors[ImGuiCol_SeparatorHovered] =
        gui->style->Colors[ImGuiCol_SliderGrab] =
        gui->style->Colors[ImGuiCol_TabActive] =
        gui->style->Colors[ImGuiCol_TabHovered] =
        gui->style->Colors[ImGuiCol_TabUnfocusedActive] =
        gui->style->Colors[ImGuiCol_TextSelectedBg] =
        gui->style->Colors[ImGuiCol_TitleBgActive] =
    settings->style_accent.Value;
    // clang-format on

    // clang-format off
        gui->style->Colors[ImGuiCol_FrameBgHovered] =
        gui->style->Colors[ImGuiCol_ResizeGrip] =
        gui->style->Colors[ImGuiCol_Tab] =
        gui->style->Colors[ImGuiCol_TabUnfocused] =
    gui_ui_color_alpha(gui, settings->style_accent, 0.25f).Value;
    // clang-format on

    // clang-format off
        gui->style->Colors[ImGuiCol_TableHeaderBg] =
        gui->style->Colors[ImGuiCol_TableRowBgAlt] =
    settings->style_alt_bg.Value;
    // clang-format on

    // clang-format off
        gui->style->Colors[ImGuiCol_Button] =
        gui->style->Colors[ImGuiCol_ChildBg] =
        gui->style->Colors[ImGuiCol_FrameBg] =
        gui->style->Colors[ImGuiCol_Header] =
        gui->style->Colors[ImGuiCol_PopupBg] =
        gui->style->Colors[ImGuiCol_ScrollbarBg] =
        gui->style->Colors[ImGuiCol_SliderGrabActive] =
        gui->style->Colors[ImGuiCol_TitleBg] =
        gui->style->Colors[ImGuiCol_WindowBg] =
    settings->style_bg.Value;
    // clang-format on

    // clang-format off
        gui->style->Colors[ImGuiCol_Border] =
        gui->style->Colors[ImGuiCol_Separator] =
    settings->style_border.Value;
    // clang-format on

    // clang-format off
        gui->style->ChildRounding =
        gui->style->FrameRounding =
        gui->style->GrabRounding =
        gui->style->PopupRounding =
        gui->style->ScrollbarRounding =
        gui->style->TabRounding =
        gui->style->WindowRounding =
    gui_ui_size(gui, (f32)settings->style_corner_radius);
    // clang-format on

    // clang-format off
        gui->style->Colors[ImGuiCol_Text] =
    settings->style_text.Value;
    // clang-format on

    // clang-format off
        gui->style->Colors[ImGuiCol_TextDisabled] =
    settings->style_text_dim.Value;
    // clang-format on
}

void gui_ui_draw(Gui* gui) {
    const f32 sidebar_width = gui_ui_size(gui, 250.0f);

    // Main pane

    ImGui_BeginChild(
        "###main_frame",
        (ImVec2){-sidebar_width, 0.0f},
        ImGuiChildFlags_None,
        ImGuiWindowFlags_None);

    gui_ui_tab_bar(gui);

    switch(settings->display_mode) {
    default:
    case DisplayMode_List:
        gui_ui_games_list(gui);
        break;
    case DisplayMode_Grid:
        gui_ui_games_grid(gui);
        break;
    case DisplayMode_Kanban:
        gui_ui_games_kanban(gui);
        break;
    }

    gui_ui_bottom_bar(gui);

    ImGui_EndChild();

    ImGui_SameLineEx(0.0f, gui->style->ItemSpacing.x);

    // Sidebar

    ImGui_BeginChild(
        "###sidebar_frame",
        (ImVec2){sidebar_width - gui->style->ItemSpacing.x + 1.0f, 0.0f},
        ImGuiChildFlags_None,
        ImGuiWindowFlags_None);

    gui_ui_side_bar(gui);

    ImGui_EndChild();

    // FIXME: status/watermark text

    return;

    ImGui_Text("Hello, World!");

    ImGui_Text("Version: " F95CHECKER_VERSION " WIP");

    ImGui_Text("FPS: %.1f", (f64)gui->io->Framerate);

    ImGui_Spacing();

    ImGui_Text("DB test/demo:");

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Browser Custom Arguments:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(690.0f);
    gui_ui_input_text(
        gui,
        "###browser_custom_arguments",
        settings->browser_custom_arguments,
        ImGuiInputTextFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_setting(db, settings, SettingsColumn_browser_custom_arguments);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("BG interval:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(100.0f);
    ImGui_DragIntEx(
        "###bg_refresh_interval",
        &settings->bg_refresh_interval,
        4.0,
        30,
        1440,
        "%d min",
        ImGuiSliderFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_setting(db, settings, SettingsColumn_bg_refresh_interval);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Vsync:");
    ImGui_SameLine();
    if(gui_ui_checkbox(gui, "###vsync", &settings->vsync)) {
        db_save_setting(db, settings, SettingsColumn_vsync);
        gui_window_set_vsync(gui, settings->vsync);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("BG on Close:");
    ImGui_SameLine();
    if(gui_ui_checkbox(gui, "###background_on_close", &settings->background_on_close)) {
        db_save_setting(db, settings, SettingsColumn_background_on_close);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Accent color:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(200.0f);
    ImGui_ColorEdit3(
        "###style_accent",
        (f32*)&settings->style_accent.Value,
        ImGuiColorEditFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_setting(db, settings, SettingsColumn_style_accent);
    }

    if(ImGui_Button("Test save JSON fields")) {
        db_save_setting(db, settings, SettingsColumn_default_exe_dir);
        db_save_setting(db, settings, SettingsColumn_downloads_dir);
        db_save_setting(db, settings, SettingsColumn_tags_highlights);
        db_save_setting(db, settings, SettingsColumn_manual_sort_list);
        db_save_setting(db, settings, SettingsColumn_hidden_timeline_events);
    }

    ImGui_BeginGroup();
    ImGui_AlignTextToFramePadding();
    ImGui_Text("Tabs:");
    if(ImGui_Button("Test add new tab")) {
        Tab_ptr tab = db_create_tab(db, tabs);
        UNUSED(tab);
    }
    for each(Tab_ptr, tab, TabList, tabs) {
        ImGui_PushIDInt(tab->id);
        bool do_delete = ImGui_Button(mdi_trash_can_outline);
        ImGui_SameLine();
        ImGui_Text("%d %d %s", tab->position, tab->id, m_string_get_cstr(tab->name));
        ImGui_PopID();
        if(do_delete) {
            tab_list_next(tab_it);
            db_delete_tab(db, tab, tabs);
        }
    }
    ImGui_EndGroup();

    ImGui_SameLine();

    ImGui_BeginGroup();
    ImGui_AlignTextToFramePadding();
    ImGui_Text("Labels:");
    if(ImGui_Button("Test add new label")) {
        Label_ptr label = db_create_label(db, labels);
        UNUSED(label);
    }
    for each(Label_ptr, label, LabelList, labels) {
        ImGui_PushIDInt(label->id);
        bool do_delete = ImGui_Button(mdi_trash_can_outline);
        ImGui_SameLine();
        ImGui_Text("%d %d %s", label->position, label->id, m_string_get_cstr(label->name));
        ImGui_PopID();
        if(do_delete) {
            label_list_next(label_it);
            db_delete_label(db, label, labels);
        }
    }
    ImGui_EndGroup();

    for each(GameDict_pair, pair, GameDict, games) {
        Game* game = pair.value;
        ImGui_Text("%d %s", game->id, m_string_get_cstr(game->name));
        for each(Label_ptr, label, LabelPtrList, game->labels) {
            ImGui_SameLine();
            ImGui_TextUnformatted(m_string_get_cstr(label->name));
        }
    }
}

void gui_ui_free(Gui* gui) {
    UNUSED(gui);
    // FIXME: clear sorted+filtered games
}
