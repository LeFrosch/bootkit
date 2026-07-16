#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "grub2.h"
#include "utils.h"
#include "init.h"

STATIC EFI_STATUS MSAPI GrubLoadFile2Delegate(
    GRUB_LOAD_FILE2_DELEGATE_PROTOCOL* This,
    EFI_DEVICE_PATH* FilePath,
    BOOLEAN BootPolicy,
    UINTN* BufferSize,
    VOID* Buffer
) {
    if (Buffer == NULL) {
        CONST AUTO Status = GrubLoadFile2(This, FilePath, BootPolicy, BufferSize, NULL);
        if (Status == EFI_BUFFER_TOO_SMALL) {
            *BufferSize += init_archive_len;
        }

        return Status;
    }

    UINTN ActualSize = *BufferSize;
    CONST AUTO Status = GrubLoadFile2(This, FilePath, BootPolicy, &ActualSize, Buffer);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    EfiCopyMem((UINT8*)Buffer + ActualSize, init_archive, init_archive_len);
    *BufferSize = ActualSize + init_archive_len;

    Log("Modified initrd Image (%d -> %d)", ActualSize, *BufferSize);

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);

    Log(L"Preparing GRUB environment...");

    Status = GrubInstallLoadFile2Delegate(GrubLoadFile2Delegate);
    if (EFI_ERROR(Status)) {
        Log(L"Could not install LoadFile2 delegate: %r", Status);
    }

    Status = BootDebian(ImageHandle);
    if (EFI_ERROR(Status)) {
        Log(L"Could not boot debian: %r", Status);
    }

    return EFI_SUCCESS;
}
