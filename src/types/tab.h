#pragma once

#include <dcimgui/dcimgui.h>
#include <fonts/mdi.h>

#include <std.h>

#define TAB_DFLT_ICON mdi_heart_box

typedef i32 TabId;

M_TUPLE_EX_DEF(
    tab,
    Tab,
    (id, TabId),
    (name, m_string_t),
    (icon, m_string_t),
    (color, ImColor4),
    (position, i32))
#define M_OPL_Tab() M_TUPLE_EX_OPL(tab, TabId, m_string_t, m_string_t, ImColor4, i32)

M_LIST_DUAL_PUSH_EX_DEF(tab_list, TabList, Tab)
#define M_OPL_TabList() M_LIST_DUAL_PUSH_EX_OPL(tab_list, Tab)

void tab_list_update_positions(TabList_ptr tabs);
