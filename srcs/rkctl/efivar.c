#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "efivar.h"
#include "utils.h"

// standard GUID for efi global variables
#define EFI_GLOBAL_GUID "8be4df61-93ca-11d2-aa0d-00e098032b8c"

// the path to Linux's efivars file to interact with the firmware
#define EFI_VAR_PATH "/sys/firmware/efi/efivars"

// the header for every efi variable
constexpr uint8_t EFI_VAR_HEADER[] = {0x07, 0x00, 0x00, 0x00};

static int efivar_read_global(const char* name, uint8_t* buffer, size_t* b_size) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s-%s", EFI_VAR_PATH, name, EFI_GLOBAL_GUID);

    struct stat st;
    if (stat(path, &st) != 0) {
        return errno;
    }

    if (st.st_size < (off_t)sizeof(EFI_VAR_HEADER)) {
        return -EINVAL;
    }

    const auto v_size = (size_t)st.st_size - sizeof(EFI_VAR_HEADER);

    if (buffer == nullptr) {
        *b_size = v_size;
        return 0;
    }

    if (*b_size < v_size) {
        return -ENOSPC;
    }

    __close_file const auto file = fopen(path, "rb");
    if (file == nullptr) {
        return errno;
    }

    uint8_t header[sizeof(EFI_VAR_HEADER)] = {0};
    if (fread(header, 1, sizeof(header), file) < sizeof(header)) {
        return errno ? errno : -EIO;
    }
    if (memcmp(header, EFI_VAR_HEADER, sizeof(header)) != 0) {
        return -EINVAL;
    }

    if (v_size > 0 && fread(buffer, 1, v_size, file) != v_size) {
        return errno ? errno : -EIO;
    }

    *b_size = v_size;

    return 0;
}

static int efivar_write_global(const char* name, const uint8_t* buffer, size_t size) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s-%s", EFI_VAR_PATH, name, EFI_GLOBAL_GUID);

    __close_file const auto file = fopen(path, "wb");
    if (file == nullptr) {
        return errno;
    }

    if (fwrite(EFI_VAR_HEADER, 1, sizeof(EFI_VAR_HEADER), file) != sizeof(EFI_VAR_HEADER)) {
        return errno;
    }
    if (fwrite(buffer, 1, size, file) != size) {
        return errno;
    }

    return 0;
}

int efivar_boot_order_read(const boot_order_t** out) {
    int ret = 0;

    size_t size = 0;
    ret = efivar_read_global("BootOrder", nullptr, &size);
    if (ret != 0) {
        return ret;
    }

    auto boot_order = (boot_order_t*)malloc(sizeof(boot_order_t) + size);
    if (boot_order == nullptr) {
        return -ENOMEM;
    }

    boot_order->count = size / 2;

    ret = efivar_read_global("BootOrder", (uint8_t*)boot_order->options, &size);
    if (ret != 0) {
        return ret;
    }

    *out = boot_order;
    return 0;
}

int efivar_boot_order_write(const boot_order_t* boot_order) {
    return efivar_write_global("BootOrder", (const uint8_t*)boot_order->options, boot_order->count * 2);
}

int efivar_load_option_read(uint16_t index, const load_option_t** out) {
    int ret = 0;

    char name[10];
    snprintf(name, sizeof(name), "Boot%04X", index);

    size_t size = 0;
    ret = efivar_read_global(name, nullptr, &size);
    if (ret != 0) {
        return ret;
    }

    auto option = (load_option_t*)malloc(sizeof(boot_order_t) + size);
    if (option == nullptr) {
        return -ENOMEM;
    }

    ret = efivar_read_global(name, (uint8_t*)option, &size);
    if (ret != 0) {
        return ret;
    }

    *out = option;
    return 0;
}

int efivar_load_option_write(uint16_t index, const load_option_t* option) {
    char name[10];
    snprintf(name, sizeof(name), "Boot%04X", index);

    const auto size = load_option_get_size(option);
    return efivar_write_global(name, (const uint8_t*)option, size);
}
