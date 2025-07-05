// Remove imgui.h defines before dcimgui.h re-defines them
#undef IMGUI_CHECKVERSION
#undef IM_ALLOC
#undef IM_FREE

// 32-bit characters for drawing
#define IMGUI_USE_WCHAR32 1

// Max amount of items for ListClipper before 32bit int coordinates cause issues
#define IMGUI_LISTCLIPPER_MAX 1'000'000

// ImVec4 alias to distinguish color types without wrapping in ImColor
#ifndef __cplusplus
typedef struct ImVec4_t ImColor4;
#define ImGui_PushStyleColor4 ImGui_PushStyleColorImVec4

// Define OPLIST for use with M*LIB types
#define M_OPL_ImColor4() M_POD_OPLIST
#endif
