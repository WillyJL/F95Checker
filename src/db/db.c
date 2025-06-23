#include "db.h"

#include "db_i.h"

#include "path/path.h"

#include <globals.h>

#define DB_FILE "temp.sqlite3" // TODO: change when its ready

static void db_thread(void* ctx);

Db* db_init(void) {
    Db* db = malloc(sizeof(Db));
    int32_t res;

    res = sqlite3_initialize();
    if(res != SQLITE_OK) {
        custom_perror("sqlite3_initialize()", sqlite3_errstr(res));
        free(db);
        return NULL;
    }

    db->path = path_init_data_dir();
    path_join(db->path, DB_FILE);
    res = sqlite3_open(path_cstr(db->path), &db->conn);

    if(res != SQLITE_OK) {
        db_perror(db, "sqlite3_open()");
        sqlite3_close(db->conn);
        path_free(db->path);
        free(db);
        return NULL;
    }

    // TODO: make note somewhere that legacy json and ini config (pre v9.0) are no longer migrated,
    // need v11.x to migrate to sqlite3 from those legacy configs, then this will work fine

    db->name = strdup(sqlite3_db_name(db->conn, 0));

    // Use WAL mode for more efficient writes
    res = sqlite3_exec(db->conn, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Consolidate WAL into DB every 100 frames, usually means ~100 write operations
    res = sqlite3_wal_autocheckpoint(db->conn, 100);
    db_assert(db, res, SQLITE_OK, "sqlite3_wal_autocheckpoint()");

    db->did_migration_backup = false;
    db_message_queue_init(db->queue, 100);
    m_thread_create(db->thread, db_thread, db);

    return db;
}

static void db_send_message_async(Db* db, const DbMessage message) {
    db_message_queue_push(db->queue, message);
}

static void db_send_message_blocking(Db* db, DbMessage message) {
    m_eflag_t eflag;
    m_eflag_init(eflag);
    message.eflag = eflag;
    db_send_message_async(db, message);
    m_eflag_wait(eflag);
    m_eflag_clear(eflag);
}

static void db_append_backup_id(m_string_ptr str) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    struct tm* tp = localtime(&ts.tv_sec);
    m_string_t id_str;
    m_string_init_printf(
        id_str,
        "_v%s_%04d%02d%02d_%02d%02d%02d_%d",
        F95CHECKER_VERSION,
        tp->tm_year + 1900,
        tp->tm_mon + 1,
        tp->tm_mday,
        tp->tm_hour,
        tp->tm_min,
        tp->tm_sec,
        ts.tv_nsec);
    m_string_replace_all_cstr(id_str, ".", "");
    m_string_cat(str, id_str);
    m_string_clear(id_str);
}

void db_backup(Db* db) {
    const DbMessage message = {
        .type = DbMessageType_Backup,
    };
    db_send_message_blocking(db, message);
}

static void db_do_backup(Db* db) {
    int32_t res;

    res = sqlite3_wal_checkpoint_v2(db->conn, db->name, SQLITE_CHECKPOINT_TRUNCATE, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_wal_checkpoint_v2()");

    Path* backup = path_dup(db->path);
    m_string_t name;
    m_string_init_set(name, path_name(backup));
    db_append_backup_id(name);
    path_set_name(backup, m_string_get_cstr(name));
    m_string_clear(name);

    m_bstring_t bytes;
    m_bstring_init(bytes);
    if(path_read_bytes(db->path, bytes) && path_write_bytes(backup, bytes)) {
        printf("Saved DB backup: %s\n", path_cstr(backup));
    } else {
        custom_perror(path_cstr(backup), "DB backup failed");
    }
    m_bstring_clear(bytes);

    path_free(backup);
}

void db_append_column_names(m_string_ptr sql, const DbTable* table) {
    for(size_t col = 0; col < table->columns_count; col++) {
        m_string_cat(sql, table->columns[col].name);
        m_string_cat(sql, ",");
    }
    m_string_strim(sql, ",");
}

static void db_append_column_spec(m_string_ptr sql, const DbColumn* column) {
    m_string_cat_printf(sql, "%s %s", column->name, column->type);
    if(column->dflt) {
        m_string_cat_printf(sql, " DEFAULT %s", column->dflt);
    }
    if(column->primary_key) {
        m_string_cat(sql, " PRIMARY KEY");
        if(column->autoincrement) {
            m_string_cat(sql, " AUTOINCREMENT");
        }
    }
    if(column->extra) {
        m_string_cat_printf(sql, " %s", column->extra);
    }
}

static void db_migration_prelude(Db* db, const char* format, ...) {
    if(!db->did_migration_backup) {
        printf("Saving DB backup before schema migration...\n");
        db_do_backup(db);
        db->did_migration_backup = true;
    }
    printf("DB schema migration: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void db_create_table(Db* db, const DbTable* table) {
    int32_t res;
    m_string_t sql;
    m_string_init(sql);
    sqlite3_stmt* stmt;

    // Create table
    m_string_printf(sql, "CREATE TABLE IF NOT EXISTS %s (", table->name);
    for(size_t col = 0; col < table->columns_count; col++) {
        const DbColumn* column = &table->columns[col];
        db_append_column_spec(sql, column);
        m_string_cat(sql, ",");
    }
    m_string_strim(sql, ",");
    m_string_cat(sql, ")");

    res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Rename columns
    for(size_t ren = 0; ren < table->renames_count; ren++) {
        const DbRename* rename = &table->renames[ren];
        res = sqlite3_table_column_metadata(
            db->conn,
            db->name,
            table->name,
            rename->prev,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);
        if(res == SQLITE_ERROR) {
            continue; // Doesn't exist, nothing to rename
        }
        db_migration_prelude(
            db,
            "Renaming '%s.%s' to '%s.%s'",
            table->name,
            rename->prev,
            table->name,
            rename->name);

        m_string_printf(
            sql,
            "ALTER TABLE %s RENAME COLUMN %s TO %s",
            table->name,
            rename->prev,
            rename->name);
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");
    }

    // Add columns
    for(size_t col = 0; col < table->columns_count; col++) {
        const DbColumn* column = &table->columns[col];
        res = sqlite3_table_column_metadata(
            db->conn,
            db->name,
            table->name,
            column->name,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);
        if(res == SQLITE_OK) {
            continue; // Already exists, nothing to add
        }
        db_migration_prelude(db, "Adding '%s.%s'", table->name, column->name);

        m_string_printf(sql, "ALTER TABLE %s ADD COLUMN ", table->name);
        db_append_column_spec(sql, column);
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");
    }

    // Remove columns
    m_string_printf(sql, "PRAGMA table_info(%s)", table->name);
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        const char* found_column = sqlite3_column_text(stmt, 1);
        bool is_known = false;
        for(size_t col = 0; col < table->columns_count; col++) {
            const DbColumn* known_column = &table->columns[col];
            if(strcmp(known_column->name, found_column) == 0) {
                is_known = true;
                break;
            }
        }
        if(is_known) {
            continue; // Should exist, nothing to remove
        }
        db_migration_prelude(db, "Removing '%s.%s'", table->name, found_column);

        m_string_printf(sql, "ALTER TABLE %s DROP COLUMN %s", table->name, found_column);
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    // Update column defs
    m_string_printf(sql, "PRAGMA table_info(%s)", table->name);
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    bool recreate = false;
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        const char* found_column = sqlite3_column_text(stmt, 1);
        const DbColumn* column = NULL;
        for(size_t col = 0; col < table->columns_count; col++) {
            const DbColumn* known_column = &table->columns[col];
            if(strcmp(known_column->name, found_column) == 0) {
                column = known_column;
                break;
            }
        }
        assert(column != NULL);

        const char* found_type = sqlite3_column_text(stmt, 2);
        const char* found_dflt = sqlite3_column_text(stmt, 4);
        bool changed_type = (column->type == NULL) != (found_type == NULL);
        bool changed_dflt = (column->dflt == NULL) != (found_dflt == NULL);
        if(column->type != NULL && found_type != NULL) {
            changed_type = strcasecmp(column->type, found_type) != 0;
        }
        if(column->dflt != NULL && found_dflt != NULL) {
            changed_type = strcasecmp(column->dflt, found_dflt) != 0;
        }
        bool changed_primary_key = column->primary_key != (sqlite3_column_int(stmt, 5) != 0);

        if(changed_type || changed_dflt || changed_primary_key) {
            recreate = true;
            break;
        }
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    if(recreate) {
        db_migration_prelude(db, "Recreating '%s' table with new types/defaults", table->name);

        m_string_t table_temp;
        m_string_init_printf(table_temp, "%s_temp", table->name);
        db_append_backup_id(table_temp);

        m_string_printf(
            sql,
            "ALTER TABLE %s RENAME TO %s",
            table->name,
            m_string_get_cstr(table_temp));
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

        db_create_table(db, table);

        m_string_printf(sql, "INSERT INTO %s (", table->name);
        db_append_column_names(sql, table);
        m_string_cat(sql, ") SELECT ");
        db_append_column_names(sql, table);
        m_string_cat_printf(sql, " FROM %s", m_string_get_cstr(table_temp));
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

        m_string_printf(sql, "DROP TABLE %s", m_string_get_cstr(table_temp));
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

        m_string_clear(table_temp);
    }

    m_string_clear(sql);
}

ImColor sqlite3_column_imcolor(sqlite3_stmt* stmt, int32_t col) {
    const char* hex_color = sqlite3_column_text(stmt, col);
    uint8_t r, g, b;
    int32_t res = sscanf(hex_color, "#%02hhx%02hhx%02hhx", &r, &g, &b);
    if(res != 3) {
        r = g = b = 255;
    }
    ImColor im_color = {{
        .x = (flt32_t)r / 255,
        .y = (flt32_t)g / 255,
        .z = (flt32_t)b / 255,
        .w = 1.0,
    }};
    return im_color;
}

int32_t sqlite3_bind_imcolor(sqlite3_stmt* stmt, int32_t param, ImColor im_color) {
    char hex_color[8];
    int32_t res = snprintf(
        hex_color,
        sizeof(hex_color),
        "#%02hhx%02hhx%02hhx",
        (uint8_t)(im_color.Value.x * 255),
        (uint8_t)(im_color.Value.y * 255),
        (uint8_t)(im_color.Value.z * 255));
    if(res != sizeof(hex_color) - 1) {
        strlcpy(hex_color, "#FFFFFF", sizeof(hex_color));
    }
    return sqlite3_bind_text(stmt, param, hex_color, -1, SQLITE_TRANSIENT);
}

void db_load_settings(Db* db, Settings* settings) {
    const DbMessage message = {
        .type = DbMessageType_LoadSettings,
        .load.settings = settings,
    };
    db_send_message_blocking(db, message);
}

void db_save_setting(Db* db, Settings* settings, SettingsColumn column) {
    const DbMessage message = {
        .type = DbMessageType_SaveSetting,
        .save.setting =
            {
                .ptr = settings,
                .column = column,
            },
    };
    db_send_message_async(db, message);
}

void db_load_games(Db* db, GameDict_ptr games) {
    const DbMessage message = {
        .type = DbMessageType_LoadGames,
        .load.games = games,
    };
    db_send_message_blocking(db, message);
}

void db_save_game(Db* db, Game* game, GamesColumn column) {
    const DbMessage message = {
        .type = DbMessageType_SaveGame,
        .save.game =
            {
                .ptr = game,
                .column = column,
            },
    };
    db_send_message_async(db, message);
}

Game* db_create_game(Db* db, GameDict_ptr games, GameId id) {
    Game* game;
    const DbMessage message = {
        .type = DbMessageType_CreateGame,
        .create.game =
            {
                .games = games,
                .id = id,
                .out = &game,
            },
    };
    db_send_message_blocking(db, message);
    return game;
}

void db_delete_game(Db* db, Game* game, GameDict_ptr games) {
    const DbMessage message = {
        .type = DbMessageType_DeleteGame,
        .delete.game =
            {
                .ptr = game,
                .games = games,
            },
    };
    db_send_message_blocking(db, message);
}

GameTimelineEvent_ptr db_create_game_timeline_event(
    Db* db,
    Game* game,
    GameTimelineEventType type,
    m_string_list_ptr arguments) {
    GameTimelineEvent_ptr timeline_event;
    const DbMessage message = {
        .type = DbMessageType_CreateGameTimelineEvent,
        .create.game_timeline_event =
            {
                .game = game,
                .type = type,
                .arguments = arguments,
                .out = &timeline_event,
            },
    };
    db_send_message_blocking(db, message);
    return timeline_event;
}

void db_load_tabs(Db* db, TabList_ptr tabs) {
    const DbMessage message = {
        .type = DbMessageType_LoadTabs,
        .load.tabs = tabs,
    };
    db_send_message_blocking(db, message);
}

void db_save_tab(Db* db, Tab_ptr tab, TabsColumn column) {
    const DbMessage message = {
        .type = DbMessageType_SaveTab,
        .save.tab =
            {
                .ptr = tab,
                .column = column,
            },
    };
    db_send_message_async(db, message);
}

Tab_ptr db_create_tab(Db* db, TabList_ptr tabs) {
    Tab_ptr tab;
    const DbMessage message = {
        .type = DbMessageType_CreateTab,
        .create.tab =
            {
                .tabs = tabs,
                .out = &tab,
            },
    };
    db_send_message_blocking(db, message);
    return tab;
}

void db_delete_tab(Db* db, Tab_ptr tab, TabList_ptr tabs) {
    const DbMessage message = {
        .type = DbMessageType_DeleteTab,
        .delete.tab =
            {
                .ptr = tab,
                .tabs = tabs,
            },
    };
    db_send_message_blocking(db, message);
}

void db_load_labels(Db* db, LabelList_ptr labels) {
    const DbMessage message = {
        .type = DbMessageType_LoadLabels,
        .load.labels = labels,
    };
    db_send_message_blocking(db, message);
}

void db_save_label(Db* db, Label_ptr label, LabelsColumn column) {
    const DbMessage message = {
        .type = DbMessageType_SaveLabel,
        .save.label =
            {
                .ptr = label,
                .column = column,
            },
    };
    db_send_message_async(db, message);
}

Label_ptr db_create_label(Db* db, LabelList_ptr labels) {
    Label_ptr label;
    const DbMessage message = {
        .type = DbMessageType_CreateLabel,
        .create.label =
            {
                .labels = labels,
                .out = &label,
            },
    };
    db_send_message_blocking(db, message);
    return label;
}

void db_delete_label(Db* db, Label_ptr label, LabelList_ptr labels) {
    const DbMessage message = {
        .type = DbMessageType_DeleteLabel,
        .delete.label =
            {
                .ptr = label,
                .labels = labels,
            },
    };
    db_send_message_blocking(db, message);
}

void db_load_cookies(Db* db, CookieDict_ptr cookies) {
    const DbMessage message = {
        .type = DbMessageType_LoadCookies,
        .load.cookies = cookies,
    };
    db_send_message_blocking(db, message);
}

void db_save_cookies(Db* db, CookieDict_ptr cookies) {
    const DbMessage message = {
        .type = DbMessageType_SaveCookies,
        .save.cookies =
            {
                .ptr = cookies,
            },
    };
    db_send_message_blocking(db, message);
}

static void db_thread(void* ctx) {
    Db* db = ctx;
    bool quit = false;

    DbMessage message;
    while(db_message_queue_pop_blocking(&message, db->queue, !quit)) {
        switch(message.type) {
        case DbMessageType_Quit:
            quit = true;
            break;
        case DbMessageType_Backup:
            db_do_backup(db);
            break;

        case DbMessageType_LoadSettings:
            db_do_load_settings(db, message.load.settings);
            break;
        case DbMessageType_SaveSetting:
            db_do_save_setting(db, message.save.setting.ptr, message.save.setting.column);
            break;

        case DbMessageType_LoadGames:
            db_do_load_games(db, message.load.games);
            break;
        case DbMessageType_SaveGame:
            db_do_save_game(db, message.save.game.ptr, message.save.game.column);
            break;
        case DbMessageType_CreateGame:
            *message.create.game.out =
                db_do_create_game(db, message.create.game.games, message.create.game.id);
            break;
        case DbMessageType_DeleteGame:
            db_do_delete_game(db, message.delete.game.ptr, message.delete.game.games);
            break;

        case DbMessageType_CreateGameTimelineEvent:
            *message.create.game_timeline_event.out = db_do_create_game_timeline_event(
                db,
                message.create.game_timeline_event.game,
                message.create.game_timeline_event.type,
                message.create.game_timeline_event.arguments);
            break;

        case DbMessageType_LoadTabs:
            db_do_load_tabs(db, message.load.tabs);
            break;
        case DbMessageType_SaveTab:
            db_do_save_tab(db, message.save.tab.ptr, message.save.tab.column);
            break;
        case DbMessageType_CreateTab:
            *message.create.tab.out = db_do_create_tab(db, message.create.tab.tabs);
            break;
        case DbMessageType_DeleteTab:
            db_do_delete_tab(db, message.delete.tab.ptr, message.delete.tab.tabs);
            break;

        case DbMessageType_LoadLabels:
            db_do_load_labels(db, message.load.labels);
            break;
        case DbMessageType_SaveLabel:
            db_do_save_label(db, message.save.label.ptr, message.save.label.column);
            break;
        case DbMessageType_CreateLabel:
            *message.create.label.out = db_do_create_label(db, message.create.label.labels);
            break;
        case DbMessageType_DeleteLabel:
            db_do_delete_label(db, message.delete.label.ptr, message.delete.label.labels);
            break;

        case DbMessageType_LoadCookies:
            db_do_load_cookies(db, message.load.cookies);
            break;
        case DbMessageType_SaveCookies:
            db_do_save_cookies(db, message.save.cookies.ptr);
            break;
        }

        if(message.eflag != NULL) {
            m_eflag_broadcast(message.eflag);
        }
    }
}

void db_free(Db* db) {
    const DbMessage message = {
        .type = DbMessageType_Quit,
    };
    db_send_message_async(db, message);
    m_thread_join(db->thread);
    db_message_queue_clear(db->queue);

    free((char*)db->name);
    sqlite3_close(db->conn);
    path_free(db->path);
    sqlite3_shutdown();
    free(db);
}
