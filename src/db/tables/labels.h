#pragma once

#include "_table.h"

#include <std.h>

#define _LABELS($, COLUMN, RENAME)                                               \
    COLUMN($, id, .type = "INTEGER", .primary_key = true, .autoincrement = true) \
    COLUMN($, name, .type = "TEXT", .dflt = "''")                                \
    COLUMN($, color, .type = "TEXT", .dflt = "'" LABEL_DFLT_COLOR "'")           \
    COLUMN($, position, .type = "INTEGER", .dflt = "0")
DB_TABLE_DECLARE(_LABELS, labels, LabelsColumn)
