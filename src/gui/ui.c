#include "ui.h"

#include "util.h"

#include <globals.h>

const char* ok = mdi_check " Ok";
const char* cancel = mdi_cancel " Cancel";

void gui_ui_draw(Gui* gui) {
    ImGui_Text("Hello, World!");

    ImGui_Text("Version: " F95CHECKER_VERSION " WIP");

    ImGui_Text("FPS: %.1f", (flt64_t)gui->io->Framerate);

    ImGui_Spacing();

    ImGui_Text("DB test/demo:");

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Browser Custom Arguments:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(690.0f);
    ImGui_InputTextMString(
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
    ImGui_Text("Accent color:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(200.0f);
    ImGui_ColorEdit3(
        "###style_accent",
        (flt32_t*)&settings->style_accent.Value,
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
