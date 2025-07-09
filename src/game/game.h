#pragma once

#include "types.h"

#include "types/datetime.h"
#include "types/label.h"
#include "types/tab.h"

#include <std.h>

#define GAME_IMAGE_URL_MISSING "missing"

typedef i32 GameId;
#define M_OPL_GameId() M_INT_EX_OPL(i32)

M_ARRAY_EX_DEF(game_id_array, GameIdArray, GameId)
#define M_OPL_GameIdArray() M_ARRAY_EX_OPL(game_id_array, M_OPL_GameId())

typedef struct {
    GameId id;
    bool custom;
    m_string_t name;
    m_string_t version;
    m_string_t developer;
    GameType type;
    GameStatus status;
    m_string_t url;
    Datestamp added_on;
    Datestamp last_updated;
    Timestamp last_full_check;
    m_string_t last_check_version;
    Datestamp last_launched;
    f32 score;
    u32 votes;
    f32 rating;
    m_string_t finished;
    m_string_t installed;
    bool updated;
    bool archived;
    GameExecutableList executables;
    m_string_t description;
    m_string_t changelog;
    m_bitset_t tags;
    m_string_list_t unknown_tags;
    bool unknown_tags_flag;
    LabelPtrList labels;
    Tab_ptr tab;
    m_string_t notes;
    m_string_t image_url;
    m_string_list_t previews_urls;
    GameDownloadList downloads;
    u32 reviews_total;
    GameReviewList reviews;
    GameTimelineEventList timeline_events;

    // bool = False selected;
    // "imagehelper.ImageHelper" = None image;
    // list[bool] = None executables_valids;
    // bool = None executables_valid;
} Game;

Game* game_init(void);
void game_free(Game* game);

M_ARRAY_EX_DEF(game_array, GameArray, Game*, M_PTR_OPLIST)
#define M_OPL_GameArray() M_ARRAY_EX_OPL(game_array, M_PTR_OPLIST)

M_DICT_OA_EX_DEF(game_index, GameIndex, TabId, GameArray)
#define M_OPL_GameIndex() M_DICT_OA_EX_OPL(game_index, M_OPL_TabId(), M_OPL_GameArray())

M_DICT_OA_EX_DEF(game_dict, GameDict, GameId, M_OPL_GameId(), Game*, M_PTR_OPLIST)
#define M_OPL_GameDict() M_DICT_OA_EX_OPL(game_dict, M_OPL_GameId(), M_PTR_OPLIST)
