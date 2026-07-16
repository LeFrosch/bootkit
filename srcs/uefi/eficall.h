#pragma once

#include <efi.h>
#include <efilib.h>

#define EFI_WRAPPER STATIC inline

EFI_WRAPPER EFI_STATUS EfiImageLoad(
    IN BOOLEAN BootPolicy,
    IN EFI_HANDLE ParentImageHandle,
    IN EFI_DEVICE_PATH* FilePath,
    IN VOID* SourceBuffer OPTIONAL,
    IN UINTN SourceSize,
    OUT EFI_HANDLE* ImageHandle
) {
    return uefi_call_wrapper(BS->LoadImage, 6, BootPolicy, ParentImageHandle, FilePath, SourceBuffer, SourceSize, ImageHandle);
}

EFI_WRAPPER EFI_STATUS EfiImageStart(
    IN EFI_HANDLE ImageHandle, 
    OUT UINTN* ExitDataSize, 
    OUT CHAR16** ExitData OPTIONAL
) {
    return uefi_call_wrapper(BS->StartImage, 3, ImageHandle, ExitDataSize, ExitData);
}

/**
 * BOOT_SERVICE.
 *
 * Returns an array of handles that support the requested protocol in a buffer 
 * allocated from pool.
 */
EFI_WRAPPER EFI_STATUS EfiLocateHandleBuffer(
    IN EFI_LOCATE_SEARCH_TYPE SearchType,
    IN EFI_GUID* Protocol OPTIONAL,
    IN VOID* SearchKey OPTIONAL,
    OUT UINTN* NoHandles,
    OUT EFI_HANDLE** Buffer
) {
    return uefi_call_wrapper(BS->LocateHandleBuffer, 5, SearchType, Protocol, SearchKey, NoHandles, Buffer);
}

/**
 * BOOT_SERVICE.
 *
 * Queries a handle to determine if it supports a specified protocol.
 */
EFI_WRAPPER EFI_STATUS EfiHandleProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID* Protocol,
    OUT VOID** Interface
) {
    return uefi_call_wrapper(BS->HandleProtocol, 3, Handle, Protocol, Interface);
}

/**
 * BOOT_SERVICE.
 * 
 * Creates an event that is to be signaled whenever an interface is installed 
 * for a specified protocol.
 */
EFI_WRAPPER EFI_STATUS EfiRegisterProtocolNotify(
    IN EFI_GUID *Protocol,
    IN EFI_EVENT Event,
    OUT VOID **Registration
) {
    return uefi_call_wrapper(BS->RegisterProtocolNotify, 3, Protocol, Event, Registration);
}

/**
 * BOOT_SERVICE.
 *
 * Creates an event.
 */
EFI_WRAPPER EFI_STATUS EfiCreateEvent(
    IN UINT32 Type,
    IN EFI_TPL NotifyTpl,
    IN EFI_EVENT_NOTIFY NotifyFunction, 
    IN VOID *NotifyContext OPTIONAL,
    OUT EFI_EVENT *Event OPTIONAL
) {
    return uefi_call_wrapper(BS->CreateEvent, 5, Type, NotifyTpl, NotifyFunction, NotifyContext, Event);
}

EFI_WRAPPER EFI_STATUS EfiInstallProtocolInterface(
    IN OUT EFI_HANDLE *Handle,
    IN EFI_GUID *Protocol,
    IN EFI_INTERFACE_TYPE InterfaceType,
    IN VOID *Interface
) {
    return uefi_call_wrapper(BS->InstallProtocolInterface, 4, Handle, Protocol, InterfaceType, Interface);
}

EFI_WRAPPER EFI_STATUS EfiReinstallProtocolInterface(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    IN VOID *OldInterface,
    IN VOID *NewInterface
) {
    return uefi_call_wrapper(BS->ReinstallProtocolInterface, 4, Handle, Protocol, OldInterface, NewInterface);
}

/**
 * SIMPLE_FILE_SYSTEM_PROTOCOL.
 *
 * Opens the root directory on a volume.
 */
EFI_WRAPPER EFI_STATUS EfiOpenVolume(
    IN EFI_FILE_IO_INTERFACE* This,
    OUT EFI_FILE** Root
) {
    return uefi_call_wrapper(This->OpenVolume, 2, This, Root);
}

/**
 * FILE_PROTOCOL.
 *
 * Opens a new file relative to the source directory’s location.
 */
EFI_WRAPPER EFI_STATUS EfiOpenFile(
    IN EFI_FILE* This,
    OUT EFI_FILE** NewHandle,
    IN CHAR16* FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes
) {
    return uefi_call_wrapper(This->Open, 5, This, NewHandle, FileName, OpenMode, Attributes);
}

/**
 * FILE_PROTOCOL.
 *
 * Closes a specified file handle.
 */
EFI_WRAPPER EFI_STATUS EfiCloseFile(
    IN EFI_FILE* This
) {
    return uefi_call_wrapper(This->Close, 1, This);
}

EFI_WRAPPER VOID EfiCopyMem(
    IN VOID *Destination,
    IN VOID *Source,
    IN UINTN Length
) {
    uefi_call_wrapper(BS->CopyMem, 3, Destination, Source, Length);
}
