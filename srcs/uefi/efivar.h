#pragma once

#include <efi.h>
#include <efilib.h>

typedef struct {
    UINT32 Attributes;
    UINT16 FilePathListLength;

    // --- variable length fields ---

    // UINT16 Description[];
    // UINT8 FilePathList[];
    // UINT8 OptionalData[];

    UINT8 Data[];
} __attribute__((packed)) EFI_LOAD_OPTION;

typedef struct {
    UINTN Count;
    UINT16 Options[];
} EFI_BOOT_ORDER;

EFI_STATUS EfiVarBootOrderRead(EFI_BOOT_ORDER** Out);

EFI_STATUS EfiVarLoadOptionRead(UINT16 Index, EFI_LOAD_OPTION** Out);

EFI_STATUS EfiVarBootCurrentRead(UINT16* Out);

CHAR16* EfiVarLoadOptionGetDescription(CONST EFI_LOAD_OPTION* Options);

EFI_DEVICE_PATH* EfiVarLoadOptionGetPathList(CONST EFI_LOAD_OPTION* Options);
