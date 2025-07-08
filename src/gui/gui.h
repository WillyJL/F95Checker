#pragma once

#include "game/game.h"
#include "types/tab.h"

#include <dcimgui/dcimgui.h>
#include <fonts/mdi.h>
#include <SDL3/SDL.h>

#include <std.h>

typedef struct {
    SDL_Tray* tray;
    SDL_TrayMenu* tray_menu;
    struct {
        SDL_TrayEntry* watermark;
        SDL_TrayEntry* next_refresh;
        SDL_TrayEntry* refresh_now;
        SDL_TrayEntry* toggle_pause;
        SDL_TrayEntry* toggle_gui;
        SDL_TrayEntry* quit;
    } tray_items;
    struct {
        m_string_t next_refresh_str;
        bool prev_bg;
        bool prev_paused;
        bool prev_refreshing;
    } tray_state;

    SDL_Window* window;
    SDL_GPUDevice* window_gpu;
    struct {
        ImVec2 scroll_energy;
        Vec2 prev_size;
    } window_state;

    ImGuiIO* io;
    ImGuiStyle* style;

    struct {
        Tab_ptr current_tab;
        Tab_ptr dragging_tab;
        GameIndex game_index;
        bool need_game_index_rebuild;
        bool is_filtering;
        f32 ghost_columns_enabled_count;
        bool columns_enabled[22];
        bool prev_manual_sort;
    } ui_state;

    struct {
        ImFont* base;
        ImFont* mono;
    } fonts;

    struct {
        SDL_Surface* icon;
        SDL_Surface* logo;
        SDL_Surface* paused;
        SDL_Surface* refreshing[16];
    } icons;

    bool should_close;
    bool background_mode;
} Gui;

Gui* gui_init(void);

bool gui_should_close(Gui* gui);
void gui_tick(Gui* gui);

void gui_free(Gui* gui);

static inline void gui_perror(const char* s) {
    custom_perror(s, SDL_GetError());
}
