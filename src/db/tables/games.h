#pragma once

#include "_table.h"

#include <std.h>

#define _GAMES($, COLUMN, RENAME)                                                \
    COLUMN($, id, .type = "INTEGER", .primary_key = true)                        \
    COLUMN($, custom, .type = "INTEGER", .dflt = "NULL")                         \
    COLUMN($, name, .type = "TEXT", .dflt = "''")                                \
    COLUMN($, version, .type = "TEXT", .dflt = "'Unchecked'")                    \
    COLUMN($, developer, .type = "TEXT", .dflt = "''")                           \
    COLUMN($, type, .type = "INTEGER", .dflt = "23") /* GameType_Unchecked */    \
    COLUMN($, status, .type = "INTEGER", .dflt = "5") /* GameStatus_Unchecked */ \
    COLUMN($, url, .type = "TEXT", .dflt = "''")                                 \
    COLUMN($, added_on, .type = "INTEGER", .dflt = "0")                          \
    COLUMN($, last_updated, .type = "INTEGER", .dflt = "0")                      \
    COLUMN($, last_full_check, .type = "INTEGER", .dflt = "0")                   \
    COLUMN($, last_check_version, .type = "TEXT", .dflt = "''")                  \
    COLUMN($, last_launched, .type = "INTEGER", .dflt = "0")                     \
    COLUMN($, score, .type = "REAL", .dflt = "0")                                \
    COLUMN($, votes, .type = "INTEGER", .dflt = "0")                             \
    COLUMN($, rating, .type = "REAL", .dflt = "0")                               \
    COLUMN($, finished, .type = "TEXT", .dflt = "''")                            \
    COLUMN($, installed, .type = "TEXT", .dflt = "''")                           \
    COLUMN($, updated, .type = "INTEGER", .dflt = "NULL")                        \
    COLUMN($, archived, .type = "INTEGER", .dflt = "0")                          \
    COLUMN($, executables, .type = "TEXT", .dflt = "'[]'")                       \
    COLUMN($, description, .type = "TEXT", .dflt = "''")                         \
    COLUMN($, changelog, .type = "TEXT", .dflt = "''")                           \
    COLUMN($, tags, .type = "TEXT", .dflt = "'[]'")                              \
    COLUMN($, unknown_tags, .type = "TEXT", .dflt = "'[]'")                      \
    COLUMN($, unknown_tags_flag, .type = "INTEGER", .dflt = "0")                 \
    COLUMN($, labels, .type = "TEXT", .dflt = "'[]'")                            \
    COLUMN($, tab, .type = "INTEGER", .dflt = "NULL")                            \
    COLUMN($, notes, .type = "TEXT", .dflt = "''")                               \
    COLUMN($, image_url, .type = "TEXT", .dflt = "''")                           \
    COLUMN($, previews_urls, .type = "TEXT", .dflt = "'[]'")                     \
    COLUMN($, downloads, .type = "TEXT", .dflt = "'[]'")                         \
    COLUMN($, reviews_total, .type = "INTEGER", .dflt = "0")                     \
    COLUMN($, reviews, .type = "TEXT", .dflt = "'[]'")                           \
    RENAME($, executable, executables)                                           \
    RENAME($, last_full_refresh, last_full_check)                                \
    RENAME($, last_refresh_version, last_check_version)                          \
    RENAME($, played, finished)                                                  \
    RENAME($, last_played, last_launched)
DB_TABLE_DECLARE(_GAMES, games, GamesColumn)
