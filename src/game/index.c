#include "game.h"

#include "gui/ui/components/games_list_i.h"

#include <globals.h>

// TODO: i dont like this, find ways to clean up this mess
// array of structs with function pointers would work
// but clang has no warning for incomplete array initializers
// so adding new columns would not give an error here so could forget to add here

static i32 game_index_sort_asc_type(Game* const* a, Game* const* b) {
    return strcasecmp(GameType_toString((*a)->type), GameType_toString((*b)->type));
}
static i32 game_index_sort_desc_type(Game* const* a, Game* const* b) {
    return -strcasecmp(GameType_toString((*a)->type), GameType_toString((*b)->type));
}

static i32 game_index_sort_asc_name(Game* const* a, Game* const* b) {
    return m_string_cmpi((*a)->name, (*b)->name);
}
static i32 game_index_sort_desc_name(Game* const* a, Game* const* b) {
    return -m_string_cmpi((*a)->name, (*b)->name);
}

static i32 game_index_sort_asc_developer(Game* const* a, Game* const* b) {
    return m_string_cmpi((*a)->developer, (*b)->developer);
}
static i32 game_index_sort_desc_developer(Game* const* a, Game* const* b) {
    return -m_string_cmpi((*a)->developer, (*b)->developer);
}

static i32 game_index_sort_asc_last_updated(Game* const* a, Game* const* b) {
    return ((*a)->last_updated < (*b)->last_updated) ? -1 :
                                                       ((*a)->last_updated > (*b)->last_updated);
}
static i32 game_index_sort_desc_last_updated(Game* const* a, Game* const* b) {
    return ((*a)->last_updated > (*b)->last_updated) ? -1 :
                                                       ((*a)->last_updated < (*b)->last_updated);
}

static i32 game_index_sort_asc_last_launched(Game* const* a, Game* const* b) {
    return ((*a)->last_launched < (*b)->last_launched) ?
               -1 :
               ((*a)->last_launched > (*b)->last_launched);
}
static i32 game_index_sort_desc_last_launched(Game* const* a, Game* const* b) {
    return ((*a)->last_launched > (*b)->last_launched) ?
               -1 :
               ((*a)->last_launched < (*b)->last_launched);
}

static i32 game_index_sort_asc_added_on(Game* const* a, Game* const* b) {
    return ((*a)->added_on < (*b)->added_on) ? -1 : ((*a)->added_on > (*b)->added_on);
}
static i32 game_index_sort_desc_added_on(Game* const* a, Game* const* b) {
    return ((*a)->added_on > (*b)->added_on) ? -1 : ((*a)->added_on < (*b)->added_on);
}

static i32 game_index_sort_asc_finished(Game* const* a, Game* const* b) {
    // FIXME: placeholder, sort correctly
    return ((*a)->id < (*b)->id) ? -1 : ((*a)->id > (*b)->id);
}
static i32 game_index_sort_desc_finished(Game* const* a, Game* const* b) {
    // FIXME: placeholder, sort correctly
    return ((*a)->id < (*b)->id) ? -1 : ((*a)->id > (*b)->id);
}

static i32 game_index_sort_asc_installed(Game* const* a, Game* const* b) {
    // FIXME: placeholder, sort correctly
    return ((*a)->id < (*b)->id) ? -1 : ((*a)->id > (*b)->id);
}
static i32 game_index_sort_desc_installed(Game* const* a, Game* const* b) {
    // FIXME: placeholder, sort correctly
    return ((*a)->id < (*b)->id) ? -1 : ((*a)->id > (*b)->id);
}

static i32 game_index_sort_asc_rating(Game* const* a, Game* const* b) {
    return ((*a)->rating < (*b)->rating) ? -1 : ((*a)->rating > (*b)->rating);
}
static i32 game_index_sort_desc_rating(Game* const* a, Game* const* b) {
    return ((*a)->rating > (*b)->rating) ? -1 : ((*a)->rating < (*b)->rating);
}

static i32 game_index_sort_asc_notes(Game* const* a, Game* const* b) {
    return (m_string_empty_p((*a)->notes) || m_string_empty_p((*b)->notes)) ?
               (m_string_empty_p((*a)->notes) > m_string_empty_p((*b)->notes)) :
               m_string_cmpi((*a)->notes, (*b)->notes);
}
static i32 game_index_sort_desc_notes(Game* const* a, Game* const* b) {
    return (m_string_empty_p((*a)->notes) || m_string_empty_p((*b)->notes)) ?
               (m_string_empty_p((*a)->notes) < m_string_empty_p((*b)->notes)) :
               -m_string_cmpi((*a)->notes, (*b)->notes);
}

static i32 game_index_sort_asc_status(Game* const* a, Game* const* b) {
    return ((*a)->status < (*b)->status) ? -1 : ((*a)->status > (*b)->status);
}
static i32 game_index_sort_desc_status(Game* const* a, Game* const* b) {
    return ((*a)->status > (*b)->status) ? -1 : ((*a)->status < (*b)->status);
}

static i32 game_index_sort_asc_score(Game* const* a, Game* const* b) {
    return ((*a)->score < (*b)->score) ? -1 : ((*a)->score > (*b)->score);
}
static i32 game_index_sort_desc_score(Game* const* a, Game* const* b) {
    return ((*a)->score > (*b)->score) ? -1 : ((*a)->score < (*b)->score);
}

static i32 game_index_sort_archived(Game* const* a, Game* const* b) {
    return (*a)->archived > (*b)->archived;
}

static i32 game_index_sort_unchecked(Game* const* a, Game* const* b) {
    return ((*a)->type == GameType_Unchecked) < ((*b)->type == GameType_Unchecked);
}

static void game_index_sort_array(GameArray_ptr game_array, ImGuiTableSortSpecs* sort_specs) {
    if(gui->ui_state.columns_enabled[GamesListColumn_ManualSort]) return;

    // Iterate in reverse for proper multi-sort ordering
    for(i32 i = sort_specs->SpecsCount - 1; i >= 0; i--) {
        const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[i];
        i32 (*sort_func)(Game* const*, Game* const*) = NULL;
        const bool sort_asc = sort_spec->SortDirection == ImGuiSortDirection_Ascending;

        switch((GamesListColumn)sort_spec->ColumnIndex) {
        case GamesListColumn_COUNT:
        case GamesListColumn_ManualSort:
        case GamesListColumn_Version:
        case GamesListColumn_FinishedVersion:
        case GamesListColumn_InstalledVersion:
        case GamesListColumn_StatusAfterName:
        case GamesListColumn_Separator:
        case GamesListColumn_LaunchButton:
        case GamesListColumn_OpenUrlButton:
        case GamesListColumn_CopyUrlButton:
        case GamesListColumn_OpenFolderButton:
            // Not sortable
            break;
        case GamesListColumn_Type:
            sort_func = sort_asc ? &game_index_sort_asc_type : &game_index_sort_desc_type;
            break;
        case GamesListColumn_Name:
            sort_func = sort_asc ? &game_index_sort_asc_name : &game_index_sort_desc_name;
            break;
        case GamesListColumn_Developer:
            sort_func = sort_asc ? &game_index_sort_asc_developer :
                                   &game_index_sort_desc_developer;
            break;
        case GamesListColumn_LastUpdated:
            sort_func = sort_asc ? &game_index_sort_asc_last_updated :
                                   &game_index_sort_desc_last_updated;
            break;
        case GamesListColumn_LastLaunched:
            sort_func = sort_asc ? &game_index_sort_asc_last_launched :
                                   &game_index_sort_desc_last_launched;
            break;
        case GamesListColumn_AddedOn:
            sort_func = sort_asc ? &game_index_sort_asc_added_on : &game_index_sort_desc_added_on;
            break;
        case GamesListColumn_FinishedCheckbox:
            sort_func = sort_asc ? &game_index_sort_asc_finished : &game_index_sort_desc_finished;
            break;
        case GamesListColumn_InstalledCheckbox:
            sort_func = sort_asc ? &game_index_sort_asc_installed :
                                   &game_index_sort_desc_installed;
            break;
        case GamesListColumn_Rating:
            sort_func = sort_asc ? &game_index_sort_asc_rating : &game_index_sort_desc_rating;
            break;
        case GamesListColumn_Notes:
            sort_func = sort_asc ? &game_index_sort_asc_notes : &game_index_sort_desc_notes;
            break;
        case GamesListColumn_StatusOwnColumn:
            sort_func = sort_asc ? &game_index_sort_asc_status : &game_index_sort_desc_status;
            break;
        case GamesListColumn_Score:
            // FIXME: use weighted score if set
            sort_func = sort_asc ? &game_index_sort_asc_score : &game_index_sort_desc_score;
            break;
        }
        if(sort_func != NULL) {
            game_array_special_sort(game_array, sort_func);
        }
    }

    // Group into archived and unchecked
    game_array_special_sort(game_array, game_index_sort_archived);
    game_array_special_sort(game_array, game_index_sort_unchecked);
    // TODO: add option to pin games to top?
}

static void game_index_fill_array(GameArray_ptr game_array) {
    if(gui->ui_state.columns_enabled[GamesListColumn_ManualSort]) {
        bool changed = false;
        for each(GameDict_pair, pair, GameDict, games) {
            bool found = false;
            for each(Game*, game, GameArray, settings->manual_sort_list) {
                if(game == pair.value) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                game_array_push_back(settings->manual_sort_list, pair.value);
                changed = true;
            }
        }
        if(changed) {
            db_save_setting(db, settings, SettingsColumn_manual_sort_list);
        }
        game_array_set(game_array, settings->manual_sort_list);
    } else {
        game_array_reset(game_array);
        for each(GameDict_pair, pair, GameDict, games) {
            Game* game = pair.value;
            game_array_push_back(game_array, game);
        }
    }
}

void game_index_rebuild(GameIndex_ptr game_index, ImGuiTableSortSpecs* sort_specs) {
    GameIndex new_game_index;
    game_index_init(new_game_index);

    GameArray game_array;
    game_array_init(game_array);

    game_index_fill_array(game_array);
    // TODO: also do filtering
    game_index_sort_array(game_array, sort_specs);
    // FIXME: check if sorting each bucket after splitting based on tabs is faster

    GameArray tab_game_array;
    game_array_init(tab_game_array);

    for each(Game*, game, GameArray, game_array) {
        if(game->tab == NULL) {
            game_array_push_back(tab_game_array, game);
        }
    }
    game_index_set_at(new_game_index, TAB_ID_NULL, tab_game_array);

    for each(Tab_ptr, tab, TabList, tabs) {
        game_array_reset(tab_game_array);
        for each(Game*, game, GameArray, game_array) {
            if(game->tab == tab) {
                game_array_push_back(tab_game_array, game);
            }
        }
        game_index_set_at(new_game_index, tab->id, tab_game_array);
    }

    game_array_clear(tab_game_array);

    game_array_clear(game_array);

    game_index_move(game_index, new_game_index);
}
