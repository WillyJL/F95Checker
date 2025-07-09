#pragma once

#include <std.h>

M_DICT_EX_DEF(cookie_dict, CookieDict, m_string_t, m_string_t)
#define M_OPL_CookieDict() M_DICT_EX_OPL(cookie_dict, M_OPL_m_string_t(), M_OPL_m_string_t())
