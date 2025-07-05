#include "labels.h"

#include "db_i.h"

DB_TABLE_DEFINE(_LABELS, labels, LabelsColumn)

static void db_parse_label(Db* db, sqlite3_stmt* stmt, Label_ptr label) {
    UNUSED(db);
    size_t col = 0;

    label->id = sqlite3_column_int(stmt, col++);
    m_string_set(label->name, sqlite3_column_text(stmt, col++));
    label->color = sqlite3_column_imcolor4(stmt, col++);
    label->position = sqlite3_column_int(stmt, col++);
}

void db_do_load_labels(Db* db, LabelList_ptr labels) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &labels_table);

    // Read all labels
    m_string_set(sql, "SELECT ");
    db_append_column_names(sql, &labels_table);
    m_string_cat_printf(sql, " FROM %s ORDER BY position ASC", labels_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    assert(sqlite3_column_count(stmt) == labels_table.columns_count);
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        Label label;
        label_init(label);
        db_parse_label(db, stmt, label);
        label_list_push_back_move(labels, &label);
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);

    label_list_update_positions(labels);
}

void db_do_save_label(Db* db, Label_ptr label, LabelsColumn column) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "UPDATE %s SET %s=? WHERE id=?",
        labels_table.name,
        labels_table.columns[column].name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_bind_int(stmt, 2, label->id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    switch(column) {
    case LabelsColumn_id:
        res = sqlite3_bind_int(stmt, 1, label->id);
        break;
    case LabelsColumn_name:
        res = sqlite3_bind_mstring(stmt, 1, label->name);
        break;
    case LabelsColumn_color:
        res = sqlite3_bind_imcolor4(stmt, 1, label->color);
        break;
    case LabelsColumn_position:
        res = sqlite3_bind_int(stmt, 1, label->position);
        break;
    }
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_*()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

Label_ptr db_do_create_label(Db* db, LabelList_ptr labels) {
    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(sql, "INSERT INTO %s DEFAULT VALUES RETURNING ", labels_table.name);
    db_append_column_names(sql, &labels_table);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    assert(sqlite3_column_count(stmt) == labels_table.columns_count);
    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_ROW, "sqlite3_step()");

    Label label;
    label_init(label);
    db_parse_label(db, stmt, label);
    label_list_push_back_move(labels, &label);
    Label_ptr label_ptr = *label_list_back(labels);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);

    label_list_update_positions(labels);
    return label_ptr;
}

void db_do_delete_label(Db* db, Label_ptr label, LabelList_ptr labels) {
    LabelId id = label->id;
    bool removed = false;
    LabelList_it it;
    for(label_list_it(it, labels); !label_list_end_p(it); label_list_next(it)) {
        if(*label_list_cref(it) == label) {
            label_list_remove(labels, it);
            removed = true;
            break;
        }
    }
    assert(removed);
    UNUSED(removed);

    i32 res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(sql, "DELETE FROM %s WHERE id=?", labels_table.name);
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

    label_list_update_positions(labels);
}
