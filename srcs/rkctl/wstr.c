#include <stdlib.h>
#include <string.h>

#include "wstr.h"

size_t wstr_len(const wchar* wstr) {
    size_t len = 0;
    while (wstr[len] != 0) {
        len++;
    }

    return len;
}

size_t wstr_size(const wchar* wstr) {
    return (wstr_len(wstr) + 1) * 2;
}

const char* wstr_to_cstr(const wchar* wstr) {
    if (wstr == nullptr) {
        return nullptr;
    }

    const auto len = wstr_len(wstr);

    auto cstr = (char*)malloc(len + 1);
    if (cstr == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < len; i++) {
        cstr[i] = (char)wstr[i];
    }
    cstr[len] = 0;

    return cstr;
}

const wchar* wstr_from_cstr(const char* cstr) {
    if (cstr == nullptr) {
        return nullptr;
    }

    const auto len = strlen(cstr);

    auto wstr = (uint16_t*)malloc((len + 1) * 2);
    if (wstr == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < len; i++) {
        wstr[i] = (uint16_t)cstr[i];
    }
    wstr[len] = 0;

    return wstr;
}
