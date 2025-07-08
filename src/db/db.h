#pragma once

#include "tables/_tables.h"

#include "game/game.h"
#include "settings/settings.h"
#include "types/cookie.h"
#include "types/label.h"
#include "types/tab.h"

#include <std.h>

typedef struct Db Db;

Db* db_init(void);

void db_backup(Db* db);

void db_load_settings(Db* db, Settings* settings);
void db_save_setting(Db* db, Settings* settings, SettingsColumn column);

void db_load_games(Db* db, GameDict_ptr games);
void db_save_game(Db* db, Game* game, GamesColumn column);
Game* db_create_game(Db* db, GameDict_ptr games, GameId id);
void db_delete_game(Db* db, Game* game, GameDict_ptr games);

GameTimelineEvent_ptr db_create_game_timeline_event(
    Db* db,
    Game* game,
    GameTimelineEventType type,
    m_string_list_ptr arguments);

void db_load_tabs(Db* db, TabList_ptr tabs);
void db_save_tab(Db* db, Tab_ptr tab, TabsColumn column);
Tab_ptr db_create_tab(Db* db, TabList_ptr tabs);
void db_delete_tab(Db* db, Tab_ptr tab, TabList_ptr tabs);

void db_load_labels(Db* db, LabelList_ptr labels);
void db_save_label(Db* db, Label_ptr label, LabelsColumn column);
Label_ptr db_create_label(Db* db, LabelList_ptr labels);
void db_delete_label(Db* db, Label_ptr label, LabelList_ptr labels);

void db_load_cookies(Db* db, CookieDict_ptr cookies);
void db_save_cookies(Db* db, CookieDict_ptr cookies);

void db_free(Db* db);
