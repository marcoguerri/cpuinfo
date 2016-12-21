#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#define __STDC_FORMAT_MACROS

#ifdef DEBUG
#define debug(mess, ...) fprintf(stderr, "[%s:%d at %s] DEBUG: " mess "\n", __FILE__, __LINE__, __TIME__, ##__VA_ARGS__)
#else
#define debug(mess, ...)
#endif

#define error(mess, ...) fprintf(stderr, "[%s:%d] ERROR: " mess "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define CHECK(func, mess, retcode, ...)              \
        do {                                         \
            if(!(func)) {                            \
               error(mess, ##__VA_ARGS__);           \
                return retcode;                      \
                                                     \
            }                                        \
        } while(0)

#endif
