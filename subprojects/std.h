#pragma once

// IWYU pragma: begin_exports
#include <assert.h>
// #include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <mlib/m-array.h>
#include <mlib/m-bitset.h>
#include <mlib/m-bstring.h>
#include <mlib/m-buffer.h>
#include <mlib/m-dict.h>
#include <mlib/m-list.h>
#include <mlib/m-string.h>
#include <mlib/m-thread.h>
#include <mlib/m-tuple.h>
// #include <stdarg.h>
// #include <stdbool.h>
// #include <stddef.h>
// #include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
// #include <time.h>
#include <unistd.h>
// IWYU pragma: end_exports

#define OS_WINDOWS 1
#define OS_LINUX   2
#define OS_MACOS   3
#if defined(_WIN32)
#define OS OS_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define OS OS_LINUX
#elif defined(__APPLE__)
#define OS OS_MACOS
#else
#error "Platform not supported!"
#endif

static_assert(
    sizeof(float) * CHAR_BIT == 32,
    "float is not 32 bit on this architecture, fix the f32 typedef.");
typedef float f32;

static_assert(
    sizeof(double) * CHAR_BIT == 64,
    "double is not 64 bit on this architecture, fix the f64 typedef.");
typedef double f64;

static_assert(
    sizeof(long double) * CHAR_BIT == 128,
    "long double is not 128 bit on this architecture, fix the f128 typedef.");
typedef long double f128;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    int32_t x;
    int32_t y;
} Vec2;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Vec3;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t w;
} Vec4;

#define CLOCKS_TO_USEC(clocks) (clocks * 1'000'000 / CLOCKS_PER_SEC)

#ifndef MAX
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif

#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#ifndef ABS
#define ABS(a) ({ (a) < 0 ? -(a) : (a); })
#endif

#ifndef ROUND_UP_TO
#define ROUND_UP_TO(a, b)       \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a / _b + !!(_a % _b);  \
    })
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif

#define STR(x)  #x
#define XSTR(x) STR(x)

#define M_ARRAY_EX_DEF(name, name_t, ...)                  \
    M_ARRAY_DEF_AS(name, name_t, name_t##_it, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_ARRAY_EX_OPL(name, type_t) M_ARRAY_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(type_t)())

#define M_BUFFER_EX_DEF(name, name_t, ...)     \
    M_BUFFER_DEF_AS(name, name_t, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_BUFFER_EX_OPL(name, type_t) M_BUFFER_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(type_t)())

#define M_DICT_EX_DEF(name, name_t, ...)                                   \
    M_DICT_DEF2_AS(name, name_t, name_t##_it, name_t##_itref, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;                                       \
    typedef name##_pair_ct name_t##_pair;
#define M_DICT_EX_OPL(name, key_t, value_t) \
    M_DICT_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(key_t)(), M_GLOBAL_OPLIST_OR_DEF(value_t)())

#define M_DICT_OA_EX_DEF(name, name_t, ...)                                   \
    M_DICT_OA_DEF2_AS(name, name_t, name_t##_it, name_t##_itref, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;                                          \
    typedef name##_pair_ct name_t##_pair;
#define M_DICT_OA_EX_OPL(name, key_t, value_t) \
    M_DICT_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(key_t)(), M_GLOBAL_OPLIST_OR_DEF(value_t)())

#define M_LIST_DUAL_PUSH_EX_DEF(name, name_t, ...)                  \
    M_LIST_DUAL_PUSH_DEF_AS(name, name_t, name_t##_it, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_LIST_DUAL_PUSH_EX_OPL(name, type_t) M_LIST_OPLIST(name, M_GLOBAL_OPLIST_OR_DEF(type_t)())

#define M_TUPLE_EX_DEF(name, name_t, ...)      \
    M_TUPLE_DEF2_AS(name, name_t, __VA_ARGS__) \
    typedef name##_ptr name_t##_ptr;
#define M_GLOBAL_OPLIST_OR_DEF_CALL(type_t) M_GLOBAL_OPLIST_OR_DEF(type_t)()
#define M_TUPLE_EX_OPL(name, ...) \
    M_TUPLE_OPLIST(name, M_MAP_C(M_GLOBAL_OPLIST_OR_DEF_CALL, __VA_ARGS__))

/*
 * Based on: https://github.com/P-p-H-d/mlib/blob/5c57ad1f5185446842210934d022266f926a8aeb/example/ex-dict05.c
 * Provide OOR methods for OA Hashmap.
 * See documentation for definition of OOR
 * OOR values are represented as:
 * empty (0) is INT_MIN
 * deleted (1) is INT_MIN + 1
 * So valid range for integers are [INT_MIN+2, INT_MAX]
 */
#define M_INT32_OOR_EQUAL(n, oor) (n == INT32_MIN + oor)
#define M_INT32_OOR_SET(oor)      (INT32_MIN + oor)
#define M_INT_OOR_EQUAL(int)      M_##int##_OOR_EQUAL
#define M_INT_OOR_SET(int)        M_##int##_OOR_SET
#define M_INT_HASH(n)             ((size_t)n) // Identity hash!
#define M_INT_EX_OPL(int)                       \
    M_OPEXTEND(                                 \
        M_BASIC_OPLIST,                         \
        OOR_EQUAL(API_0(M_INT_OOR_EQUAL(int))), \
        OOR_SET(API_4(M_INT_OOR_SET(int))),     \
        HASH(M_INT_HASH))

// Based on M_EACH() but also dereferences item pointer for convenience;
// container and container_t are swapped to resemble variable declaration.
#define each(item_t, item, container_t, container) \
    eachi_oplist(item_t, item, container, M_GLOBAL_OPLIST(container_t))

// clang-format off
/* Internal for each */
#define eachi_oplist(item_t, item, container, oplist)                         \
  M_IF_METHOD(IT_REF, oplist)(eachi, eachi_const)                             \
  (item_t, item, container, oplist, M_C(item, _it),                           \
   M_C(local_cont_, __LINE__), M_C(local_ptr_, __LINE__))

/* Internal for each with M_GET_IT_REF operator */
#define eachi(item_t,item,container,oplist, iterator, cont, ptr)              \
  (bool cont = true; cont; cont = false)                                      \
  for(M_GET_SUBTYPE oplist *ptr; cont ; cont = false)                         \
    for(item_t item; cont ; cont = false)                                     \
      for(M_GET_IT_TYPE oplist iterator; cont ; cont = false)                 \
        for(M_GET_IT_FIRST oplist (iterator, container) ;                     \
            !M_GET_IT_END_P oplist (iterator)                                 \
              && (ptr = M_GET_IT_REF oplist (iterator), item = *ptr, true) ;  \
            !M_GET_IT_END_P oplist (iterator)                                 \
              && (M_GET_IT_NEXT oplist (iterator), true))

/* Internal for each with M_GET_IT_CREF operator */
#define eachi_const(item_t,item,container,oplist, iterator, cont, ptr)        \
  (bool cont = true; cont; cont = false)                                      \
  for(const M_GET_SUBTYPE oplist *ptr; cont ; cont = false)                   \
    for(item_t item; cont ; cont = false)                                     \
      for(M_GET_IT_TYPE oplist iterator; cont ; cont = false)                 \
        for(M_GET_IT_FIRST oplist (iterator, container) ;                     \
            !M_GET_IT_END_P oplist (iterator)                                 \
              && (ptr = M_GET_IT_CREF oplist (iterator), item = *ptr, true) ; \
            !M_GET_IT_END_P oplist (iterator)                                 \
              && (M_GET_IT_NEXT oplist (iterator), true))
// clang-format on

M_LIST_DUAL_PUSH_DEF(m_string_list, m_string_t)
#define M_OPL_m_string_list_t() M_LIST_DUAL_PUSH_EX_OPL(m_string_list, m_string_t)

typedef struct m_bstring_s* m_bstring_ptr;

typedef struct m_eflag_s {
    m_mutex_t mutex;
    m_cond_t cond;
} m_eflag_t[1];
typedef struct m_eflag_s* m_eflag_ptr;

static inline void m_eflag_init(m_eflag_t eflag) {
    m_mutex_init(eflag->mutex);
    m_cond_init(eflag->cond);
}

static inline void m_eflag_wait(m_eflag_t eflag) {
    m_cond_wait(eflag->cond, eflag->mutex);
}

static inline void m_eflag_broadcast(m_eflag_t eflag) {
    m_cond_broadcast(eflag->cond);
}

static inline void m_eflag_clear(m_eflag_t eflag) {
    m_mutex_clear(eflag->mutex);
    m_cond_clear(eflag->cond);
}

static inline void custom_perror(const char* s, const char* e) {
    fprintf(stderr, "%s: %s\n", s, e);
}
