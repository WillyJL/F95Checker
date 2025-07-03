#pragma once

#include "../ui.h" // IWYU pragma: export

bool gui_ui_checkbox(Gui* gui, const char* label, bool* value);
bool gui_ui_input_text(Gui* gui, const char* label, m_string_ptr str, ImGuiInputTextFlags flags);
