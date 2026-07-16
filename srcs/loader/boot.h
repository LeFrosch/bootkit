#pragma once

#include <efi.h>

/**
 * Tries to boot debian from the hard coded location for debug purposes.
 */
EFI_STATUS BootDebian(EFI_HANDLE ParentImageHandle);


/**
 * Tries to boot the next boot loader in the boot order.
 */
EFI_STATUS BootNext(EFI_HANDLE ParentImageHandle);
