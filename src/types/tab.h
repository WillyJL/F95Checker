#pragma once

#include <dcimgui/dcimgui.h>
#include <fonts/mdi.h>

#include <std.h>

#define TAB_ID_NULL -1

#define TAB_DFLT_NAME "New Tab"
#define TAB_DFLT_ICON mdi_heart_box

typedef i32 TabId;
#define M_OPL_TabId() M_INT_EX_OPL(i32)

M_TUPLE_EX_DEF(
    tab,
    Tab,
    (id, TabId),
    (name, m_string_t),
    (icon, m_string_t),
    (color, ImColor4),
    (position, i32))
#define M_OPL_Tab()         \
    M_TUPLE_EX_OPL(         \
        tab,                \
        M_OPL_TabId(),      \
        M_OPL_m_string_t(), \
        M_OPL_m_string_t(), \
        M_OPL_ImColor4(),   \
        M_OPL_i32())

M_LIST_DUAL_PUSH_EX_DEF(tab_list, TabList, Tab)
#define M_OPL_TabList() M_LIST_DUAL_PUSH_EX_OPL(tab_list, M_OPL_Tab())

void tab_list_update_positions(TabList_ptr tabs);
