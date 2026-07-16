#pragma once

#include <efi.h>
#include <efilib.h>

#include "eficall.h"

#define AUTO __auto_type

STATIC VOID __attribute__((unused)) EfiAutoFreePool(CONST VOID* ptr) {
    AUTO p = *(VOID**)ptr;
    if (p != NULL) {
        FreePool(p);
    }
}

#define __FREE_POOL __attribute__((cleanup(EfiAutoFreePool)))

STATIC VOID __attribute__((unused)) EfiAutoCloseFile(CONST VOID* ptr) {
    AUTO File = *(EFI_FILE**)ptr;
    if (File != NULL) {
        EfiCloseFile(File);
    }
}

#define __CLOSE_FILE __attribute__((cleanup(EfiAutoCloseFile)))
