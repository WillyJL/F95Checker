#pragma once

#include "../../gui.h"

#include <std.h>

bool gui_ui_checkbox(Gui* gui, const char* label, bool* value);
void gui_ui_datestamp(Gui* gui, Datestamp datestamp, const char* alt);
void gui_ui_game_added_on(Gui* gui, Game* game);
void gui_ui_game_copy_url_button(Gui* gui, Game* game);
void gui_ui_game_developer(Gui* gui, Game* game);
void gui_ui_game_finished_checkbox(Gui* gui, Game* game);
void gui_ui_game_installed_checkbox(Gui* gui, Game* game);
void gui_ui_game_last_launched(Gui* gui, Game* game);
void gui_ui_game_last_updated(Gui* gui, Game* game);
void gui_ui_game_launch_button(Gui* gui, Game* game);
void gui_ui_game_name(Gui* gui, Game* game);
void gui_ui_game_notes(Gui* gui, Game* game);
void gui_ui_game_open_folder_button(Gui* gui, Game* game);
void gui_ui_game_open_url_button(Gui* gui, Game* game);
void gui_ui_game_rating(Gui* gui, Game* game);
void gui_ui_game_remove_button(Gui* gui, Game* game);
void gui_ui_game_status(Gui* gui, Game* game);
void gui_ui_game_score(Gui* gui, Game* game);
void gui_ui_game_type(Gui* gui, Game* game);
bool gui_ui_input_text(Gui* gui, const char* label, m_string_ptr str, ImGuiInputTextFlags flags);
