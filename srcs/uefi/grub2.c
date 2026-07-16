#include <efi.h>
#include <efiapi.h>

#include "grub2.h"
#include "utils.h"
#include "eficall.h"

// copied from the GRUB2 sources (include/grub/efi/api.h:152)
STATIC EFI_GUID gGrubEfiLoadFile2ProtocolGuid = {
    0x4006c0c1,
    0xfcb3,
    0x403e,
    {0x99, 0x6d, 0x4a, 0x6c, 0x87, 0x24, 0xe0, 0x6d}
};

STATIC VOID* gProtocolInstalledRegistration = NULL;
STATIC EFI_EVENT gNotifyEvent;

STATIC VOID MSAPI OnGrubLoadFile2Installed(EFI_EVENT Event, VOID* Context) {
    (VOID) Event;

    EFI_STATUS Status = EFI_SUCCESS;
    AUTO Trampoline = (GRUB_LOAD_FILE2_TRAMPOLINE)Context;

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
        if ((UINT64)Original->LoadFile == (UINT64)Trampoline) {
            continue;
        }

        GRUB_LOAD_FILE2_DELEGATE_PROTOCOL* Delegate = AllocatePool(sizeof(GRUB_LOAD_FILE2_DELEGATE_PROTOCOL));
        Delegate->Trampoline = Trampoline;
        Delegate->Protocol = Original;

        Status = EfiReinstallProtocolInterface(HandleBuffer[i], &gGrubEfiLoadFile2ProtocolGuid, Original, Delegate);
        if (EFI_ERROR(Status)) {
            FreePool(Delegate);
            continue;
        }
    }
}

EFI_STATUS GrubInstallLoadFile2Delegate(GRUB_LOAD_FILE2_TRAMPOLINE Trampoline) {
    EFI_STATUS Status = EFI_SUCCESS;

    Status = EfiCreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, OnGrubLoadFile2Installed, Trampoline, &gNotifyEvent);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = EfiRegisterProtocolNotify(&gGrubEfiLoadFile2ProtocolGuid, gNotifyEvent, &gProtocolInstalledRegistration);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS GrubUninstallLoadFile2Delegate() {
    if (gNotifyEvent != NULL) {
        return EfiCloseEvent(gNotifyEvent);
    }
    return EFI_SUCCESS;
}

EFI_STATUS GrubLoadFile2(
    GRUB_LOAD_FILE2_DELEGATE_PROTOCOL* This,
    EFI_DEVICE_PATH* FilePath,
    BOOLEAN BootPolicy,
    UINTN* BufferSize,
    VOID* Buffer
) {
    return uefi_call_wrapper(This->Protocol->LoadFile, 5, This->Protocol, FilePath, BootPolicy, BufferSize, Buffer);
}

