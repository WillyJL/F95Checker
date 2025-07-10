#pragma once

#include "../../gui.h"

#include <std.h>

#define _GamesListColumn(_, $)  \
    _($, ManualSort, 0)         \
    _($, Version, 1)            \
    _($, FinishedVersion, 2)    \
    _($, InstalledVersion, 3)   \
    _($, StatusAfterName, 4)    \
    _($, Separator, 5)          \
    _($, LaunchButton, 6)       \
    _($, Type, 7)               \
    _($, Name, 8)               \
    _($, Developer, 9)          \
    _($, LastUpdated, 10)       \
    _($, LastLaunched, 11)      \
    _($, AddedOn, 12)           \
    _($, FinishedCheckbox, 13)  \
    _($, InstalledCheckbox, 14) \
    _($, Rating, 15)            \
    _($, Notes, 16)             \
    _($, OpenUrlButton, 17)     \
    _($, CopyUrlButton, 18)     \
    _($, OpenFolderButton, 19)  \
    _($, StatusOwnColumn, 20)   \
    _($, Score, 21)
SMARTENUM_DECLARE(_GamesListColumn, GamesListColumn)
typedef struct {
    const char* icon;
    const char* label;
    const char* header;
    ImGuiTableColumnFlags flags;
    bool ghost;
} GamesListColumnInfo;
extern const GamesListColumnInfo games_list_column[GamesListColumn_COUNT];
static_assert(
    COUNT_OF(((Gui*)0)->ui_state.columns_enabled) == GamesListColumn_COUNT,
    "Update the column count too, would you kindly?");

void gui_ui_games_list_tick_columns(Gui* gui, bool draw_header);
