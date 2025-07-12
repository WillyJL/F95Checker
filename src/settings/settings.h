#pragma once

#include "types.h"

#include "browser/browser.h"
#include "game/game.h"
#include "path/path.h"
#include "types/datetime.h"
#include "types/tab.h"

#include <dcimgui/dcimgui.h>

#include <std.h>

#define SETTINGS_DFLT_STYLE_ACCENT   "#d4202e"
#define SETTINGS_DFLT_STYLE_ALT_BG   "#101010"
#define SETTINGS_DFLT_STYLE_BG       "#0a0a0a"
#define SETTINGS_DFLT_STYLE_BORDER   "#454545"
#define SETTINGS_DFLT_STYLE_CORNERS  6
#define SETTINGS_DFLT_STYLE_TEXT     "#ffffff"
#define SETTINGS_DFLT_STYLE_TEXT_DIM "#808080"

typedef struct {
    bool background_on_close;
    i32 bg_notifs_interval;
    i32 bg_refresh_interval;
    Browser_ptr browser;
    m_string_t browser_custom_arguments;
    m_string_t browser_custom_executable;
    bool browser_html;
    bool browser_private;
    f32 cell_image_ratio;
    bool check_notifs;
    bool compact_timeline;
    bool confirm_on_remove;
    bool copy_urls_as_bbcode;
    m_string_t datestamp_format;
    Path* default_exe_dir[1 + Os_COUNT];
    bool default_tab_is_new;
    DisplayMode display_mode;
    Tab_ptr display_tab;
    Path* downloads_dir[1 + Os_COUNT];
    bool ext_background_add;
    bool ext_highlight_tags;
    bool ext_icon_glow;
    bool filter_all_tabs;
    bool fit_images;
    i32 grid_columns;
    bool hidden_timeline_events[1 + GameTimelineEventType_COUNT];
    bool hide_empty_tabs;
    bool highlight_tags;
    bool ignore_semaphore_timeouts;
    bool independent_tab_views;
    bool insecure_ssl;
    f32 interface_scaling;
    Timestamp last_successful_refresh;
    GameArray manual_sort_list;
    bool mark_installed_after_add;
    i32 max_connections;
    i32 max_retries;
    bool notifs_show_update_banner;
    bool play_gifs;
    bool play_gifs_unfocused;
    bool preload_nearby_images;
    ProxyType proxy_type;
    m_string_t proxy_host;
    i32 proxy_port;
    m_string_t proxy_username;
    m_string_t proxy_password;
    bool quick_filters;
    bool refresh_archived_games;
    bool refresh_completed_games;
    bool render_when_unfocused;
    i32 request_timeout;
    bool rpc_enabled;
    m_string_t rpdl_password;
    m_string_t rpdl_token;
    m_string_t rpdl_username;
    f32 scroll_amount;
    bool scroll_smooth;
    f32 scroll_smooth_speed;
    bool select_executable_after_add;
    bool show_remove_btn;
    bool software_webview;
    bool start_in_background;
    bool start_refresh;
    ImColor4 style_accent;
    ImColor4 style_alt_bg;
    ImColor4 style_bg;
    ImColor4 style_border;
    i32 style_corner_radius;
    ImColor4 style_text;
    ImColor4 style_text_dim;
    bool table_header_outside_list;
    TagHighlight tags_highlights[GameTag_COUNT];
    TexCompress tex_compress;
    bool tex_compress_replace;
    m_string_t timestamp_format;
    bool unload_offscreen_images;
    bool vsync;
    bool weighted_score;
    i32 zoom_area;
    bool zoom_enabled;
    f32 zoom_times;

    GameIdArray _manual_sort_list_temp;
} Settings;

Settings* settings_init(void);

void settings_after_games_load(Settings* settings, GameDict_ptr games);

void settings_free(Settings* settings);
