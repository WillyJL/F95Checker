#include "settings.h"

void settings_after_games_load(Settings* settings, GameDict_ptr games) {
    for each(GameId, id, GameIdArray, settings->_manual_sort_list_temp) {
        Game** game = game_dict_get(games, id);
        if(game) {
            game_array_push_back(settings->manual_sort_list, *game);
        }
    }
}

Settings* settings_init(void) {
    Settings* settings = malloc(sizeof(Settings));

    m_string_init(settings->browser_custom_arguments);
    m_string_init(settings->browser_custom_executable);
    m_string_init(settings->datestamp_format);
    for(Os os = Os_min(); os <= Os_max(); os++) {
        settings->default_exe_dir[os] = path_init("");
    }
    for(Os os = Os_min(); os <= Os_max(); os++) {
        settings->downloads_dir[os] = path_init("");
    }
    game_array_init(settings->manual_sort_list);
    m_string_init(settings->proxy_host);
    m_string_init(settings->proxy_username);
    m_string_init(settings->proxy_password);
    m_string_init(settings->rpdl_password);
    m_string_init(settings->rpdl_token);
    m_string_init(settings->rpdl_username);
    m_string_init(settings->timestamp_format);
    game_id_array_init(settings->_manual_sort_list_temp);

    return settings;
}

void settings_free(Settings* settings) {
    m_string_clear(settings->browser_custom_arguments);
    m_string_clear(settings->browser_custom_executable);
    m_string_clear(settings->datestamp_format);
    for(Os os = Os_min(); os <= Os_max(); os++) {
        path_free(settings->default_exe_dir[os]);
    }
    for(Os os = Os_min(); os <= Os_max(); os++) {
        path_free(settings->downloads_dir[os]);
    }
    game_array_clear(settings->manual_sort_list);
    m_string_clear(settings->proxy_host);
    m_string_clear(settings->proxy_username);
    m_string_clear(settings->proxy_password);
    m_string_clear(settings->rpdl_password);
    m_string_clear(settings->rpdl_token);
    m_string_clear(settings->rpdl_username);
    m_string_clear(settings->timestamp_format);
    game_id_array_clear(settings->_manual_sort_list_temp);

    free(settings);
}
