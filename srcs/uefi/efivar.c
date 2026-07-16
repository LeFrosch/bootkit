#include <efi.h>
#include <efilib.h>

#include "eficall.h"
#include "efivar.h"
#include "utils.h"

EFI_STATUS EfiVarBootOrderRead(EFI_BOOT_ORDER** Out) {
    UINTN DataSize;
    __FREE_POOL CONST AUTO Buffer = LibGetVariableAndSize(L"BootOrder", &EfiGlobalVariable, &DataSize);
    if (Buffer == NULL) {
        return EFI_NOT_FOUND;
    }

    AUTO Order = (EFI_BOOT_ORDER*)AllocatePool(sizeof(EFI_BOOT_ORDER) + DataSize);
    if (Order == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    Order->Count = DataSize / sizeof(UINT16);
    EfiCopyMem(Order->Options, Buffer, DataSize);

    *Out = Order;
    return EFI_SUCCESS;
}

EFI_STATUS EfiVarLoadOptionRead(UINT16 Index, EFI_LOAD_OPTION** Out) {
    CHAR16 Name[10];
    SPrint(Name, sizeof(Name), L"Boot%04x", Index);

    UINTN DataSize;
    __FREE_POOL CONST AUTO Buffer = LibGetVariableAndSize(Name, &EfiGlobalVariable, &DataSize);
    if (Buffer == NULL) {
        return EFI_NOT_FOUND;
    }

    AUTO Option = (EFI_LOAD_OPTION*)AllocatePool(DataSize);
    if (Option == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    EfiCopyMem(Option, Buffer, DataSize);

    *Out = Option;
    return EFI_SUCCESS;
}

EFI_STATUS EfiVarBootCurrentRead(UINT16* Out) {
    UINTN DataSize;
    __FREE_POOL CONST AUTO Buffer = LibGetVariableAndSize(L"BootCurrent", &EfiGlobalVariable, &DataSize);
    if (Buffer == NULL) {
        return EFI_NOT_FOUND;
    }

    if (DataSize != sizeof(UINT16)) {
        return EFI_UNSUPPORTED;
    }

    EfiCopyMem(Out, Buffer, sizeof(UINT16));
    return EFI_SUCCESS;
}

CHAR16* EfiVarLoadOptionGetDescription(CONST EFI_LOAD_OPTION* Options) {
    if (Options == NULL) {
        return NULL;
    }
    return (CHAR16*)Options->Data;
}

EFI_DEVICE_PATH* EfiVarLoadOptionGetPathList(CONST EFI_LOAD_OPTION* Options) {
    if (Options == NULL) {
        return NULL;
    }
    return (EFI_DEVICE_PATH*)(Options->Data + StrSize(EfiVarLoadOptionGetDescription(Options)));
}
