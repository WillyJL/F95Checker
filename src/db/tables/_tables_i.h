#pragma once

#include "../db.h"

#include "game/game.h"
#include "settings/settings.h"
#include "types/cookie.h"
#include "types/label.h"
#include "types/tab.h"

#include <std.h>

void db_do_load_cookies(Db* db, CookieDict_ptr cookies);
void db_do_save_cookies(Db* db, CookieDict_ptr cookies);

void db_do_load_game_timeline_events(Db* db, GameDict_ptr games);
GameTimelineEvent_ptr db_do_create_game_timeline_event(
    Db* db,
    Game* game,
    GameTimelineEventType type,
    m_string_list_ptr arguments);
void db_do_delete_game_timeline_events(Db* db, GameId game_id);

void db_do_load_games(Db* db, GameDict_ptr games);
void db_do_save_game(Db* db, Game* game, GamesColumn column);
Game* db_do_create_game(Db* db, GameDict_ptr games, GameId id);
void db_do_delete_game(Db* db, Game* game, GameDict_ptr games);

void db_do_load_labels(Db* db, LabelList_ptr labels);
void db_do_save_label(Db* db, Label_ptr label, LabelsColumn column);
Label_ptr db_do_create_label(Db* db, LabelList_ptr labels);
void db_do_delete_label(Db* db, Label_ptr label, LabelList_ptr labels);

void db_do_load_settings(Db* db, Settings* settings);
void db_do_save_setting(Db* db, Settings* settings, SettingsColumn column);

void db_do_load_tabs(Db* db, TabList_ptr tabs);
void db_do_save_tab(Db* db, Tab_ptr tab, TabsColumn column);
Tab_ptr db_do_create_tab(Db* db, TabList_ptr tabs);
void db_do_delete_tab(Db* db, Tab_ptr tab, TabList_ptr tabs);
