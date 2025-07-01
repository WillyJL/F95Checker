#include "gui.h"

#include "backend.h"
#include "icons.h"
#include "tray.h"
#include "ui.h"
#include "window.h"

#include <globals.h>

Gui* gui_init(void) {
    Gui* gui = malloc(sizeof(Gui));
    gui->should_close = false;
    gui->background_mode = settings->start_in_background;

    if(!gui_backend_init(gui)) {
        free(gui);
        return NULL;
    }

    gui_icons_init(gui);

    if(!gui_tray_init(gui)) {
        gui_icons_free(gui);
        gui_backend_free(gui);
        free(gui);
        return NULL;
    }

    if(!gui_window_init(gui)) {
        gui_tray_free(gui);
        gui_icons_free(gui);
        gui_backend_free(gui);
        free(gui);
        return NULL;
    }

    gui->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    gui->io->IniFilename = NULL;
    gui->io->LogFilename = NULL;
    gui->io->ConfigDragClickToInputText = true;
    ImGui_StyleColorsDark(NULL);
    gui->style->ScrollbarSize = 10.0f;
    gui->style->FrameBorderSize = 1.0f;
    gui->style->ItemSpacing.x = gui->style->ItemSpacing.y;
    gui->style->Colors[ImGuiCol_ModalWindowDimBg] = (ImVec4){0.0f, 0.0f, 0.0f, 0.5f};

    return gui;
}

bool gui_should_close(Gui* gui) {
    return gui->should_close;
}

void gui_tick(Gui* gui) {
    gui_backend_process_events(gui);
    if(gui->should_close) {
        return;
    }
    if(!gui_window_should_draw(gui)) {
        gui_backend_wait_idle_time(gui);
        return;
    }
    gui_window_new_frame(gui);
    gui_ui_draw(gui);
    gui_window_render(gui);
}

void gui_free(Gui* gui) {
    gui_window_free(gui);
    gui_tray_free(gui);
    gui_icons_free(gui);
    gui_backend_free(gui);
    free(gui);
}
