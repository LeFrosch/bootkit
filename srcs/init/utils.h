#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

// converts any macro expansion to a string
#define _STR(x) #x
#define STR(x) _STR(x)

// exits if the call returns non-zero
#define try(call)                                                                                                      \
    do {                                                                                                               \
        if ((call) != 0) {                                                                                             \
            error(STR(__LINE__) ": " #call " failed", errno);                                                          \
        }                                                                                                              \
    } while (false)

[[noreturn]] static inline void error(const char* msg, int err) {
    fprintf(stderr, "ERROR: %s: %s (%d)\n", msg, strerror(err < 0 ? -err : err), err);
    exit(EXIT_FAILURE);
}
