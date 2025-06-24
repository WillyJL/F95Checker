#include "tabs.h"

#include "db_i.h"

DB_TABLE_DEFINE(_TABS, tabs, TabsColumn)

static void db_parse_tab(Db* db, sqlite3_stmt* stmt, Tab_ptr tab) {
    UNUSED(db);
    size_t col = 0;

    tab->id = sqlite3_column_int(stmt, col++);
    m_string_set(tab->name, sqlite3_column_text(stmt, col++));
    m_string_set(tab->icon, sqlite3_column_text(stmt, col++));

    if(sqlite3_column_type(stmt, col) == SQLITE_NULL) {
        tab->color = (ImColor){{0, 0, 0, 0}};
    } else {
        tab->color = sqlite3_column_imcolor(stmt, col);
    }
    col++;

    tab->position = sqlite3_column_int(stmt, col++);
}

void db_do_load_tabs(Db* db, TabList_ptr tabs) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &tabs_table);

    // Read all tabs
    m_string_set(sql, "SELECT ");
    db_append_column_names(sql, &tabs_table);
    m_string_cat_printf(sql, " FROM %s ORDER BY position ASC", tabs_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    assert(sqlite3_column_count(stmt) == tabs_table.columns_count);
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        Tab tab;
        tab_init(tab);
        db_parse_tab(db, stmt, tab);
        tab_list_push_back_move(tabs, &tab);
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);

    tab_list_update_positions(tabs);
}

void db_do_save_tab(Db* db, Tab_ptr tab, TabsColumn column) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "UPDATE %s SET %s=? WHERE id=?",
        tabs_table.name,
        tabs_table.columns[column].name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_bind_int(stmt, 2, tab->id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    switch(column) {
    case TabsColumn_id:
        res = sqlite3_bind_int(stmt, 1, tab->id);
        break;
    case TabsColumn_name:
        res = sqlite3_bind_mstring(stmt, 1, tab->name);
        break;
    case TabsColumn_icon:
        res = sqlite3_bind_mstring(stmt, 1, tab->icon);
        break;
    case TabsColumn_color:
        if(tab->color.Value.w == 0) {
            res = sqlite3_bind_null(stmt, 1);
        } else {
            res = sqlite3_bind_imcolor(stmt, 1, tab->color);
        }
        break;
    case TabsColumn_position:
        res = sqlite3_bind_int(stmt, 1, tab->position);
        break;
    }
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_*()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

Tab_ptr db_do_create_tab(Db* db, TabList_ptr tabs) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(sql, "INSERT INTO %s DEFAULT VALUES RETURNING ", tabs_table.name);
    db_append_column_names(sql, &tabs_table);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    assert(sqlite3_column_count(stmt) == tabs_table.columns_count);
    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_ROW, "sqlite3_step()");

    Tab tab;
    tab_init(tab);
    db_parse_tab(db, stmt, tab);
    tab_list_push_back_move(tabs, &tab);
    Tab_ptr tab_ptr = *tab_list_back(tabs);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);

    tab_list_update_positions(tabs);
    return tab_ptr;
}

void db_do_delete_tab(Db* db, Tab_ptr tab, TabList_ptr tabs) {
    TabId id = tab->id;
    bool removed = false;
    TabList_it it;
    for(tab_list_it(it, tabs); !tab_list_end_p(it); tab_list_next(it)) {
        if(*tab_list_cref(it) == tab) {
            tab_list_remove(tabs, it);
            removed = true;
            break;
        }
    }
    assert(removed);
    UNUSED(removed);

    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(sql, "DELETE FROM %s WHERE id=?", tabs_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_bind_int(stmt, 1, id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);

    tab_list_update_positions(tabs);
}
