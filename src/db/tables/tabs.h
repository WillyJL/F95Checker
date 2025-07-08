#pragma once

#include "_table.h"

#include <std.h>

#define _TABS($, COLUMN, RENAME)                                                 \
    COLUMN($, id, .type = "INTEGER", .primary_key = true, .autoincrement = true) \
    COLUMN($, name, .type = "TEXT", .dflt = "'" TAB_DFLT_NAME "'")               \
    COLUMN($, icon, .type = "TEXT", .dflt = "'" TAB_DFLT_ICON "'")               \
    COLUMN($, color, .type = "TEXT", .dflt = "NULL")                             \
    COLUMN($, position, .type = "INTEGER", .dflt = "0")
DB_TABLE_DECLARE(_TABS, tabs, TabsColumn)
