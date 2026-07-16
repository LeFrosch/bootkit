#pragma once

#include <efi.h>

#include "utils.h"

// custom protocol for installing a delegate for the load file 2 protocol
INTERFACE_DECL(_EFI_LOAD_FILE2_DELEGATE_PROTOCOL);

typedef EFI_STATUS(MSAPI* EFI_LOAD_FILE2_TRAMPOLINE)(
    IN struct _EFI_LOAD_FILE2_DELEGATE_PROTOCOL* This,
    IN EFI_DEVICE_PATH* FilePath,
    IN BOOLEAN BootPolicy,
    IN OUT UINTN* BufferSize,
    OUT VOID* Buffer
);

typedef struct _EFI_LOAD_FILE2_DELEGATE_PROTOCOL {
    EFI_LOAD_FILE2_TRAMPOLINE Trampoline;
    EFI_LOAD_FILE2_PROTOCOL* Protocol;
} EFI_LOAD_FILE2_DELEGATE_PROTOCOL;

EFI_STATUS GrubInstallLoadFile2Delegate(EFI_LOAD_FILE2_TRAMPOLINE Trampoline);

EFI_STATUS GrubUninstallLoadFile2Delegate();

