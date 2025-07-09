#pragma once

#include "db.h"

#include "tables/_tables_i.h" // IWYU pragma: export

#include <dcimgui/dcimgui.h>
#include <json-c/json.h>
#include <sqlite3/sqlite3.h>

#include <std.h>

typedef struct {
    enum {
        DbMessageType_Quit,
        DbMessageType_Backup,

        DbMessageType_LoadSettings,
        DbMessageType_SaveSetting,

        DbMessageType_LoadGames,
        DbMessageType_SaveGame,
        DbMessageType_CreateGame,
        DbMessageType_DeleteGame,

        DbMessageType_CreateGameTimelineEvent,

        DbMessageType_LoadTabs,
        DbMessageType_SaveTab,
        DbMessageType_CreateTab,
        DbMessageType_DeleteTab,

        DbMessageType_LoadLabels,
        DbMessageType_SaveLabel,
        DbMessageType_CreateLabel,
        DbMessageType_DeleteLabel,

        DbMessageType_LoadCookies,
        DbMessageType_SaveCookies,
    } type;
    m_eflag_ptr eflag;
    union {
        union {
            Settings* settings;
            GameDict_ptr games;
            TabList_ptr tabs;
            LabelList_ptr labels;
            CookieDict_ptr cookies;
        } load;
        union {
            struct {
                Settings* ptr;
                SettingsColumn column;
            } setting;
            struct {
                Game* ptr;
                GamesColumn column;
            } game;
            struct {
                Tab_ptr ptr;
                TabsColumn column;
            } tab;
            struct {
                Label_ptr ptr;
                LabelsColumn column;
            } label;
            struct {
                CookieDict_ptr ptr;
            } cookies;
        } save;
        union {
            struct {
                GameDict_ptr games;
                GameId id;
                Game** out;
            } game;
            struct {
                Game* game;
                GameTimelineEventType type;
                m_string_list_ptr arguments;
                GameTimelineEvent_ptr* out;
            } game_timeline_event;
            struct {
                TabList_ptr tabs;
                Tab_ptr* out;
            } tab;
            struct {
                LabelList_ptr labels;
                Label_ptr* out;
            } label;
        } create;
        union {
            struct {
                Game* ptr;
                GameDict_ptr games;
            } game;
            struct {
                Tab_ptr ptr;
                TabList_ptr tabs;
            } tab;
            struct {
                Label_ptr ptr;
                LabelList_ptr labels;
            } label;
        } delete;
    };
} DbMessage;
#define M_OPL_DbMessage() M_POD_OPLIST

// TODO: check if 100 queue is really useful
M_BUFFER_EX_DEF(db_message_queue, DbMessageQueue, DbMessage, 100, M_BUFFER_QUEUE)
#define M_OPL_DbMessageQueue() M_BUFFER_EX_OPL(db_message_queue, M_OPL_DbMessage())

struct Db {
    Path* path;
    sqlite3* conn;
    const char* name;
    m_thread_t thread;
    DbMessageQueue queue;
    bool did_migration_backup;
};

static inline void db_perror(Db* db, const char* s) {
    custom_perror(s, sqlite3_errmsg(db->conn));
}

#define db_assert(db, res, exp, s) \
    if(res != exp) {               \
        db_perror(db, s);          \
    }                              \
    assert(res == exp)

void db_create_table(Db* db, const DbTable* table);
void db_append_column_names(m_string_ptr sql, const DbTable* table);

#define sqlite3_column_count(pStmt)    (size_t)sqlite3_column_count(pStmt)
#define sqlite3_column_text(pStmt, i)  (const char*)sqlite3_column_text(pStmt, i)
#define sqlite3_column_float(pStmt, i) (float)sqlite3_column_double(pStmt, i)
#define sqlite3_column_json(pStmt, i)  json_tokener_parse(sqlite3_column_text(pStmt, i))
ImColor4 sqlite3_column_imcolor4(sqlite3_stmt* stmt, i32 col);
#define sqlite3_bind_float(pStmt, i, rValue) sqlite3_bind_double(pStmt, i, (double)rValue)
#define sqlite3_bind_mstring(pStmt, i, str) \
    sqlite3_bind_text(pStmt, i, m_string_get_cstr(str), -1, SQLITE_TRANSIENT)
#define sqlite3_bind_json(pStmt, i, json)                             \
    sqlite3_bind_text(                                                \
        pStmt,                                                        \
        i,                                                            \
        json_object_to_json_string_ext(                               \
            json,                                                     \
            JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOSLASHESCAPE), \
        -1,                                                           \
        SQLITE_TRANSIENT)
#define json_object_object_add_unique(obj, key, val) \
    json_object_object_add_ex(obj, key, val, JSON_C_OBJECT_ADD_KEY_IS_NEW)
i32 sqlite3_bind_imcolor4(sqlite3_stmt* stmt, i32 param, ImColor4 im_color);
