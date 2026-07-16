#pragma once

#include <efi.h>

#include "utils.h"

// copied from the GRUB2 sources (include/grub/efi/api.h:1822)
INTERFACE_DECL(_GRUB_EFI_LOAD_FILE2_PROTOCOL);

typedef EFI_STATUS(MSAPI* GRUB_LOAD_FILE2)(
    IN struct _GRUB_EFI_LOAD_FILE2_PROTOCOL* This,
    IN EFI_DEVICE_PATH* FilePath,
    IN BOOLEAN BootPolicy,
    IN OUT UINTN* BufferSize,
    OUT VOID* Buffer
);

typedef struct _GRUB_EFI_LOAD_FILE2_PROTOCOL {
    GRUB_LOAD_FILE2 LoadFile;
} GRUB_LOAD_FILE2_PROTOCOL;

// custom protocol for installing a delegate for the load file 2 protocol
INTERFACE_DECL(_GRUB_EFI_LOAD_FILE2_DELEGATE_PROTOCOL);

typedef EFI_STATUS(MSAPI* GRUB_LOAD_FILE2_TRAMPOLINE)(
    IN struct _GRUB_EFI_LOAD_FILE2_DELEGATE_PROTOCOL* This,
    IN EFI_DEVICE_PATH* FilePath,
    IN BOOLEAN BootPolicy,
    IN OUT UINTN* BufferSize,
    OUT VOID* Buffer
);

typedef struct _GRUB_EFI_LOAD_FILE2_DELEGATE_PROTOCOL {
    GRUB_LOAD_FILE2_TRAMPOLINE Trampoline;
    GRUB_LOAD_FILE2_PROTOCOL* Protocol;
} GRUB_LOAD_FILE2_DELEGATE_PROTOCOL;

EFI_STATUS GrubInstallLoadFile2Delegate(GRUB_LOAD_FILE2_TRAMPOLINE Trampoline);

EFI_STATUS GrubUninstallLoadFile2Delegate();

EFI_STATUS GrubLoadFile2(
    GRUB_LOAD_FILE2_DELEGATE_PROTOCOL* This,
    EFI_DEVICE_PATH* FilePath,
    BOOLEAN BootPolicy,
    UINTN* BufferSize,
    VOID* Buffer
);

