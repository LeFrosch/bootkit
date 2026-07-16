#pragma once

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
        const auto __ret = call;                                                                                       \
        if (__ret != 0) {                                                                                              \
            error(STR(__LINE__) ": " #call " failed", __ret);                                                          \
        }                                                                                                              \
    } while (false)

// report the error and exit with 1
[[noreturn]] static inline void error(const char* msg, int err) {
    fprintf(stderr, "ERROR: %s: %s (%d)\n", msg, strerror(err < 0 ? -err : err), err);
    exit(EXIT_FAILURE);
}

static void __attribute__((unused)) auto_close_file(const void* ptr) {
    const auto file = *(FILE**)ptr;
    if (file != nullptr) {
        fclose(file);
    }
}

#define __close_file __attribute__((cleanup(auto_close_file)))

static void __attribute__((unused)) auto_free(const void* ptr) {
    const auto file = *(void**)ptr;
    if (file != nullptr) {
        free(file);
    }
}

#define __free __attribute__((cleanup(auto_free)))
