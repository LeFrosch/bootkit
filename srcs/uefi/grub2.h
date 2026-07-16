#pragma once

#include <efi.h>

/**
 * The UEFI specification mandates the Microsoft x64 Calling Convention (MS ABI) 
 * for all firmware interactions and protocols on x86_64 architectures. 
 * 
 * Since this program is built with gnu-efi (which uses the standard Linux System V ABI), 
 * any function pointers to/from GRUB must be explicitly marked with the MS ABI.  * 
 * 
 * Not sure why EFIAPI does not expand to __attribute__((ms_abi)), but for some reason 
 * it does not, and thus we need this custom MACRO for GRUB functions.
 */
#define GRUBAPI __attribute__((ms_abi))

// copied from the GRUB2 sources (include/grub/efi/api.h:1822)
INTERFACE_DECL(_GRUB_EFI_LOAD_FILE2_PROTOCOL);

typedef EFI_STATUS(GRUBAPI* GRUB_LOAD_FILE2)(
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
