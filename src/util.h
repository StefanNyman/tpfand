#ifndef _UTIL_H
#define _UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DEBUG
#define _assert(c, m)                                                  \
    do {                                                               \
        if (!(c)) {                                                    \
            printf("ASSERT FAILED: %s %s:%d %s\n", __FILE__, __func__, \
                   __LINE__, m);                                       \
            exit(1);                                                   \
        }                                                              \
    } while (false)
#else
#define _assert(c, m) \
    { (void)(c), (void)(m); }
#endif

#ifdef DEBUG
#define DBG(msg) printf("%s %s:%d %s\n", __FILE__, __func__, __LINE__, msg)
#define DBGF(fmt, ...)                                     \
    do {                                                   \
        printf("%s %s:%d ", __FILE__, __func__, __LINE__); \
        printf(fmt, __VA_ARGS__);                          \
        printf("\n");                                      \
    } while (false)
#else
#define DBG(msg) (void)(0)
#define DBGF(fmt, ...) (void)(0)
#endif

#endif
