#pragma once

#include <efi.h>
#include <efilib.h>

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
#define MSAPI __attribute__((ms_abi))

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
        uefi_call_wrapper(File->Close, 1, File);
    }
}

#define __CLOSE_FILE __attribute__((cleanup(EfiAutoCloseFile)))
