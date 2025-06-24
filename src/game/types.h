#pragma once

#include "types/datetime.h"

#include <dcimgui/dcimgui.h>
#include <smart_enum/smart_enum.h>

#include <std.h>

#define _GameCategory(_, $) \
    _($, Games, 1)          \
    _($, Media, 2)          \
    _($, Misc, 3)
SMARTENUM_DECLARE(_GameCategory, GameCategory)

M_TUPLE_EX_DEF(game_download_link, GameDownloadLink, (label, m_string_t), (url, m_string_t))
#define M_OPL_GameDownloadLink() M_TUPLE_EX_OPL(game_download_link, m_string_t, m_string_t)

M_LIST_DUAL_PUSH_EX_DEF(game_download_link_list, GameDownloadLinkList, GameDownloadLink)
#define M_OPL_GameDownloadLinkList() \
    M_LIST_DUAL_PUSH_EX_OPL(game_download_link_list, GameDownloadLink)

M_TUPLE_EX_DEF(game_download, GameDownload, (text, m_string_t), (links, GameDownloadLinkList))
#define M_OPL_GameDownload() M_TUPLE_EX_OPL(game_download, m_string_t, GameDownloadLinkList)

M_LIST_DUAL_PUSH_EX_DEF(game_download_list, GameDownloadList, GameDownload)
#define M_OPL_GameDownloadList() M_LIST_DUAL_PUSH_EX_OPL(game_download_list, GameDownload)

M_LIST_DUAL_PUSH_EX_DEF(game_executable_list, GameExecutableList, m_string_list_t)
#define M_OPL_GameExecutableList() M_LIST_DUAL_PUSH_EX_OPL(game_executable_list, m_string_list_t)

M_TUPLE_EX_DEF(
    game_review,
    GameReview,
    (user, m_string_t),
    (score, u8),
    (message, m_string_t),
    (likes, u32),
    (timestamp, Timestamp))
#define M_OPL_GameReview() M_TUPLE_EX_OPL(game_review, m_string_t, u8, m_string_t, u32, Timestamp)

M_LIST_DUAL_PUSH_EX_DEF(game_review_list, GameReviewList, GameReview)
#define M_OPL_GameReviewList() M_LIST_DUAL_PUSH_EX_OPL(game_review_list, GameReview)

#define _GameStatus(_, $) \
    _($, Normal, 1)       \
    _($, Completed, 2)    \
    _($, OnHold, 3)       \
    _($, Abandoned, 4)    \
    _($, Unchecked, 5)    \
    _($, Custom, 6)
SMARTENUM_DECLARE(_GameStatus, GameStatus)
typedef struct {
    ImColor color;
    const char* icon;
} GameStatusInfo;
extern const GameStatusInfo game_status[1 + GameStatus_COUNT];

#define _GameTag(_, $)             \
    _($, 2dGame, 1)                \
    _($, 2dcg, 2)                  \
    _($, 3dGame, 3)                \
    _($, 3dcg, 4)                  \
    _($, Adventure, 5)             \
    _($, Ahegao, 6)                \
    _($, AiCg, 140)                \
    _($, AnalSex, 7)               \
    _($, Animated, 8)              \
    _($, AssetAddon, 9)            \
    _($, AssetAiShoujo, 10)        \
    _($, AssetAnimal, 11)          \
    _($, AssetAnimation, 12)       \
    _($, AssetAudio, 13)           \
    _($, AssetBundle, 14)          \
    _($, AssetCharacter, 15)       \
    _($, AssetClothing, 16)        \
    _($, AssetDazGen1, 151)        \
    _($, AssetDazGen2, 141)        \
    _($, AssetDazGen3, 142)        \
    _($, AssetDazGen8, 143)        \
    _($, AssetDazGen81, 144)       \
    _($, AssetDazGen9, 145)        \
    _($, AssetDazM4, 152)          \
    _($, AssetDazV4, 153)          \
    _($, AssetEnvironment, 17)     \
    _($, AssetExpression, 18)      \
    _($, AssetFemale, 146)         \
    _($, AssetHair, 19)            \
    _($, AssetHdri, 20)            \
    _($, AssetHoneySelect, 21)     \
    _($, AssetHoneySelect2, 22)    \
    _($, AssetKoikatu, 23)         \
    _($, AssetLight, 24)           \
    _($, AssetMale, 147)           \
    _($, AssetMorph, 25)           \
    _($, AssetNonbinary, 148)      \
    _($, AssetPlayhome, 150)       \
    _($, AssetPlugin, 26)          \
    _($, AssetPose, 27)            \
    _($, AssetProp, 28)            \
    _($, AssetScene, 149)          \
    _($, AssetScript, 29)          \
    _($, AssetShader, 30)          \
    _($, AssetTexture, 31)         \
    _($, AssetUtility, 32)         \
    _($, AssetVehicle, 33)         \
    _($, Bdsm, 34)                 \
    _($, Bestiality, 35)           \
    _($, BigAss, 36)               \
    _($, BigTits, 37)              \
    _($, Blackmail, 38)            \
    _($, Bukkake, 39)              \
    _($, Censored, 40)             \
    _($, CharacterCreation, 41)    \
    _($, Cheating, 42)             \
    _($, Combat, 43)               \
    _($, Corruption, 44)           \
    _($, Cosplay, 45)              \
    _($, Creampie, 46)             \
    _($, DatingSim, 47)            \
    _($, Dilf, 48)                 \
    _($, Drugs, 49)                \
    _($, DystopianSetting, 50)     \
    _($, Exhibitionism, 51)        \
    _($, Fantasy, 52)              \
    _($, FemaleProtagonist, 54)    \
    _($, Femaledomination, 53)     \
    _($, Footjob, 55)              \
    _($, Furry, 56)                \
    _($, FutaTrans, 57)            \
    _($, FutaTransProtagonist, 58) \
    _($, Gay, 59)                  \
    _($, GraphicViolence, 60)      \
    _($, Groping, 61)              \
    _($, GroupSex, 62)             \
    _($, Handjob, 63)              \
    _($, Harem, 64)                \
    _($, Horror, 65)               \
    _($, Humiliation, 66)          \
    _($, Humor, 67)                \
    _($, Incest, 68)               \
    _($, InternalView, 69)         \
    _($, Interracial, 70)          \
    _($, JapaneseGame, 71)         \
    _($, KineticNovel, 72)         \
    _($, Lactation, 73)            \
    _($, Lesbian, 74)              \
    _($, Loli, 75)                 \
    _($, MaleProtagonist, 77)      \
    _($, Maledomination, 76)       \
    _($, Management, 78)           \
    _($, Masturbation, 79)         \
    _($, Milf, 80)                 \
    _($, MindControl, 81)          \
    _($, MobileGame, 82)           \
    _($, Monster, 83)              \
    _($, MonsterGirl, 84)          \
    _($, MultipleEndings, 85)      \
    _($, MultiplePenetration, 86)  \
    _($, MultipleProtagonist, 87)  \
    _($, Necrophilia, 88)          \
    _($, NoSexualContent, 89)      \
    _($, Ntr, 90)                  \
    _($, OralSex, 91)              \
    _($, Paranormal, 92)           \
    _($, Parody, 93)               \
    _($, Platformer, 94)           \
    _($, PointClick, 95)           \
    _($, Possession, 96)           \
    _($, Pov, 97)                  \
    _($, Pregnancy, 98)            \
    _($, Prostitution, 99)         \
    _($, Puzzle, 100)              \
    _($, Rape, 101)                \
    _($, RealPorn, 102)            \
    _($, Religion, 103)            \
    _($, Romance, 104)             \
    _($, Rpg, 105)                 \
    _($, Sandbox, 106)             \
    _($, Scat, 107)                \
    _($, SchoolSetting, 108)       \
    _($, SciFi, 109)               \
    _($, SexToys, 110)             \
    _($, SexualHarassment, 111)    \
    _($, Shooter, 112)             \
    _($, Shota, 113)               \
    _($, SideScroller, 114)        \
    _($, Simulator, 115)           \
    _($, Sissification, 116)       \
    _($, Slave, 117)               \
    _($, SleepSex, 118)            \
    _($, Spanking, 119)            \
    _($, Strategy, 120)            \
    _($, Stripping, 121)           \
    _($, Superpowers, 122)         \
    _($, Swinging, 123)            \
    _($, Teasing, 124)             \
    _($, Tentacles, 125)           \
    _($, TextBased, 126)           \
    _($, Titfuck, 127)             \
    _($, Trainer, 128)             \
    _($, Transformation, 129)      \
    _($, Trap, 130)                \
    _($, TurnBasedCombat, 131)     \
    _($, Twins, 132)               \
    _($, Urination, 133)           \
    _($, VaginalSex, 134)          \
    _($, Virgin, 135)              \
    _($, VirtualReality, 136)      \
    _($, Voiced, 137)              \
    _($, Vore, 138)                \
    _($, Voyeurism, 139)
SMARTENUM_DECLARE(_GameTag, GameTag)
typedef struct {
    const char* text;
} GameTagInfo;
extern const GameTagInfo game_tag[1 + GameTag_COUNT];

#define _GameTimelineEventType(_, $)      \
    _($, Added, 1)                        \
    _($, Launched, 2)                     \
    _($, Finished, 3)                     \
    _($, Installed, 4)                    \
    _($, ChangedName, 5)                  \
    _($, ChangedStatus, 6)                \
    _($, ChangedVersion, 7)               \
    _($, ChangedDeveloper, 8)             \
    _($, ChangedType, 9)                  \
    _($, TagsAdded, 10)                   \
    _($, TagsRemoved, 11)                 \
    _($, ScoreIncreased, 12)              \
    _($, ScoreDecreased, 13)              \
    _($, RecheckExpired, 14) /* Unused */ \
    _($, RecheckUserReq, 15)
SMARTENUM_DECLARE(_GameTimelineEventType, GameTimelineEventType)
typedef struct {
    const char* display;
    const char* icon;
    u8 args_min;
    const char* template;
} GameTimelineEventTypeInfo;
extern const GameTimelineEventTypeInfo game_timeline_event_type[1 + GameTimelineEventType_COUNT];

M_TUPLE_EX_DEF(
    game_timeline_event,
    GameTimelineEvent,
    (type, GameTimelineEventType),
    (timestamp, Timestamp),
    (arguments, m_string_list_t))
#define M_OPL_GameTimelineEvent() \
    M_TUPLE_EX_OPL(game_timeline_event, GameTimelineEventType, Timestamp, m_string_list_t)

M_LIST_DUAL_PUSH_EX_DEF(game_timeline_event_list, GameTimelineEventList, GameTimelineEvent)
#define M_OPL_GameTimelineEventList() \
    M_LIST_DUAL_PUSH_EX_OPL(game_timeline_event_list, GameTimelineEvent)

#define _GameType(_, $) \
    _($, ADRIFT, 2)     \
    _($, Flash, 4)      \
    _($, HTML, 5)       \
    _($, Java, 6)       \
    _($, Others, 9)     \
    _($, QSP, 10)       \
    _($, RAGS, 11)      \
    _($, RenPy, 14)     \
    _($, RPGM, 13)      \
    _($, Tads, 16)      \
    _($, Unity, 19)     \
    _($, UnrealEng, 20) \
    _($, WebGL, 21)     \
    _($, WolfRPG, 22)   \
    _($, CG, 30)        \
    _($, Collection, 7) \
    _($, Comics, 24)    \
    _($, GIF, 25)       \
    _($, Manga, 26)     \
    _($, Pinup, 27)     \
    _($, SiteRip, 28)   \
    _($, Video, 29)     \
    _($, CheatMod, 3)   \
    _($, Mod, 8)        \
    _($, README, 12)    \
    _($, Request, 15)   \
    _($, Tool, 17)      \
    _($, Tutorial, 18)  \
    _($, Misc, 1)       \
    _($, Unchecked, 23)
SMARTENUM_DECLARE(_GameType, GameType)
typedef struct {
    ImColor color;
    GameCategory category;
} GameTypeInfo;
extern const GameTypeInfo game_type[1 + GameType_COUNT];
