#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "efierr.h"
#include "efivar.h"
#include "utils.h"

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

        Log(L"Found Debian on a filesystem! Booting...");

        // build a full Device Path for the debian bootloader
        __FREE_POOL AUTO DevPath = FileDevicePath(HandleBuffer[i], DEBIAN_BOOT_LOADER);

        EFI_HANDLE BootLoader;
        Status = EfiImageLoad(FALSE, ParentImageHandle, DevPath, NULL, 0, &BootLoader);
        if (!EFI_ERROR(Status)) {
            EfiImageStart(BootLoader, NULL, NULL);
        } else {
            Log(L"Failed to load Debian image: %r", Status);
        }
    }

    Log(L"Could not find " DEBIAN_BOOT_LOADER " on any filesystem.");

    return EFI_NOT_FOUND;
}

EFI_STATUS BootNext(EFI_HANDLE ParentImageHandle) {
    EFI_STATUS Status = EFI_SUCCESS;

    __FREE_POOL EFI_BOOT_ORDER* BootOrder = NULL;
    Status = EfiVarBootOrderRead(&BootOrder);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    for (UINT16 i = 1; i < BootOrder->Count; i++) {
        __FREE_POOL EFI_LOAD_OPTION* Option = NULL;
        Status = EfiVarLoadOptionRead(BootOrder->Options[i], &Option);
        if (EFI_ERROR(Status)) {
            continue;
        }

        CONST AUTO Path = EfiVarLoadOptionGetPathList(Option);

        EFI_HANDLE BootLoader;
        Status = EfiImageLoad(FALSE, ParentImageHandle, Path, NULL, 0, &BootLoader);
        if (EFI_ERROR(Status)) {
            continue;
        }

        Log(L"Starting next boot loader (%d): %s", BootOrder->Options[i], EfiVarLoadOptionGetDescription(Option));

        EfiImageStart(BootLoader, NULL, NULL);
    }

    Log(L"No boot loader could be started (%d).", BootOrder->Count);

    return EFI_NOT_FOUND;
}
