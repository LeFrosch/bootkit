#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "efidef.h"
#include "efierr.h"
#include "efivar.h"
#include "utils.h"

STATIC EFI_STATUS BootFilePath(EFI_HANDLE ParentImageHandle, CHAR16* Path) {
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
        Status = EfiOpenFile(Root, &File, Path, EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // build a full Device Path for the debian bootloader
        __FREE_POOL AUTO DevPath = FileDevicePath(HandleBuffer[i], Path);

        EFI_HANDLE BootLoader;
        Status = EfiImageLoad(FALSE, ParentImageHandle, DevPath, NULL, 0, &BootLoader);
        if (EFI_ERROR(Status)) {
            Log(L"Failed to load %s image: %r", Path, Status);
            continue;
        }

        Log(L"Found %s on a filesystem! Booting...", Path);

        EfiImageStart(BootLoader, NULL, NULL);
        return EFI_NOT_STARTED;
    }

    Log(L"Could not find %s on any filesystem.", Path);

    return EFI_NOT_FOUND;
}

EFI_STATUS BootDebian(EFI_HANDLE ParentImageHandle) {
    return BootFilePath(ParentImageHandle, L"\\EFI\\debian\\grubx64.efi");
}

STATIC EFI_STATUS GetFilePath(EFI_DEVICE_PATH* Path, CHAR16** Out) {
    while (Path != NULL && !IsDevicePathEnd(Path)) {
        if (DevicePathType(Path) == MEDIA_DEVICE_PATH && DevicePathSubType(Path) == MEDIA_FILEPATH_DP) {
            *Out = ((FILEPATH_DEVICE_PATH*)Path)->PathName;
            return EFI_SUCCESS;
        }
        Path = NextDevicePathNode(Path);
    }

    return EFI_NOT_FOUND;
}

EFI_STATUS BootNext(EFI_HANDLE ParentImageHandle) {
    (void)ParentImageHandle;
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

        EFI_HANDLE BootLoader = NULL;
        Status = EfiImageLoad(TRUE, ParentImageHandle, Path, NULL, 0, &BootLoader);

        // fallback to manually booting the file
        if (EFI_ERROR(Status)) {
            CHAR16* FilePath = NULL;
            Status = GetFilePath(Path, &FilePath);
            if (EFI_ERROR(Status)) {
                continue;
            }

            Status = BootFilePath(ParentImageHandle, FilePath);
            if (!EFI_ERROR(Status)) {
                continue;
            }
        } else {
            Log(L"Starting next boot loader (%d): %s", BootOrder->Options[i], EfiVarLoadOptionGetDescription(Option));
            EfiImageStart(BootLoader, NULL, NULL);
        }

        return EFI_NOT_STARTED;
    }

    Log(L"No boot loader could be started (%d).", BootOrder->Count);

    return EFI_NOT_FOUND;
}
