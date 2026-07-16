#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint16_t wchar;

size_t wstr_len(const wchar* str);

size_t wstr_size(const wchar* str);

const char* wstr_to_cstr(const wchar* str);

const wchar* wstr_from_cstr(const char* str);
