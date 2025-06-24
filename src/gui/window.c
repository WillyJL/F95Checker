#include "window.h"

#include "fonts.h"

#include <dcimgui/dcimgui_impl_sdl3.h>
#include <dcimgui/dcimgui_impl_sdlrenderer3.h>

#include <globals.h>

bool gui_window_init(Gui* gui) {
    // FIXME: handle DPI correctly
    const SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
    // FIXME: load and save window size from imgui.ini
    // FIXME: use data path for imgui.ini
    gui->window = SDL_CreateWindow("F95Checker WIP C Rewrite", 1280, 720, window_flags);
    if(gui->window == NULL) {
        gui_perror("SDL_CreateWindow()");
        return false;
    }
    SDL_SetWindowMinimumSize(gui->window, 720, 400);
    SDL_SetWindowIcon(gui->window, gui->icons.icon);

    // FIXME: add setting to select software rendering
    gui->window_renderer = SDL_CreateRenderer(gui->window, NULL);
    if(gui->window_renderer == NULL) {
        gui_perror("SDL_CreateRenderer()");
        SDL_DestroyWindow(gui->window);
        return false;
    }
    SDL_SetRenderVSync(gui->window_renderer, settings->vsync_ratio);

    IMGUI_CHECKVERSION();
    ImGui_CreateContext(NULL);
    gui->io = ImGui_GetIO();
    gui->style = ImGui_GetStyle();
    gui->window_state.prev_size = (Vec2){0, 0};
    gui->window_state.scroll_energy = (ImVec2){0.0f, 0.0f};

    gui_fonts_init(gui);

    ImGui_ImplSDL3_InitForSDLRenderer(gui->window, gui->window_renderer);
    ImGui_ImplSDLRenderer3_Init(gui->window_renderer);

    return true;
}

void gui_window_show(Gui* gui) {
    if(gui->window_hidden) {
        SDL_ShowWindow(gui->window);
    }
    if(gui->window_minimized) {
        SDL_RestoreWindow(gui->window);
    }
    SDL_RaiseWindow(gui->window);
}

void gui_window_hide(Gui* gui) {
    SDL_HideWindow(gui->window);
}

void gui_window_process_event(Gui* gui, SDL_Event* event) {
    if(event->type == SDL_EVENT_MOUSE_WHEEL &&
       event->window.windowID == SDL_GetWindowID(gui->window)) {
        // Handle wheel events locally to apply scroll settings
        event->wheel.x *= settings->scroll_amount;
        event->wheel.y *= settings->scroll_amount;
        if(settings->scroll_smooth) {
            // Immediately stop if direction changes
            if(gui->window_state.scroll_energy.x * event->wheel.x < 0.0f) {
                gui->window_state.scroll_energy.x = 0.0f;
            }
            if(gui->window_state.scroll_energy.y * event->wheel.y < 0.0f) {
                gui->window_state.scroll_energy.y = 0.0f;
            }
        }
        gui->window_state.scroll_energy.x += event->wheel.x;
        gui->window_state.scroll_energy.y += event->wheel.y;
    } else if(
        (event->type == SDL_EVENT_WINDOW_MINIMIZED || event->type == SDL_EVENT_WINDOW_RESTORED) &&
        event->window.windowID == SDL_GetWindowID(gui->window)) {
        gui->window_minimized = event->type == SDL_EVENT_WINDOW_MINIMIZED;
    }
    ImGui_ImplSDL3_ProcessEvent(event);
}

void gui_window_new_frame(Gui* gui) {
    UNUSED(gui);

    // Apply smooth scrolling
    ImVec2 scroll_now;
    if(settings->scroll_smooth) {
        if(ABS(gui->window_state.scroll_energy.x) > 0.01f) {
            scroll_now.x = gui->window_state.scroll_energy.x * gui->io->DeltaTime *
                           settings->scroll_smooth_speed;
            gui->window_state.scroll_energy.x -= scroll_now.x;
        } else {
            // Cutoff smoothing when it's basically stopped
            scroll_now.x = 0.0f;
            gui->window_state.scroll_energy.x = 0.0f;
        }
        if(ABS(gui->window_state.scroll_energy.y) > 0.01f) {
            scroll_now.y = gui->window_state.scroll_energy.y * gui->io->DeltaTime *
                           settings->scroll_smooth_speed;
            gui->window_state.scroll_energy.y -= scroll_now.y;
        } else {
            // Cutoff smoothing when it's basically stopped
            scroll_now.y = 0.0f;
            gui->window_state.scroll_energy.y = 0.0f;
        }
    } else {
        scroll_now = gui->window_state.scroll_energy;
        gui->window_state.scroll_energy = (ImVec2){0, 0};
    }
    gui->io->MouseWheel = scroll_now.y;
    gui->io->MouseWheelH = -scroll_now.x;

    // Hand cursor when hovering buttons and similar
    if(ImGui_IsAnyItemHovered()) {
        ImGui_SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui_NewFrame();

    ImGui_SetNextWindowPos((ImVec2){0.0f, 0.0f}, ImGuiCond_Once);
    Vec2 window_size;
    SDL_GetWindowSize(gui->window, &window_size.x, &window_size.y);
    if(window_size.x != gui->window_state.prev_size.x ||
       window_size.y != gui->window_state.prev_size.y) {
        ImGui_SetNextWindowSize(
            (ImVec2){(flt32_t)window_size.x, (flt32_t)window_size.y},
            ImGuiCond_Always);
        gui->window_state.prev_size = window_size;
    }

    ImGui_PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui_Begin(
        "F95Checker",
        NULL,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
    ImGui_PopStyleVar();
}

void gui_window_render(Gui* gui) {
    ImGui_End();

    ImGui_Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui_GetDrawData(), gui->window_renderer);
    SDL_RenderPresent(gui->window_renderer);
}

void gui_window_free(Gui* gui) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    gui_fonts_free(gui);
    ImGui_DestroyContext(NULL);

    SDL_DestroyRenderer(gui->window_renderer);
    SDL_DestroyWindow(gui->window);
}
