#include <efi.h>
#include <efiapi.h>

#include "eficall.h"
#include "efidef.h"
#include "efidevp.h"
#include "grub2.h"
#include "legacy/efilib.h"
#include "utils.h"

// copied from include/grub/efi/api.h:377
#define LINUX_EFI_INITRD_MEDIA_GUID                                                                                    \
    {                                                                                                                  \
        0x5568e427,                                                                                                    \
        0x68fc,                                                                                                        \
        0x4f3d,                                                                                                        \
        {0xac, 0x74, 0xca, 0x55, 0x52, 0x31, 0xcc, 0x68},                                                              \
}

STATIC EFI_GUID gLinuxEfiInitrdMediaGuid = LINUX_EFI_INITRD_MEDIA_GUID;
STATIC VOID* gProtocolRegistration = NULL;
STATIC EFI_EVENT gNotifyEvent;

STATIC VOID MSAPI OnGrubLoadFile2Installed(EFI_EVENT Event, VOID* Context) {
    (VOID) Event;

    EFI_STATUS Status = EFI_SUCCESS;
    AUTO Trampoline = (EFI_LOAD_FILE2_TRAMPOLINE)Context;

    while (TRUE) {
        // find the handler that was installed with this event
        UINTN NumHandles = 0;
        __FREE_POOL EFI_HANDLE* HandleBuffer = NULL;
        Status = EfiLocateHandleBuffer(ByRegisterNotify, NULL, gProtocolRegistration, &NumHandles, &HandleBuffer);
        if (EFI_ERROR(Status)) {
            break;
        }
        if (NumHandles != 1) {
            Log("Expected exactly on protocol to be found for ByRegisterNotify: %d", NumHandles);
            break;
        }

        // GRUB registers the LoadFile2 protocol for a special handle that also has a device path protocol
        EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;
        Status = EfiHandleProtocol(HandleBuffer[0], &DevicePathProtocol, (VOID**)&DevicePath);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // check if the device path is GRUB's
        if (DevicePath->Type != MEDIA_DEVICE_PATH || DevicePath->SubType != MEDIA_VENDOR_DP) {
            Log("Skipping device path: 0x%02x 0x%02x", DevicePath->Type, DevicePath->SubType);
            continue;
        }

        CONST AUTO VendorGuid = &((VENDOR_DEVICE_PATH*)DevicePath)->Guid;
        if (CompareGuid(VendorGuid, &gLinuxEfiInitrdMediaGuid) != 0) {
            Log("Skipping registration for vendor GUID: 0x%x", VendorGuid->Data1);
            continue;
        }

        // load the actual LoadFile2 protocol we are interested in
        EFI_LOAD_FILE2_PROTOCOL* Original = NULL;
        Status = EfiHandleProtocol(HandleBuffer[0], &LoadFile2Protocol, (VOID**)&Original);
        if (EFI_ERROR(Status)) {
            Log("No load file2 path protocol found: %r", Status);
            continue;
        }

        // prevent infinite loop: EfiReinstallProtocolInterface triggers this event again!
        if ((UINT64)Original->LoadFile == (UINT64)Trampoline) {
            continue;
        }

        EFI_LOAD_FILE2_DELEGATE_PROTOCOL* Delegate = AllocatePool(sizeof(EFI_LOAD_FILE2_DELEGATE_PROTOCOL));
        Delegate->Trampoline = Trampoline;
        Delegate->Protocol = Original;

        Status = EfiReinstallProtocolInterface(HandleBuffer[0], &LoadFile2Protocol, Original, Delegate);
        if (EFI_ERROR(Status)) {
            Log("Reinstall failed: %r", Status);
            FreePool(Delegate);
            continue;
        }

        Log("Installed GRUB LoadFile2 delegate!");
    }
}

EFI_STATUS GrubInstallLoadFile2Delegate(EFI_LOAD_FILE2_TRAMPOLINE Trampoline) {
    EFI_STATUS Status = EFI_SUCCESS;

    Status = EfiCreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, OnGrubLoadFile2Installed, Trampoline, &gNotifyEvent);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = EfiRegisterProtocolNotify(&LoadFile2Protocol, gNotifyEvent, &gProtocolRegistration);
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
