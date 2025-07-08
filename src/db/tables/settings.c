#include "../db_i.h"

#include <globals.h>

DB_TABLE_DEFINE(_SETTINGS, settings, SettingsColumn)

static void db_parse_settings(Db* db, sqlite3_stmt* stmt, Settings* settings) {
    UNUSED(db);
    size_t col = 1; // Skip _ column

    settings->background_on_close = sqlite3_column_int(stmt, col++);
    settings->bg_notifs_interval = sqlite3_column_int(stmt, col++);
    settings->bg_refresh_interval = sqlite3_column_int(stmt, col++);
    m_string_set(settings->browser_custom_arguments, sqlite3_column_text(stmt, col++));
    m_string_set(settings->browser_custom_executable, sqlite3_column_text(stmt, col++));
    settings->browser_html = sqlite3_column_int(stmt, col++);
    settings->browser_private = sqlite3_column_int(stmt, col++);

    settings->browser = *browser_list_front(browsers); // Integrated by default
    BrowserHash browser_hash = sqlite3_column_int64(stmt, col);
    for each(Browser_ptr, browser, BrowserList, browsers) {
        if(browser->hash == browser_hash) {
            settings->browser = browser;
            break;
        }
    }
    col++;

    settings->cell_image_ratio = sqlite3_column_float(stmt, col++);
    settings->check_notifs = sqlite3_column_int(stmt, col++);
    settings->compact_timeline = sqlite3_column_int(stmt, col++);
    settings->confirm_on_remove = sqlite3_column_int(stmt, col++);
    settings->copy_urls_as_bbcode = sqlite3_column_int(stmt, col++);
    m_string_set(settings->datestamp_format, sqlite3_column_text(stmt, col++));

    const char* default_exe_dir_text = sqlite3_column_text(stmt, col++);
    if(default_exe_dir_text[0] == '{') {
        json_object* default_exe_dir_json = json_tokener_parse(default_exe_dir_text);
        for(Os os = Os_min(); os <= Os_max(); os++) {
            char os_key[2];
            snprintf(os_key, sizeof(os_key), "%d", os);
            json_object* default_exe_dir = json_object_object_get(default_exe_dir_json, os_key);
            if(!json_object_is_type(default_exe_dir, json_type_string)) continue;
            path_set(settings->default_exe_dir[os], json_object_get_string(default_exe_dir));
        }
        json_object_put(default_exe_dir_json);
    } else {
        path_set(settings->default_exe_dir[os], default_exe_dir_text);
    }

    settings->default_tab_is_new = sqlite3_column_int(stmt, col++);
    settings->display_mode = sqlite3_column_int(stmt, col++);

    settings->display_tab = NULL;
    if(sqlite3_column_type(stmt, col) != SQLITE_NULL) {
        TabId tab_id = sqlite3_column_int(stmt, col);
        for each(Tab_ptr, tab, TabList, tabs) {
            if(tab->id == tab_id) {
                settings->display_tab = tab;
                break;
            }
        }
    }
    col++;

    json_object* downloads_dir_json = sqlite3_column_json(stmt, col++);
    for(Os os = Os_min(); os <= Os_max(); os++) {
        char os_key[2];
        snprintf(os_key, sizeof(os_key), "%d", os);
        json_object* downloads_dir = json_object_object_get(downloads_dir_json, os_key);
        if(!json_object_is_type(downloads_dir, json_type_string)) continue;
        path_set(settings->downloads_dir[os], json_object_get_string(downloads_dir));
    }
    json_object_put(downloads_dir_json);

    settings->ext_background_add = sqlite3_column_int(stmt, col++);
    settings->ext_highlight_tags = sqlite3_column_int(stmt, col++);
    settings->ext_icon_glow = sqlite3_column_int(stmt, col++);
    settings->filter_all_tabs = sqlite3_column_int(stmt, col++);
    settings->fit_images = sqlite3_column_int(stmt, col++);
    settings->grid_columns = sqlite3_column_int(stmt, col++);

    json_object* hidden_timeline_events_json = sqlite3_column_json(stmt, col++);
    memset(settings->hidden_timeline_events, false, sizeof(settings->hidden_timeline_events));
    for(size_t i = 0; i < json_object_array_length(hidden_timeline_events_json); i++) {
        json_object* timeline_event = json_object_array_get_idx(hidden_timeline_events_json, i);
        settings->hidden_timeline_events[json_object_get_int(timeline_event)] = true;
    }
    json_object_put(hidden_timeline_events_json);

    settings->hide_empty_tabs = sqlite3_column_int(stmt, col++);
    settings->highlight_tags = sqlite3_column_int(stmt, col++);
    settings->ignore_semaphore_timeouts = sqlite3_column_int(stmt, col++);
    settings->independent_tab_views = sqlite3_column_int(stmt, col++);
    settings->insecure_ssl = sqlite3_column_int(stmt, col++);
    settings->interface_scaling = sqlite3_column_float(stmt, col++);
    settings->last_successful_refresh = sqlite3_column_int64(stmt, col++);

    json_object* manual_sort_list_json = sqlite3_column_json(stmt, col++);
    game_id_array_resize(
        settings->manual_sort_list,
        json_object_array_length(manual_sort_list_json));
    for(size_t i = 0; i < json_object_array_length(manual_sort_list_json); i++) {
        json_object* manual_sort_id = json_object_array_get_idx(manual_sort_list_json, i);
        game_id_array_set_at(settings->manual_sort_list, i, json_object_get_int(manual_sort_id));
    }
    json_object_put(manual_sort_list_json);

    settings->mark_installed_after_add = sqlite3_column_int(stmt, col++);
    settings->max_connections = sqlite3_column_int(stmt, col++);
    settings->max_retries = sqlite3_column_int(stmt, col++);
    settings->notifs_show_update_banner = sqlite3_column_int(stmt, col++);
    settings->play_gifs = sqlite3_column_int(stmt, col++);
    settings->play_gifs_unfocused = sqlite3_column_int(stmt, col++);
    settings->preload_nearby_images = sqlite3_column_int(stmt, col++);
    settings->proxy_type = sqlite3_column_int(stmt, col++);
    m_string_set(settings->proxy_host, sqlite3_column_text(stmt, col++));
    settings->proxy_port = sqlite3_column_int(stmt, col++);
    m_string_set(settings->proxy_username, sqlite3_column_text(stmt, col++));
    m_string_set(settings->proxy_password, sqlite3_column_text(stmt, col++));
    settings->quick_filters = sqlite3_column_int(stmt, col++);
    settings->refresh_archived_games = sqlite3_column_int(stmt, col++);
    settings->refresh_completed_games = sqlite3_column_int(stmt, col++);
    settings->render_when_unfocused = sqlite3_column_int(stmt, col++);
    settings->request_timeout = sqlite3_column_int(stmt, col++);
    settings->rpc_enabled = sqlite3_column_int(stmt, col++);
    m_string_set(settings->rpdl_password, sqlite3_column_text(stmt, col++));
    m_string_set(settings->rpdl_token, sqlite3_column_text(stmt, col++));
    m_string_set(settings->rpdl_username, sqlite3_column_text(stmt, col++));
    settings->scroll_amount = sqlite3_column_float(stmt, col++);
    settings->scroll_smooth = sqlite3_column_int(stmt, col++);
    settings->scroll_smooth_speed = sqlite3_column_float(stmt, col++);
    settings->select_executable_after_add = sqlite3_column_int(stmt, col++);
    settings->show_remove_btn = sqlite3_column_int(stmt, col++);
    settings->software_webview = sqlite3_column_int(stmt, col++);
    settings->start_in_background = sqlite3_column_int(stmt, col++);
    settings->start_refresh = sqlite3_column_int(stmt, col++);
    settings->style_accent = sqlite3_column_imcolor4(stmt, col++);
    settings->style_alt_bg = sqlite3_column_imcolor4(stmt, col++);
    settings->style_bg = sqlite3_column_imcolor4(stmt, col++);
    settings->style_border = sqlite3_column_imcolor4(stmt, col++);
    settings->style_corner_radius = sqlite3_column_int(stmt, col++);
    settings->style_text = sqlite3_column_imcolor4(stmt, col++);
    settings->style_text_dim = sqlite3_column_imcolor4(stmt, col++);
    settings->table_header_outside_list = sqlite3_column_int(stmt, col++);

    json_object* tags_highlights_json = sqlite3_column_json(stmt, col++);
    for(GameTag tag = GameTag_min(); tag <= GameTag_max(); tag++) {
        char tag_key[4];
        snprintf(tag_key, sizeof(tag_key), "%d", tag);
        json_object* tag_highlight = json_object_object_get(tags_highlights_json, tag_key);
        if(!json_object_is_type(tag_highlight, json_type_int)) {
            settings->tags_highlights[tag] = TagHighlight_None;
        } else {
            settings->tags_highlights[tag] = json_object_get_int(tag_highlight);
        }
    }
    json_object_put(tags_highlights_json);

    settings->tex_compress = sqlite3_column_int(stmt, col++);
    settings->tex_compress_replace = sqlite3_column_int(stmt, col++);
    m_string_set(settings->timestamp_format, sqlite3_column_text(stmt, col++));
    settings->unload_offscreen_images = sqlite3_column_int(stmt, col++);
    settings->vsync = sqlite3_column_int(stmt, col++);
    settings->weighted_score = sqlite3_column_int(stmt, col++);
    settings->zoom_area = sqlite3_column_int(stmt, col++);
    settings->zoom_enabled = sqlite3_column_int(stmt, col++);
    settings->zoom_times = sqlite3_column_float(stmt, col++);
}

void db_do_load_settings(Db* db, Settings* settings) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &settings_table);

    // Insert the main settings row
    m_string_printf(
        sql,
        "INSERT INTO %s (_) VALUES (0) ON CONFLICT DO NOTHING",
        settings_table.name);
    res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Read the main settings row
    m_string_set(sql, "SELECT ");
    db_append_column_names(sql, &settings_table);
    m_string_cat_printf(sql, " FROM %s", settings_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    assert(sqlite3_column_count(stmt) == settings_table.columns_count);
    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
    db_parse_settings(db, stmt, settings);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

void db_do_save_setting(Db* db, Settings* settings, SettingsColumn column) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "UPDATE %s SET %s=? WHERE _=0",
        settings_table.name,
        settings_table.columns[column].name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    switch(column) {
    case SettingsColumn__:
        res = sqlite3_bind_int(stmt, 1, 0);
        break;
    case SettingsColumn_background_on_close:
        res = sqlite3_bind_int(stmt, 1, settings->background_on_close);
        break;
    case SettingsColumn_bg_notifs_interval:
        res = sqlite3_bind_int(stmt, 1, settings->bg_notifs_interval);
        break;
    case SettingsColumn_bg_refresh_interval:
        res = sqlite3_bind_int(stmt, 1, settings->bg_refresh_interval);
        break;
    case SettingsColumn_browser_custom_arguments:
        res = sqlite3_bind_mstring(stmt, 1, settings->browser_custom_arguments);
        break;
    case SettingsColumn_browser_custom_executable:
        res = sqlite3_bind_mstring(stmt, 1, settings->browser_custom_executable);
        break;
    case SettingsColumn_browser_html:
        res = sqlite3_bind_int(stmt, 1, settings->browser_html);
        break;
    case SettingsColumn_browser_private:
        res = sqlite3_bind_int(stmt, 1, settings->browser_private);
        break;
    case SettingsColumn_browser:
        res = sqlite3_bind_int64(stmt, 1, settings->browser->hash);
        break;
    case SettingsColumn_cell_image_ratio:
        res = sqlite3_bind_float(stmt, 1, settings->cell_image_ratio);
        break;
    case SettingsColumn_check_notifs:
        res = sqlite3_bind_int(stmt, 1, settings->check_notifs);
        break;
    case SettingsColumn_compact_timeline:
        res = sqlite3_bind_int(stmt, 1, settings->compact_timeline);
        break;
    case SettingsColumn_confirm_on_remove:
        res = sqlite3_bind_int(stmt, 1, settings->confirm_on_remove);
        break;
    case SettingsColumn_copy_urls_as_bbcode:
        res = sqlite3_bind_int(stmt, 1, settings->copy_urls_as_bbcode);
        break;
    case SettingsColumn_datestamp_format:
        res = sqlite3_bind_mstring(stmt, 1, settings->datestamp_format);
        break;
    case SettingsColumn_default_exe_dir:
        json_object* default_exe_dir_json = json_object_new_object();
        for(Os os = Os_min(); os <= Os_max(); os++) {
            if(path_is_empty(settings->default_exe_dir[os])) continue;
            char os_key[2];
            snprintf(os_key, sizeof(os_key), "%d", os);
            json_object_object_add_unique(
                default_exe_dir_json,
                os_key,
                json_object_new_string(path_cstr(settings->default_exe_dir[os])));
        }
        res = sqlite3_bind_json(stmt, 1, default_exe_dir_json);
        json_object_put(default_exe_dir_json);
        break;
    case SettingsColumn_default_tab_is_new:
        res = sqlite3_bind_int(stmt, 1, settings->default_tab_is_new);
        break;
    case SettingsColumn_display_mode:
        res = sqlite3_bind_int(stmt, 1, settings->display_mode);
        break;
    case SettingsColumn_display_tab:
        if(settings->display_tab == NULL) {
            res = sqlite3_bind_null(stmt, 1);
        } else {
            res = sqlite3_bind_int(stmt, 1, settings->display_tab->id);
        }
        break;
    case SettingsColumn_downloads_dir:
        json_object* downloads_dir_json = json_object_new_object();
        for(Os os = Os_min(); os <= Os_max(); os++) {
            if(path_is_empty(settings->downloads_dir[os])) continue;
            char os_key[2];
            snprintf(os_key, sizeof(os_key), "%d", os);
            json_object_object_add_unique(
                downloads_dir_json,
                os_key,
                json_object_new_string(path_cstr(settings->downloads_dir[os])));
        }
        res = sqlite3_bind_json(stmt, 1, downloads_dir_json);
        json_object_put(downloads_dir_json);
        break;
    case SettingsColumn_ext_background_add:
        res = sqlite3_bind_int(stmt, 1, settings->ext_background_add);
        break;
    case SettingsColumn_ext_highlight_tags:
        res = sqlite3_bind_int(stmt, 1, settings->ext_highlight_tags);
        break;
    case SettingsColumn_ext_icon_glow:
        res = sqlite3_bind_int(stmt, 1, settings->ext_icon_glow);
        break;
    case SettingsColumn_filter_all_tabs:
        res = sqlite3_bind_int(stmt, 1, settings->filter_all_tabs);
        break;
    case SettingsColumn_fit_images:
        res = sqlite3_bind_int(stmt, 1, settings->fit_images);
        break;
    case SettingsColumn_grid_columns:
        res = sqlite3_bind_int(stmt, 1, settings->grid_columns);
        break;
    case SettingsColumn_hidden_timeline_events:
        json_object* hidden_timeline_events_json = json_object_new_array();
        for(GameTimelineEventType ev = GameTimelineEventType_min();
            ev <= GameTimelineEventType_max();
            ev++) {
            if(settings->hidden_timeline_events[ev] == false) continue;
            json_object_array_add(hidden_timeline_events_json, json_object_new_int(ev));
        }
        res = sqlite3_bind_json(stmt, 1, hidden_timeline_events_json);
        json_object_put(hidden_timeline_events_json);
        break;
    case SettingsColumn_hide_empty_tabs:
        res = sqlite3_bind_int(stmt, 1, settings->hide_empty_tabs);
        break;
    case SettingsColumn_highlight_tags:
        res = sqlite3_bind_int(stmt, 1, settings->highlight_tags);
        break;
    case SettingsColumn_ignore_semaphore_timeouts:
        res = sqlite3_bind_int(stmt, 1, settings->ignore_semaphore_timeouts);
        break;
    case SettingsColumn_independent_tab_views:
        res = sqlite3_bind_int(stmt, 1, settings->independent_tab_views);
        break;
    case SettingsColumn_insecure_ssl:
        res = sqlite3_bind_int(stmt, 1, settings->insecure_ssl);
        break;
    case SettingsColumn_interface_scaling:
        res = sqlite3_bind_float(stmt, 1, settings->interface_scaling);
        break;
    case SettingsColumn_last_successful_refresh:
        res = sqlite3_bind_int64(stmt, 1, settings->last_successful_refresh);
        break;
    case SettingsColumn_manual_sort_list:
        json_object* manual_sort_list_json =
            json_object_new_array_ext(game_id_array_size(settings->manual_sort_list));
        for each(GameId, id, GameIdArray, settings->manual_sort_list) {
            json_object_array_add(manual_sort_list_json, json_object_new_int(id));
        }
        res = sqlite3_bind_json(stmt, 1, manual_sort_list_json);
        json_object_put(manual_sort_list_json);
        break;
    case SettingsColumn_mark_installed_after_add:
        res = sqlite3_bind_int(stmt, 1, settings->mark_installed_after_add);
        break;
    case SettingsColumn_max_connections:
        res = sqlite3_bind_int(stmt, 1, settings->max_connections);
        break;
    case SettingsColumn_max_retries:
        res = sqlite3_bind_int(stmt, 1, settings->max_retries);
        break;
    case SettingsColumn_notifs_show_update_banner:
        res = sqlite3_bind_int(stmt, 1, settings->notifs_show_update_banner);
        break;
    case SettingsColumn_play_gifs:
        res = sqlite3_bind_int(stmt, 1, settings->play_gifs);
        break;
    case SettingsColumn_play_gifs_unfocused:
        res = sqlite3_bind_int(stmt, 1, settings->play_gifs_unfocused);
        break;
    case SettingsColumn_preload_nearby_images:
        res = sqlite3_bind_int(stmt, 1, settings->preload_nearby_images);
        break;
    case SettingsColumn_proxy_type:
        res = sqlite3_bind_int(stmt, 1, settings->proxy_type);
        break;
    case SettingsColumn_proxy_host:
        res = sqlite3_bind_mstring(stmt, 1, settings->proxy_host);
        break;
    case SettingsColumn_proxy_port:
        res = sqlite3_bind_int(stmt, 1, settings->proxy_port);
        break;
    case SettingsColumn_proxy_username:
        res = sqlite3_bind_mstring(stmt, 1, settings->proxy_username);
        break;
    case SettingsColumn_proxy_password:
        res = sqlite3_bind_mstring(stmt, 1, settings->proxy_password);
        break;
    case SettingsColumn_quick_filters:
        res = sqlite3_bind_int(stmt, 1, settings->quick_filters);
        break;
    case SettingsColumn_refresh_archived_games:
        res = sqlite3_bind_int(stmt, 1, settings->refresh_archived_games);
        break;
    case SettingsColumn_refresh_completed_games:
        res = sqlite3_bind_int(stmt, 1, settings->refresh_completed_games);
        break;
    case SettingsColumn_render_when_unfocused:
        res = sqlite3_bind_int(stmt, 1, settings->render_when_unfocused);
        break;
    case SettingsColumn_request_timeout:
        res = sqlite3_bind_int(stmt, 1, settings->request_timeout);
        break;
    case SettingsColumn_rpc_enabled:
        res = sqlite3_bind_int(stmt, 1, settings->rpc_enabled);
        break;
    case SettingsColumn_rpdl_password:
        res = sqlite3_bind_mstring(stmt, 1, settings->rpdl_password);
        break;
    case SettingsColumn_rpdl_token:
        res = sqlite3_bind_mstring(stmt, 1, settings->rpdl_token);
        break;
    case SettingsColumn_rpdl_username:
        res = sqlite3_bind_mstring(stmt, 1, settings->rpdl_username);
        break;
    case SettingsColumn_scroll_amount:
        res = sqlite3_bind_float(stmt, 1, settings->scroll_amount);
        break;
    case SettingsColumn_scroll_smooth:
        res = sqlite3_bind_int(stmt, 1, settings->scroll_smooth);
        break;
    case SettingsColumn_scroll_smooth_speed:
        res = sqlite3_bind_float(stmt, 1, settings->scroll_smooth_speed);
        break;
    case SettingsColumn_select_executable_after_add:
        res = sqlite3_bind_int(stmt, 1, settings->select_executable_after_add);
        break;
    case SettingsColumn_show_remove_btn:
        res = sqlite3_bind_int(stmt, 1, settings->show_remove_btn);
        break;
    case SettingsColumn_software_webview:
        res = sqlite3_bind_int(stmt, 1, settings->software_webview);
        break;
    case SettingsColumn_start_in_background:
        res = sqlite3_bind_int(stmt, 1, settings->start_in_background);
        break;
    case SettingsColumn_start_refresh:
        res = sqlite3_bind_int(stmt, 1, settings->start_refresh);
        break;
    case SettingsColumn_style_accent:
        res = sqlite3_bind_imcolor4(stmt, 1, settings->style_accent);
        break;
    case SettingsColumn_style_alt_bg:
        res = sqlite3_bind_imcolor4(stmt, 1, settings->style_alt_bg);
        break;
    case SettingsColumn_style_bg:
        res = sqlite3_bind_imcolor4(stmt, 1, settings->style_bg);
        break;
    case SettingsColumn_style_border:
        res = sqlite3_bind_imcolor4(stmt, 1, settings->style_border);
        break;
    case SettingsColumn_style_corner_radius:
        res = sqlite3_bind_int(stmt, 1, settings->style_corner_radius);
        break;
    case SettingsColumn_style_text:
        res = sqlite3_bind_imcolor4(stmt, 1, settings->style_text);
        break;
    case SettingsColumn_style_text_dim:
        res = sqlite3_bind_imcolor4(stmt, 1, settings->style_text_dim);
        break;
    case SettingsColumn_table_header_outside_list:
        res = sqlite3_bind_int(stmt, 1, settings->table_header_outside_list);
        break;
    case SettingsColumn_tags_highlights:
        json_object* tags_highlights_json = json_object_new_object();
        for(GameTag tag = GameTag_min(); tag <= GameTag_max(); tag++) {
            if(settings->tags_highlights[tag] == TagHighlight_None) continue;
            char tag_key[4];
            snprintf(tag_key, sizeof(tag_key), "%d", tag);
            json_object_object_add_unique(
                tags_highlights_json,
                tag_key,
                json_object_new_int(settings->tags_highlights[tag]));
        }
        res = sqlite3_bind_json(stmt, 1, tags_highlights_json);
        json_object_put(tags_highlights_json);
        break;
    case SettingsColumn_tex_compress:
        res = sqlite3_bind_int(stmt, 1, settings->tex_compress);
        break;
    case SettingsColumn_tex_compress_replace:
        res = sqlite3_bind_int(stmt, 1, settings->tex_compress_replace);
        break;
    case SettingsColumn_timestamp_format:
        res = sqlite3_bind_mstring(stmt, 1, settings->timestamp_format);
        break;
    case SettingsColumn_unload_offscreen_images:
        res = sqlite3_bind_int(stmt, 1, settings->unload_offscreen_images);
        break;
    case SettingsColumn_vsync:
        res = sqlite3_bind_int(stmt, 1, settings->vsync);
        break;
    case SettingsColumn_weighted_score:
        res = sqlite3_bind_int(stmt, 1, settings->weighted_score);
        break;
    case SettingsColumn_zoom_area:
        res = sqlite3_bind_int(stmt, 1, settings->zoom_area);
        break;
    case SettingsColumn_zoom_enabled:
        res = sqlite3_bind_int(stmt, 1, settings->zoom_enabled);
        break;
    case SettingsColumn_zoom_times:
        res = sqlite3_bind_float(stmt, 1, settings->zoom_times);
        break;
    }
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_*()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}
