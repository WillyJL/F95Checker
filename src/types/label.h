#pragma once

#include <dcimgui/dcimgui.h>

#include <std.h>

#define LABEL_DFLT_COLOR "#696969"

typedef i32 LabelId;

M_TUPLE_EX_DEF(label, Label, (id, LabelId), (name, m_string_t), (color, ImColor), (position, i32))
#define M_OPL_Label() M_TUPLE_EX_OPL(label, LabelId, m_string_t, ImColor, i32)

M_LIST_DUAL_PUSH_EX_DEF(label_list, LabelList, Label)
#define M_OPL_LabelList() M_LIST_DUAL_PUSH_EX_OPL(label_list, Label)

void label_list_update_positions(LabelList_ptr labels);

M_LIST_DUAL_PUSH_EX_DEF(label_ptr_list, LabelPtrList, Label_ptr, M_PTR_OPLIST)
#define M_OPL_LabelPtrList() M_LIST_DUAL_PUSH_EX_OPL(label_ptr_list, M_PTR_OPLIST)
