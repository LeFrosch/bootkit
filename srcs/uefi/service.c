#include <efi.h>
#include <efilib.h>

#include "eficall.h"
#include "efivar.h"
#include "utils.h"

STATIC CHAR16 BootName[10];
STATIC UINT16 BootIndex;

STATIC EFI_GET_VARIABLE OriginalGetVariable = NULL;
STATIC EFI_GET_NEXT_VARIABLE_NAME OriginalGetNextVariableName = NULL;
STATIC EFI_SET_VARIABLE OriginalSetVariable = NULL;

STATIC VOID CopyMem2(VOID* Dst, VOID* Src, UINTN Size) {
    for (UINTN i = 0; i < Size; i++) {
        ((UINT8*)Dst)[i] = ((UINT8*)Src)[i];
    }
}

STATIC VOID HideBootOrder(UINTN* DataSize, UINT8* Data) {
    if (*DataSize <= sizeof(UINT16)) {
        return;
    }

    for (UINTN i = 0; i < *DataSize - sizeof(UINT16); i++) {
        Data[i] = Data[i + sizeof(UINT16)];
    }
    *DataSize -= sizeof(UINT16);
}

STATIC EFI_STATUS MSAPI
GetVariableDelegate(CHAR16* VariableName, EFI_GUID* VendorGuid, UINT32* Attributes, UINTN* DataSize, VOID* Data) {
    CONST AUTO Status = uefi_call_wrapper(OriginalGetVariable, 5, VariableName, VendorGuid, Attributes, DataSize, Data);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    if (StrCmp(VariableName, L"BootOrder") == 0) {
        HideBootOrder(DataSize, Data);
    }
    if (StrCmp(VariableName, BootName) == 0) {
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}

STATIC EFI_STATUS MSAPI
GetNextVariableNameDelegate(UINTN* VariableNameSize, CHAR16* VariableName, EFI_GUID* VendorGuid) {
    CONST AUTO OriginalSize = *VariableNameSize;

    while (TRUE) {
        *VariableNameSize = OriginalSize;

        CONST AUTO Status = uefi_call_wrapper(
            OriginalGetNextVariableName, 3, VariableNameSize, VariableName, VendorGuid
        );
        if (EFI_ERROR(Status)) {
            return Status;
        }

        if (CompareGuid(VendorGuid, &EfiGlobalVariable) != 0) {
            return EFI_SUCCESS;
        }

        if (StrnCmp(VariableName, BootName, 8) != 0) {
            return EFI_SUCCESS;
        }
    }
}

STATIC EFI_STATUS MSAPI
SetVariableDelegate(CHAR16* VariableName, EFI_GUID* VendorGuid, UINT32 Attributes, UINTN DataSize, VOID* Data) {
    if (StrCmp(VariableName, L"BootOrder") == 0) {
        CONST AUTO Size = DataSize + sizeof(UINT16);

        UINT8 Buffer[Size];
        CopyMem2(Buffer + sizeof(UINT16), Data, DataSize);
        CopyMem2(Buffer, &BootIndex, sizeof(UINT16));

        return uefi_call_wrapper(OriginalSetVariable, 5, VariableName, VendorGuid, Attributes, Size, Buffer);
    }

    if (StrCmp(VariableName, BootName)) {
        return EFI_NOT_FOUND;
    }

    return uefi_call_wrapper(OriginalSetVariable, 5, VariableName, VendorGuid, Attributes, DataSize, Data);
}

STATIC EFI_STATUS ReadBootCurrent() {
    CONST AUTO Status = EfiVarBootCurrentRead(&BootIndex);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    SPrint(BootName, sizeof(BootName), L"Boot%04x", BootIndex);
    return EFI_SUCCESS;
}

STATIC EFI_STATUS UpdateBootCurrent() {
    __FREE_POOL EFI_BOOT_ORDER* BootOrder = NULL;
    CONST AUTO Status = EfiVarBootOrderRead(&BootOrder);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    if (BootOrder->Count < 2) {
        return EFI_UNSUPPORTED;
    }

    return EfiVarBootCurrentWrite(BootOrder->Options[1]);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);

    Status = ReadBootCurrent();
    if (EFI_ERROR(Status)) {
        Log(L"Could not read BootCurrent: %r", Status);
        return Status;
    }

    Status = UpdateBootCurrent();
    if (EFI_ERROR(Status)) {
        Log(L"Could update BootCurrent: %r", Status);
        return Status;
    }

    OriginalGetVariable = RT->GetVariable;
    RT->GetVariable = (EFI_GET_VARIABLE)(VOID*)GetVariableDelegate;
    OriginalGetNextVariableName = RT->GetNextVariableName;
    RT->GetNextVariableName = (EFI_GET_NEXT_VARIABLE_NAME)(VOID*)GetNextVariableNameDelegate;
    OriginalSetVariable = RT->SetVariable;
    RT->SetVariable = (EFI_SET_VARIABLE)(VOID*)SetVariableDelegate;

    Log(L"Runtime service initialized (%s)...", BootName);

    return Status;
}
