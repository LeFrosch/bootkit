#include <efi.h>
#include <efilib.h>

#include "boot.h"
#include "eficall.h"
#include "utils.h"
#include "grub2.h"

STATIC VOID* gProtocolInstalledRegistration = NULL;

STATIC VOID WaitForExit() {
    Print(L"\nPress any key to exit...\n");
    
    // clear any pending keystrokes 
    uefi_call_wrapper(ST->ConIn->Reset, 2, ST->ConIn, FALSE);

    // wait until a key is pressed
    UINTN Index;
    uefi_call_wrapper(BS->WaitForEvent, 3, 1, &ST->ConIn->WaitForKey, &Index);

    // read the key to clear it from the buffer
    EFI_INPUT_KEY Key;
    EfiReadKeyStroke(ST->ConIn, &Key);
}

STATIC VOID EFIAPI OnProtocolInstalled(EFI_EVENT Event, VOID* Context) {
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
        GRUB_LOAD_FILE2_PROTOCOL* LoadFile2 = NULL;
        Status = EfiHandleProtocol(HandleBuffer[i], &gGrubEfiLoadFile2ProtocolGuid, (VOID**)&LoadFile2);
        if (EFI_ERROR(Status)) {
            continue;
        }

    }

    Print(L"Protocol installed.\n");
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);

    EFI_EVENT NotifyEvent;
    Status = EfiCreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, &OnProtocolInstalled, NULL, &NotifyEvent);
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
    WaitForExit();

    return EFI_SUCCESS;
}
