#pragma once

#include "types.h"

#include "types/datetime.h"
#include "types/label.h"
#include "types/tab.h"

#include <std.h>

typedef int32_t GameId;

bool GameId_oor_equal(GameId k, char n);
GameId GameId_oor_set(char n);
size_t GameId_hash(GameId n);
#define M_OPL_GameId()                  \
    M_OPEXTEND(                         \
        M_BASIC_OPLIST,                 \
        OOR_EQUAL(GameId_oor_equal),    \
        OOR_SET(API_4(GameId_oor_set)), \
        HASH(GameId_hash))

M_ARRAY_EX_DEF(game_id_array, GameIdArray, GameId)
#define M_OPL_GameIdArray() M_ARRAY_EX_OPL(game_id_array, GameId)

#define GAME_IMAGE_URL_MISSING "missing"

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
    flt32_t score;
    uint32_t votes;
    uint8_t rating;
    m_string_t finished;
    m_string_t installed;
    bool updated;
    bool archived;
    m_string_list_t executables;
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
    uint32_t reviews_total;
    GameReviewList reviews;

    // bool = False selected;
    // "imagehelper.ImageHelper" = None image;
    // list[bool] = None executables_valids;
    // bool = None executables_valid;
    // list[TimelineEvent] = dataclasses.field(default_factory = list) timeline_events;
} Game;

Game* game_init(void);
void game_free(Game* game);

M_DICT_OA_EX_DEF(game_dict, GameDict, GameId, M_OPL_GameId(), Game*, M_PTR_OPLIST)
#define M_OPL_GameDict() M_DICT_OA_EX_OPL(game_dict, M_OPL_GameId(), M_PTR_OPLIST)
