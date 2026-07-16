#pragma once

#ifdef LOGGING

#ifdef _GNU_SOURCE
// Linux / musl-gcc application
#include <stdio.h>
#define log(msg, ...) printf("\r[ INFO ] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#else
// UEFI application
#include <efi.h>
#include <efilib.h>
#define Log(msg, ...) Print(L"\r[ INFO ] " msg L"\r\n" __VA_OPT__(,) __VA_ARGS__)

#endif

#else

#define log(msg, ...)
#define Log(msg, ...)

#endif
