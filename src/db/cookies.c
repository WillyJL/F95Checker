#include "cookies.h"

#include "db_i.h"

DB_TABLE_DEFINE(_COOKIES, cookies, CookiesColumn)

void db_do_load_cookies(Db* db, CookieDict_ptr cookies) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &cookies_table);

    // Read all cookies
    m_string_set(sql, "SELECT ");
    db_append_column_names(sql, &cookies_table);
    m_string_cat_printf(sql, " FROM %s", cookies_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    m_string_t key;
    m_string_t value;
    m_string_init(key);
    m_string_init(value);

    assert(sqlite3_column_count(stmt) == cookies_table.columns_count);
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        m_string_set(key, sqlite3_column_text(stmt, CookiesColumn_key));
        m_string_set(value, sqlite3_column_text(stmt, CookiesColumn_value));
        cookie_dict_set_at(cookies, key, value);
    }

    m_string_clear(key);
    m_string_clear(value);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

void db_do_save_cookies(Db* db, CookieDict_ptr cookies) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    // Delete all old cookies
    m_string_printf(sql, "DELETE FROM %s", cookies_table.name);
    res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Save new cookies
    m_string_printf(sql, "INSERT INTO %s (key,value) VALUES (?,?)", cookies_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    for each(CookieDict_pair, cookie, CookieDict, cookies) {
        // key
        res = sqlite3_bind_mstring(stmt, 1, cookie.key);
        db_assert(db, res, SQLITE_OK, "sqlite3_bind_mstring()");

        // value
        res = sqlite3_bind_mstring(stmt, 2, cookie.value);
        db_assert(db, res, SQLITE_OK, "sqlite3_bind_mstring()");

        res = sqlite3_step(stmt);
        db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

        sqlite3_reset(stmt);
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}
