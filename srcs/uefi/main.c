#include <efi.h>
#include <efilib.h>

static void WaitForExit() {
    Print(L"\nPress any key to exit...\n");
    
    // clear any pending keystrokes 
    uefi_call_wrapper(ST->ConIn->Reset, 2, ST->ConIn, FALSE);

    // wait until a key is pressed
    UINTN Index;
    uefi_call_wrapper(BS->WaitForEvent, 3, 1, &ST->ConIn->WaitForKey, &Index);

    // read the key to clear it from the buffer
    EFI_INPUT_KEY Key;
    uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    UINTN DataSize;
    const auto Data = LibGetVariableAndSize(L"BootOrder", &EfiGlobalVariable, &DataSize);

    if (Data != NULL) {
        Print(L"Successfully read BootOrder. Size is %d bytes.\n", DataSize);
        FreePool(Data);
    } else {
        Print(L"Failed to read variable.\n");
    }

    WaitForExit();

    return EFI_SUCCESS;
}
