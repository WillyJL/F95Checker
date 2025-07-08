#include "../db_i.h"

DB_TABLE_DEFINE(_GAME_TIMELINE_EVENTS, timeline_events, GameTimelineEventsColumn)

static void
    db_parse_game_timeline_event(Db* db, sqlite3_stmt* stmt, GameTimelineEvent_ptr timeline_event) {
    UNUSED(db);
    size_t col = 1; // Skip game_id column

    timeline_event->type = sqlite3_column_int(stmt, col++);
    timeline_event->timestamp = sqlite3_column_int64(stmt, col++);

    json_object* arguments_json = sqlite3_column_json(stmt, col++);
    for(size_t i = 0; i < json_object_array_length(arguments_json); i++) {
        json_object* argument = json_object_array_get_idx(arguments_json, i);
        m_string_t argument_str;
        if(!json_object_is_type(argument, json_type_string)) {
            m_string_init_set(argument_str, json_object_to_json_string(argument));
        } else {
            m_string_init_set(argument_str, json_object_get_string(argument));
        }
        m_string_list_push_back_move(timeline_event->arguments, &argument_str);
    }
    json_object_put(arguments_json);
}

void db_do_load_game_timeline_events(Db* db, GameDict_ptr games) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &timeline_events_table);

    // Read all timeline events
    m_string_set(sql, "SELECT ");
    db_append_column_names(sql, &timeline_events_table);
    m_string_cat_printf(
        sql,
        " FROM %s ORDER BY game_id ASC, timestamp DESC",
        timeline_events_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    GameIdArray unknown_game_ids;
    game_id_array_init(unknown_game_ids);

    Game* game = NULL;
    assert(sqlite3_column_count(stmt) == timeline_events_table.columns_count);
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");

        GameId game_id = sqlite3_column_int(stmt, GameTimelineEventsColumn_game_id);
        if(game == NULL || game->id != game_id) {
            Game** game_ptr = game_dict_get(games, game_id);
            if(game_ptr == NULL) {
                bool is_duplicate = false;
                for each(GameId, unknown_game_id, GameIdArray, unknown_game_ids) {
                    if(unknown_game_id != game_id) continue;
                    is_duplicate = true;
                    break;
                }
                if(!is_duplicate) {
                    game_id_array_push_back(unknown_game_ids, game_id);
                }
                continue;
            }
            game = *game_ptr;
        }

        GameTimelineEvent timeline_event;
        game_timeline_event_init(timeline_event);
        db_parse_game_timeline_event(db, stmt, timeline_event);
        game_timeline_event_list_push_back_move(game->timeline_events, &timeline_event);
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    for each(GameId, unknown_game_id, GameIdArray, unknown_game_ids) {
        db_do_delete_game_timeline_events(db, unknown_game_id);
    }
    game_id_array_clear(unknown_game_ids);

    m_string_clear(sql);
}

GameTimelineEvent_ptr db_do_create_game_timeline_event(
    Db* db,
    Game* game,
    GameTimelineEventType type,
    m_string_list_ptr arguments) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "INSERT INTO %s (game_id,type,timestamp,arguments) VALUES (?,?,?,?) RETURNING ",
        timeline_events_table.name);
    db_append_column_names(sql, &timeline_events_table);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    // game_id
    res = sqlite3_bind_int(stmt, 1, game->id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    // type
    res = sqlite3_bind_int(stmt, 2, type);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    // timestamp
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    res = sqlite3_bind_int64(stmt, 3, ts.tv_sec);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    // arguments
    json_object* arguments_json = json_object_new_array();
    for each(m_string_ptr, argument, m_string_list_t, arguments) {
        json_object_array_add(arguments_json, json_object_new_string(m_string_get_cstr(argument)));
    }
    res = sqlite3_bind_json(stmt, 4, arguments_json);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_mstring()");
    json_object_put(arguments_json);

    assert(sqlite3_column_count(stmt) == timeline_events_table.columns_count);
    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_ROW, "sqlite3_step()");

    GameTimelineEvent timeline_event;
    game_timeline_event_init(timeline_event);
    db_parse_game_timeline_event(db, stmt, timeline_event);
    game_timeline_event_list_push_front_move(game->timeline_events, &timeline_event);
    GameTimelineEvent_ptr timeline_event_ptr =
        *game_timeline_event_list_front(game->timeline_events);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
    return timeline_event_ptr;
}

void db_do_delete_game_timeline_events(Db* db, GameId game_id) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(sql, "DELETE FROM %s WHERE game_id=?", timeline_events_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_bind_int(stmt, 1, game_id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}
