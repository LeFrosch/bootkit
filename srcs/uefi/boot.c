#include <efi.h>
#include <efilib.h>

#include "eficall.h"
#include "utils.h"
#include "boot.h"

EFI_STATUS BootDebian(EFI_HANDLE ParentImageHandle) {
    EFI_STATUS Status = EFI_SUCCESS;

    // locate all handles that support the Simple File System protocol
    UINTN NumHandles = 0;
    __FREE_POOL EFI_HANDLE* HandleBuffer = NULL;
    Status = EfiLocateHandleBuffer(ByProtocol, &FileSystemProtocol, NULL, &NumHandles, &HandleBuffer);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    for (UINTN i = 0; i < NumHandles; i++) {
        EFI_FILE_IO_INTERFACE* Fs = NULL;
        Status = EfiHandleProtocol(HandleBuffer[i], &FileSystemProtocol, (VOID**)&Fs);
        if (EFI_ERROR(Status)) {
            continue;
        }

        __CLOSE_FILE EFI_FILE* Root = NULL;
        Status = EfiOpenVolume(Fs, &Root);
        if (EFI_ERROR(Status)) {
            continue;
        }

        __CLOSE_FILE EFI_FILE* File = NULL;
        Status = EfiOpenFile(Root, &File, DEBIAN_BOOT_LOADER, EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR(Status)) {
            continue;
        }

        Print(L"Found Debian on a filesystem! Booting...\n");

        // build a full Device Path for the debian bootloader
        __FREE_POOL AUTO DevPath = FileDevicePath(HandleBuffer[i], DEBIAN_BOOT_LOADER);

        EFI_HANDLE BootLoader;
        Status = EfiImageLoad(FALSE, ParentImageHandle, DevPath, NULL, 0, &BootLoader);
        if (!EFI_ERROR(Status)) {
            EfiImageStart(BootLoader, NULL, NULL);
        } else {
            Print(L"Failed to load Debian image: %r\n", Status);
        }
    }

    Print(L"Could not find " DEBIAN_BOOT_LOADER " on any filesystem.\n");

    return EFI_NOT_FOUND;
}

// STATIC EFI_LOAD_OPTION* GetBootOption(UINTN Index) {
//     CHAR16 Buffer[9];
//     SPrint(Buffer, sizeof(Buffer), L"Boot%04x", Index);
//
//     UINTN DataSize;
//     return LibGetVariableAndSize(Buffer, &EfiGlobalVariable, &DataSize);
// }
