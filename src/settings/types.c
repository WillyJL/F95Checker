#include "types.h"

#include <fonts/mdi.h>

SMARTENUM_DEFINE(_DisplayMode, DisplayMode)
const DisplayModeInfo display_mode[1 + DisplayMode_COUNT] = {
    [DisplayMode_List] = {.icon = mdi_view_agenda_outline},
    [DisplayMode_Grid] = {.icon = mdi_view_grid_outline},
    [DisplayMode_Kanban] = {.icon = mdi_view_week_outline},
};

SMARTENUM_DEFINE(_Os, Os)

SMARTENUM_DEFINE(_ProxyType, ProxyType)

SMARTENUM_DEFINE(_TagHighlight, TagHighlight)
const TagHighlightInfo tag_highlight[TagHighlight_COUNT] = {
    [TagHighlight_None] = {.color = {{0.3f, 0.3f, 0.3f, 1.0f}}},
    [TagHighlight_Positive] = {.color = {{0.0f, 0.6f, 0.0f, 1.0f}}},
    [TagHighlight_Negative] = {.color = {{0.6f, 0.0f, 0.0f, 1.0f}}},
    [TagHighlight_Critical] = {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
};

SMARTENUM_DEFINE(_TexCompress, TexCompress)
