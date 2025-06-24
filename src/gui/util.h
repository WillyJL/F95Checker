#pragma once

#include "gui.h"

#define ImGui_InputTextMString(label, str, flags)   \
    ImGui_InputTextEx(                              \
        label,                                      \
        (char*)m_string_get_cstr(str),              \
        m_string_capacity(str),                     \
        flags | ImGuiInputTextFlags_CallbackResize, \
        gui_util_input_text_resize_callback,        \
        str)

i32 gui_util_input_text_resize_callback(ImGuiInputTextCallbackData* data);
bool gui_util_is_topmost();
bool gui_util_should_close_weak_modal();
