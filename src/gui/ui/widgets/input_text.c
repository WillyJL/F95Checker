#include "../ui_i.h"

static i32 gui_ui_input_text_resize_callback(ImGuiInputTextCallbackData* data) {
    if(data->EventFlag & ImGuiInputTextFlags_CallbackResize) {
        m_string_ptr str = data->UserData;
        m_str1ng_set_size(str, strlen(data->Buf));
        m_string_reserve(str, data->BufTextLen + 1);
        m_str1ng_set_size(str, data->BufTextLen);
        data->Buf = (char*)m_string_get_cstr(str);
    }

    return 0;
}

bool gui_ui_input_text(Gui* gui, const char* label, m_string_ptr str, ImGuiInputTextFlags flags) {
    UNUSED(gui);

    // FIXME: add right click actions and icon support
    return ImGui_InputTextEx(
        label,
        (char*)m_string_get_cstr(str),
        m_string_capacity(str),
        flags | ImGuiInputTextFlags_CallbackResize,
        gui_ui_input_text_resize_callback,
        str);
}
