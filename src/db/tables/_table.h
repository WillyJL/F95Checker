#pragma once

#include <std.h>

typedef struct {
  const char *name;
  const char *type;
  const char *dflt;
  bool primary_key;
  bool autoincrement;
  const char *extra;
} DbColumn;

typedef struct {
  const char *prev;
  const char *name;
} DbRename;

typedef struct {
  const char *name;
  const DbColumn *columns;
  size_t columns_count;
  const DbRename *renames;
  size_t renames_count;
} DbTable;

#define DB_COLUMN_ENUM(enum_name, column, ...) enum_name##_##column,
#define DB_COLUMN_ITEM(enum_name, column, ...)                                 \
  [enum_name##_##column] = {#column, __VA_ARGS__},
#define DB_RENAME_ITEM(empty, old_name, new_name)                              \
  {.prev = #old_name, .name = #new_name},
#define DB_EMPTY(...)

#define DB_TABLE_DECLARE(TABLE_MACRO, table_name, enum_name)                   \
  typedef enum { TABLE_MACRO(enum_name, DB_COLUMN_ENUM, DB_EMPTY) } enum_name;
#define DB_TABLE_DEFINE(TABLE_MACRO, table_name, enum_name)                    \
  static const DbColumn _##table_name##_columns[] = {                          \
      TABLE_MACRO(enum_name, DB_COLUMN_ITEM, DB_EMPTY)};                       \
  static const DbRename _##table_name##_renames[] = {                          \
      TABLE_MACRO(DB_EMPTY, DB_EMPTY, DB_RENAME_ITEM)};                        \
  static const DbTable table_name##_table = {                                  \
      .name = #table_name,                                                     \
      .columns = _##table_name##_columns,                                      \
      .columns_count = COUNT_OF(_##table_name##_columns),                      \
      .renames = _##table_name##_renames,                                      \
      .renames_count = COUNT_OF(_##table_name##_renames),                      \
  };
