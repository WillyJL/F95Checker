#pragma once

#include "_table.h"

#include <std.h>

#define _COOKIES($, COLUMN, RENAME)                     \
    COLUMN($, key, .type = "TEXT", .primary_key = true) \
    COLUMN($, value, .type = "TEXT", .dflt = "''")
DB_TABLE_DECLARE(_COOKIES, cookies, CookiesColumn)
