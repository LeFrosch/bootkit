#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "grub2.h"
#include "utils.h"
#include "cpioarchive.h"

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
            *BufferSize += gCpioArchive.Size;
        }

        return Status;
    }

    UINTN ActualSize = *BufferSize;
    CONST AUTO Status = GrubLoadFile2(This, FilePath, BootPolicy, &ActualSize, Buffer);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    EfiCopyMem((UINT8*)Buffer + ActualSize, gCpioArchive.Buffer, gCpioArchive.Size);
    *BufferSize = ActualSize + gCpioArchive.Size;

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);

    Status = GrubInstallLoadFile2Delegate(GrubLoadFile2Delegate);
    if (EFI_ERROR(Status)) {
        Print(L"Could not install LoadFile2 delegate: %r\n", Status);
    }

    Status = BootDebian(ImageHandle);
    if (EFI_ERROR(Status)) {
        Print(L"Could not boot debian: %r\n", Status);
    }

    return EFI_SUCCESS;
}
