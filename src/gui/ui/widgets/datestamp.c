#include "../ui_i.h"

#include <globals.h>

void gui_ui_datestamp(Gui* gui, Datestamp datestamp, const char* alt) {
    ImGui_PushFont(gui->fonts.mono);

    // FIXME: check if this is converting correctly from UTC
    if(datestamp == 0 && alt != NULL) {
        ImGui_TextUnformatted(alt);
    } else {
        char date[32];
        struct tm* tp = localtime(&datestamp);
        size_t res =
            strftime(date, sizeof(date), m_string_get_cstr(settings->datestamp_format), tp);
        if(res == 0 && !m_string_empty_p(settings->datestamp_format)) {
            ImGui_TextUnformatted("Bad strftime() format!");
        } else {
            ImGui_TextUnformatted(date);
        }
    }

    ImGui_PopFont();
}
