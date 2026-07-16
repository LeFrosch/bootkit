#include <efi.h>

// generated init header containing the CPIO archive
#include "init.h"

STATIC CONST struct {
    UINT8* Buffer;
    UINTN Size;
} gCpioArchive = {.Buffer = init_archive, .Size = sizeof(init_archive)};
