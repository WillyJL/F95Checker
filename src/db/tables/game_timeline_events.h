#pragma once

#include "_table.h"

#include <std.h>

#define _GAME_TIMELINE_EVENTS($, COLUMN, RENAME)          \
    COLUMN($, game_id, .type = "INTEGER", .dflt = "NULL") \
    COLUMN($, type, .type = "INTEGER", .dflt = "1")       \
    COLUMN($, timestamp, .type = "INTEGER", .dflt = "0")  \
    COLUMN($, arguments, .type = "TEXT", .dflt = "'[]'")
DB_TABLE_DECLARE(_GAME_TIMELINE_EVENTS, timeline_events, GameTimelineEventsColumn)
