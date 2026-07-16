#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "grub2.h"
#include "init.h"
#include "utils.h"

typedef struct _GRUB_EFI_LOAD_FILE2_DELEGATE {
    GRUB_LOAD_FILE2 LoadFile;
    GRUB_LOAD_FILE2_PROTOCOL* OriginalThis;
} GRUB_LOAD_FILE2_DELEGATE;

STATIC VOID* gProtocolInstalledRegistration = NULL;

STATIC EFI_STATUS GRUBAPI OnGrubLoadFile2(
    GRUB_LOAD_FILE2_DELEGATE* This,
    EFI_DEVICE_PATH* FilePath,
    BOOLEAN BootPolicy,
    UINTN* BufferSize,
    VOID* Buffer
) {
    if (Buffer == NULL) {
        CONST AUTO Status = uefi_call_wrapper(This->OriginalThis->LoadFile, 5, This->OriginalThis, FilePath, BootPolicy, BufferSize, NULL);
        if (Status == EFI_BUFFER_TOO_SMALL) {
            *BufferSize += init_archive_len;
        }

        return Status;
    }

    UINTN ActualSize = *BufferSize;
    CONST AUTO Status = uefi_call_wrapper(This->OriginalThis->LoadFile, 5, This->OriginalThis, FilePath, BootPolicy, &ActualSize, Buffer);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    EfiCopyMem((UINT8*)Buffer + ActualSize, init_archive, init_archive_len);
    *BufferSize = ActualSize + init_archive_len;

    return EFI_SUCCESS;
}

STATIC VOID EFIAPI OnGrubLoadFile2Installed(EFI_EVENT Event, VOID* Context) {
    EFI_STATUS Status = EFI_SUCCESS;

    (VOID) Event;
    (VOID) Context;

    // find the handler that was installed with this event
    UINTN NumHandles = 0;
    __FREE_POOL EFI_HANDLE* HandleBuffer = NULL;
    Status = EfiLocateHandleBuffer(ByRegisterNotify, NULL, gProtocolInstalledRegistration, &NumHandles, &HandleBuffer);
    if (EFI_ERROR(Status)) {
        return;
    }

    for (UINTN i = 0; i < NumHandles; i++) {
        GRUB_LOAD_FILE2_PROTOCOL* Original = NULL;
        Status = EfiHandleProtocol(HandleBuffer[i], &gGrubEfiLoadFile2ProtocolGuid, (VOID**)&Original);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // prevent infinite loop: EfiReinstallProtocolInterface triggers this event again!
        if ((UINT64)Original->LoadFile == (UINT64)&OnGrubLoadFile2) {
            continue;
        }

        GRUB_LOAD_FILE2_DELEGATE* Delegate = AllocatePool(sizeof(GRUB_LOAD_FILE2_DELEGATE));
        Delegate->LoadFile = (GRUB_LOAD_FILE2)&OnGrubLoadFile2;
        Delegate->OriginalThis = Original;

        Status = EfiReinstallProtocolInterface(HandleBuffer[i], &gGrubEfiLoadFile2ProtocolGuid, Original, Delegate);
        if (EFI_ERROR(Status)) {
            FreePool(Delegate);
            continue;
        }

        Print(L"Installed GRUB load file 2 delegate for handle: %p\n", HandleBuffer[i]);
    }
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);

    EFI_EVENT NotifyEvent;
    Status = EfiCreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, &OnGrubLoadFile2Installed, NULL, &NotifyEvent);
    if (EFI_ERROR(Status)) {
        Print(L"Could not create event %r.\n", Status);
        return Status;
    }

    Status = EfiRegisterProtocolNotify(&gGrubEfiLoadFile2ProtocolGuid, NotifyEvent, &gProtocolInstalledRegistration);
    if (EFI_ERROR(Status)) {
        Print(L"Could not register protocol notification %r.\n", Status);
        return Status;
    }

    BootDebian(ImageHandle);

    return EFI_SUCCESS;
}
