#include "backend.h"

#include "tray.h"
#include "window.h"

#include <globals.h>

bool gui_backend_init(Gui* gui) {
    UNUSED(gui);

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Prefer Wayland when available
    // SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland");

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        gui_perror("SDL_Init()");

        if(strcmp(SDL_GetError(), "wayland not available") == 0) {
            SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
            if(!SDL_Init(SDL_INIT_VIDEO)) {
                gui_perror("SDL_Init()");
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

void gui_backend_process_events(Gui* gui) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_EVENT_QUIT) {
            // QUIT sent when terminating the process and when exiting from tray menu, exit immediately
            gui->should_close = true;
        } else if(
            event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(gui->window)) {
            // Window was closed, respect background mode settings
            if(settings->background_on_close) {
                SDL_HideWindow(gui->window);
            } else {
                gui->should_close = true;
            }
        } else if(
            (event.type == SDL_EVENT_WINDOW_HIDDEN || event.type == SDL_EVENT_WINDOW_SHOWN) &&
            event.window.windowID == SDL_GetWindowID(gui->window)) {
            gui->window_hidden = event.type == SDL_EVENT_WINDOW_HIDDEN;
            gui_tray_update(gui);
        }
        gui_window_process_event(gui, &event);
    }
}

void gui_backend_free(Gui* gui) {
    UNUSED(gui);

    SDL_Quit();
}
