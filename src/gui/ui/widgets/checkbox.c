#include "widgets.h"

bool gui_ui_checkbox(Gui* gui, const char* label, bool* value) {
    const bool patch_colors = *value;

    if(patch_colors) {
        ImGui_PushStyleColor4(ImGuiCol_CheckMark, gui->style->Colors[ImGuiCol_WindowBg]);
        ImGui_PushStyleColor4(ImGuiCol_FrameBg, gui->style->Colors[ImGuiCol_ButtonHovered]);
        ImGui_PushStyleColor4(ImGuiCol_FrameBgHovered, gui->style->Colors[ImGuiCol_ButtonHovered]);
    }

    const bool ret = ImGui_Checkbox(label, value);

    if(patch_colors) {
        ImGui_PopStyleColorEx(3);
    }

    return ret;
}
