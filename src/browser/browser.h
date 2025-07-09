#pragma once

#include <std.h>

typedef i64 BrowserHash;
#define M_OPL_BrowserHash() M_INT_EX_OPL(i64)

typedef enum {
    BrowserReservedHashIntegrated = (BrowserHash)0,
    BrowserReservedHashCustom = (BrowserHash)-1,
} BrowserReservedHash;

M_TUPLE_EX_DEF(
    browser,
    Browser,
    (name, m_string_t),
    (hash, BrowserHash),
    (args_regular, m_string_list_t),
    (args_private, m_string_list_t))
#define M_OPL_Browser()          \
    M_TUPLE_EX_OPL(              \
        browser,                 \
        M_OPL_m_string_t(),      \
        M_OPL_BrowserHash(),     \
        M_OPL_m_string_list_t(), \
        M_OPL_m_string_list_t())

M_LIST_DUAL_PUSH_EX_DEF(browser_list, BrowserList, Browser)
#define M_OPL_BrowserList() M_LIST_DUAL_PUSH_EX_OPL(browser_list, M_OPL_Browser())

void browser_discover_installed(BrowserList_ptr browsers);
