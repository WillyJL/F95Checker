#include <globals.h>

#if OS == OS_WINDOWS
Os os = Os_Windows;
#elif OS == OS_LINUX
Os os = Os_Linux;
#elif OS == OS_MACOS
Os os = Os_MacOS;
#endif
BrowserList browsers;
Db* db;
GameDict games;
Gui* gui;
Settings* settings;
CookieDict cookies;
LabelList labels;
TabList tabs;

i32 main(i32 argc, char** argv) {
    UNUSED(argc);
    UNUSED(argv);
    i32 ret = 0;

    browser_list_init(browsers);
    browser_discover_installed(browsers);

    db = db_init();
    if(db == NULL) {
        ret = 1;
        goto exit_db;
    }

    tab_list_init(tabs);
    db_load_tabs(db, tabs);

    label_list_init(labels);
    db_load_labels(db, labels);

    settings = settings_init();
    db_load_settings(db, settings);

    game_dict_init(games);
    db_load_games(db, games);
    settings_after_games_load(settings, games);

    cookie_dict_init(cookies);
    db_load_cookies(db, cookies);

    gui = gui_init();
    if(gui == NULL) {
        ret = 1;
        goto exit_gui;
    }

    // Draw first frame while hidden to avoid flashbang
    gui_tick(gui);
    if(!gui->background_mode) {
        SDL_ShowWindow(gui->window);
    }

    while(!gui_should_close(gui)) {
        gui_tick(gui);
    }

    gui_free(gui);
exit_gui:

    db_free(db);

    cookie_dict_clear(cookies);

    for each(GameDict_pair, pair, GameDict, games) {
        game_free(pair.value);
    }
    game_dict_clear(games);

    settings_free(settings);

    label_list_clear(labels);

    tab_list_clear(tabs);
exit_db:

    browser_list_clear(browsers);

    return ret;
}
