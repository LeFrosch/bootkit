#pragma once

#include <efi.h>

// copied from the GRUB2 sources (include/grub/efi/api.h:1822)
INTERFACE_DECL(_GRUB_EFI_LOAD_FILE2_PROTOCOL);

typedef EFI_STATUS(EFIAPI* GRUB_LOAD_FILE2)(
    IN struct _GRUB_EFI_LOAD_FILE2_PROTOCOL* This,
    IN EFI_DEVICE_PATH* FilePath,
    IN BOOLEAN BootPolicy,
    IN OUT UINTN* BufferSize,
    OUT VOID* Buffer
);

typedef struct _GRUB_EFI_LOAD_FILE2_PROTOCOL {
    GRUB_LOAD_FILE2 LoadFile;
} GRUB_LOAD_FILE2_PROTOCOL;

// copied from the GRUB2 sources (include/grub/efi/api.h:152)
STATIC EFI_GUID gGrubEfiLoadFile2ProtocolGuid = {
    0x4006c0c1,
    0xfcb3,
    0x403e,
    {0x99, 0x6d, 0x4a, 0x6c, 0x87, 0x24, 0xe0, 0x6d}
};

