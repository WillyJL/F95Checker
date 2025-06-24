#include "tray.h"

#include "window.h"

#include "types/datetime.h"

#include <globals.h>

static void gui_tray_callback_watermark(void* ctx, SDL_TrayEntry* invoker) {
    Gui* gui = ctx;
    UNUSED(invoker);

    gui_window_show(gui);
}

static bool refreshing = false;
static void gui_tray_callback_refresh_now(void* ctx, SDL_TrayEntry* invoker) {
    Gui* gui = ctx;
    UNUSED(invoker);

    // FIXME: hook into refresh system
    refreshing = !refreshing;
    gui_tray_update(gui);
}

static bool refresh_paused = false;
static void gui_tray_callback_toggle_pause(void* ctx, SDL_TrayEntry* invoker) {
    Gui* gui = ctx;
    UNUSED(gui);
    UNUSED(invoker);

    // FIXME: hook into refresh timer
    refresh_paused = !refresh_paused;
    gui_tray_update(gui);
}

static void gui_tray_callback_toggle_gui(void* ctx, SDL_TrayEntry* invoker) {
    Gui* gui = ctx;
    UNUSED(invoker);

    if(gui->window_hidden) {
        gui_window_show(gui);
    } else {
        gui_window_hide(gui);
    }
}

static void gui_tray_callback_quit(void* ctx, SDL_TrayEntry* invoker) {
    Gui* gui = ctx;
    UNUSED(gui);
    UNUSED(invoker);

    SDL_Event event;
    event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&event);
}

bool gui_tray_init(Gui* gui) {
    gui->tray = SDL_CreateTray(gui->icons.logo, "F95Checker WIP C Rewrite");
    if(gui->tray == NULL) {
        gui_perror("SDL_CreateTray()");
        return false;
    }

    gui->tray_menu = SDL_CreateTrayMenu(gui->tray);

    gui->tray_items.watermark = SDL_InsertTrayEntryAt(
        gui->tray_menu,
        -1,
        "F95Checker " F95CHECKER_VERSION,
        SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(gui->tray_items.watermark, gui_tray_callback_watermark, gui);

    if(gui->window_hidden) {
        gui->tray_items.next_refresh = SDL_InsertTrayEntryAt(
            gui->tray_menu,
            -1,
            "Next Refresh: ???",
            SDL_TRAYENTRY_BUTTON | SDL_TRAYENTRY_DISABLED);
    } else {
        gui->tray_items.next_refresh = NULL;
    }

    gui->tray_items.refresh_now = SDL_InsertTrayEntryAt(
        gui->tray_menu,
        -1,
        refreshing ? "Cancel Refresh" : "Refresh Now!",
        SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(gui->tray_items.refresh_now, gui_tray_callback_refresh_now, gui);

    if(gui->window_hidden) {
        gui->tray_items.toggle_pause = SDL_InsertTrayEntryAt(
            gui->tray_menu,
            -1,
            refresh_paused ? "Unpause Auto Refresh" : "Pause Auto Refresh",
            SDL_TRAYENTRY_BUTTON);
        SDL_SetTrayEntryCallback(gui->tray_items.toggle_pause, gui_tray_callback_toggle_pause, gui);
    } else {
        gui->tray_items.toggle_pause = NULL;
    }

    gui->tray_items.toggle_gui = SDL_InsertTrayEntryAt(
        gui->tray_menu,
        -1,
        gui->window_hidden ? "Switch to GUI" : "Switch to BG",
        SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(gui->tray_items.toggle_gui, gui_tray_callback_toggle_gui, gui);

    gui->tray_items.quit = SDL_InsertTrayEntryAt(gui->tray_menu, -1, "Quit", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(gui->tray_items.quit, gui_tray_callback_quit, gui);

    m_string_init(gui->tray_state.next_refresh_str);
    gui->tray_state.prev_hidden = gui->window_hidden;
    gui->tray_state.prev_paused = false;
    gui->tray_state.prev_refreshing = false;
    gui_tray_update(gui);

    return true;
}

static void gui_tray_update_next_refresh(Gui* gui) {
    m_string_t next_refresh_str;
    m_string_init_set(next_refresh_str, "Next Refresh: ");

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    Timestamp now = ts.tv_sec;

    // FIXME: hook into refresh system and timer
    Timestamp next_refresh = ts.tv_sec + (rand() % (settings->bg_refresh_interval * 60));
    if(refreshing) {
        m_string_cat(next_refresh_str, "Now");
    } else if(refresh_paused) {
        m_string_cat(next_refresh_str, "Paused");
    } else if(next_refresh < now) {
        m_string_cat(next_refresh_str, "Never?");
    } else if(next_refresh == now) {
        m_string_cat(next_refresh_str, "Soon");
    } else {
        u32 seconds = next_refresh - now;
        const u32 SECOND = 1;
        const u32 MINUTE = 60 * SECOND;
        const u32 HOUR = 60 * MINUTE;
        const u32 DAY = 24 * HOUR;
        m_string_cat(next_refresh_str, "in ");
        if(seconds < MINUTE) {
            m_string_cat_printf(next_refresh_str, "%d second%s", seconds, seconds == 1 ? "" : "s");
        } else if(seconds < HOUR) {
            u32 minutes = seconds / MINUTE;
            m_string_cat_printf(next_refresh_str, "%d minute%s", minutes, minutes == 1 ? "" : "s");
        } else if(seconds < DAY) {
            u32 hours = seconds / HOUR;
            m_string_cat_printf(next_refresh_str, "%d hour%s", hours, hours == 1 ? "" : "s");
        } else {
            u32 days = seconds / DAY;
            m_string_cat_printf(next_refresh_str, "%d days%s", days, days == 1 ? "" : "s");
        }
    }

    m_string_move(gui->tray_state.next_refresh_str, next_refresh_str);
    SDL_SetTrayEntryLabel(
        gui->tray_items.next_refresh,
        m_string_get_cstr(gui->tray_state.next_refresh_str));
}

void gui_tray_update(Gui* gui) {
    if(gui->tray_state.prev_hidden != gui->window_hidden) {
        gui->tray_state.prev_hidden = gui->window_hidden;
        SDL_SetTrayEntryLabel(
            gui->tray_items.toggle_gui,
            gui->window_hidden ? "Switch to GUI" : "Switch to BG");

        if(gui->window_hidden) {
            if(refresh_paused && !refreshing) {
                SDL_SetTrayIcon(gui->tray, gui->icons.paused);
            }

            gui->tray_items.next_refresh = SDL_InsertTrayEntryAt(
                gui->tray_menu,
                1,
                "Next Refresh: ???",
                SDL_TRAYENTRY_BUTTON | SDL_TRAYENTRY_DISABLED);

            gui->tray_items.toggle_pause = SDL_InsertTrayEntryAt(
                gui->tray_menu,
                3,
                refresh_paused ? "Unpause Auto Refresh" : "Pause Auto Refresh",
                SDL_TRAYENTRY_BUTTON);
            SDL_SetTrayEntryCallback(
                gui->tray_items.toggle_pause,
                gui_tray_callback_toggle_pause,
                gui);
        } else {
            if(refresh_paused && !refreshing) {
                SDL_SetTrayIcon(gui->tray, gui->icons.logo);
            }

            SDL_RemoveTrayEntry(gui->tray_items.next_refresh);
            gui->tray_items.next_refresh = NULL;

            SDL_RemoveTrayEntry(gui->tray_items.toggle_pause);
            gui->tray_items.toggle_pause = NULL;
        }
    }

    if(gui->tray_state.prev_paused != refresh_paused) {
        gui->tray_state.prev_paused = refresh_paused;
        if(gui->window_hidden) {
            SDL_SetTrayEntryLabel(
                gui->tray_items.toggle_pause,
                refresh_paused ? "Unpause Auto Refresh" : "Pause Auto Refresh");
            if(!refreshing) {
                SDL_SetTrayIcon(gui->tray, refresh_paused ? gui->icons.paused : gui->icons.logo);
            }
        }
    }

    if(gui->tray_state.prev_refreshing != refreshing) {
        gui->tray_state.prev_refreshing = refreshing;
        SDL_SetTrayEntryLabel(
            gui->tray_items.refresh_now,
            refreshing ? "Cancel Refresh" : "Refresh Now!");
        // TODO: animate icon
        SDL_SetTrayIcon(
            gui->tray,
            refreshing                             ? gui->icons.refreshing[0] :
            (gui->window_hidden && refresh_paused) ? gui->icons.paused :
                                                     gui->icons.logo);
    }

    if(gui->window_hidden) {
        gui_tray_update_next_refresh(gui);
    }
}

void gui_tray_free(Gui* gui) {
    // Menu and entries tied to the tray are destroyed automatically
    SDL_DestroyTray(gui->tray);

    m_string_clear(gui->tray_state.next_refresh_str);
}
