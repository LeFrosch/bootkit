#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "grub2.h"
#include "init.h"
#include "utils.h"

#define ALIGN_UP_4(size) (((UINTN)(size) + 3) & ~((UINTN)3))

STATIC EFI_STATUS MSAPI GrubLoadFile2Delegate(
    EFI_LOAD_FILE2_DELEGATE_PROTOCOL* This,
    EFI_DEVICE_PATH* FilePath,
    BOOLEAN BootPolicy,
    UINTN* BufferSize,
    VOID* Buffer
) {
    if (Buffer == NULL) {
        UINTN ImageSize = 0;
        CONST AUTO Status = EfiLoadFile2(This->Protocol, FilePath, BootPolicy, &ImageSize, NULL);
        if (EFI_ERROR(Status) && Status != EFI_BUFFER_TOO_SMALL) {
            return Status;
        }

        *BufferSize = ALIGN_UP_4(ImageSize) + init_archive_len;
        return EFI_BUFFER_TOO_SMALL;
    }

    UINTN ImageSize = *BufferSize;
    CONST AUTO Status = EfiLoadFile2(This->Protocol, FilePath, BootPolicy, &ImageSize, Buffer);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // some debian images are pedded with 0xCC at the end, this causes some
    // issues with the kernels decompression routine
    UINTN ImageEnd = *BufferSize;
    while (ImageEnd > 0 && ((UINT8*)Buffer)[ImageEnd - 1] == 0xCC) {
        ImageEnd--;
    }

    Log("Trimming initrd Image (%d -> %d)", ImageSize, ImageEnd);

    CONST AUTO PatchStart = ALIGN_UP_4(ImageSize);

    // zero out the padding bytes between the archives
    EfiSetMem((UINT8*)Buffer + ImageEnd, PatchStart - ImageEnd, 0);

    // copy the archive at the padded offset
    EfiCopyMem((UINT8*)Buffer + PatchStart, init_archive, init_archive_len);

    // update the final buffer size to reflect the padding + new archive
    *BufferSize = PatchStart + init_archive_len;

    Log("Modified initrd Image (%d -> %d)", ImageSize, *BufferSize);

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);

    Log(L"Preparing GRUB environment...");

    Status = GrubInstallLoadFile2Delegate(GrubLoadFile2Delegate);
    if (EFI_ERROR(Status)) {
        Log(L"Could not install LoadFile2 delegate: %r", Status);
        goto end;
    }

    Log(L"Searching for boot loader...");

    Status = BootNext(ImageHandle);
    if (EFI_ERROR(Status)) {
        Log(L"Could not boot next: %r", Status);
        goto end;
    }

end:
    EfiStall(5000000);
    GrubUninstallLoadFile2Delegate();
    return Status;
}
