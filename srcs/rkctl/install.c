#include "install.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>

#include "device_path.h"
#include "efivar.h"
#include "load_option.h"
#include "utils.h"
#include "install.h"

static int find_debian_load_option(const load_option_t** out) {
    int ret = 0;

    __free const boot_order_t* boot_order = nullptr;
    ret = efivar_boot_order_read(&boot_order);
    if (ret != 0) {
        return ret;
    }

    for (size_t i = 0; i < boot_order->count; i++) {
        const load_option_t* option;
        ret = efivar_load_option_read(boot_order->options[i], &option);
        if (ret != 0) {
            continue;
        }

        __free const auto description = load_option_get_description(option);
        if (strstr(description, "debian") == nullptr) {
            free((void*)option);
        }

        *out = option;
        return 0;
    }

    return -ENOENT;
}

static int load_option_find_partition(const load_option_t* option, const device_partition_t** out) {
    load_option_for_path(option, list) {
        const auto partition = device_path_get_partition(list.head);
        if (partition == nullptr) {
            continue;
        }

        *out = partition;
        return 0;
    }

    return -ENOENT;
}

static int load_option_find_file(const load_option_t* option, const char** out) {
    load_option_for_path(option, list) {
        const auto file = device_path_get_file(list.head);
        if (file == nullptr) {
            continue;
        }

        *out = file;
        return 0;
    }

    return -ENOENT;
}

static int rk_create_load_option(
    const char* description,
    const char* file,
    const device_partition_t* partition,
    const load_option_t** out
) {
    __free const auto path_partition = device_path_create_partition(partition);
    if (path_partition == nullptr) {
        return -ENOMEM;
    }

    __free const auto path_file = device_path_create_file(file);
    if (path_file == nullptr) {
        return -ENOMEM;
    }

    __free const auto path_end = device_path_create_end();
    if (path_end == nullptr) {
        return -ENOMEM;
    }

    const size_t size = (size_t)(path_partition->length + path_file->length + path_end->length);

    __free auto buffer = malloc(size);
    if (buffer == nullptr) {
        return -ENOMEM;
    }

    uint8_t* ptr = buffer;
    memcpy(ptr, path_partition, path_partition->length);

    ptr += path_partition->length;
    memcpy(ptr, path_file, path_file->length);

    ptr += path_file->length;
    memcpy(ptr, path_end, path_end->length);

    const auto option = load_option_create(description, (device_path_list_t){.head = buffer, .size = size});
    if (option == nullptr) {
        return -ENOMEM;
    }

    *out = option;
    return 0;
}

static int rk_create_boot_order(const boot_order_t** out) {
    __free const boot_order_t* boot_order = nullptr;
    const auto ret = efivar_boot_order_read(&boot_order);
    if (ret != 0) {
        return ret;
    }

    uint16_t index = 0;
    for (size_t i = 0; i < boot_order->count; i++) {
        const auto k = boot_order->options[i];
        index = index > k ? index : k + 1;
    }

    const auto size = sizeof(boot_order_t) + sizeof(uint16_t) * (boot_order->count + 1);

    auto upated_order = (boot_order_t*) malloc(size);
    if (upated_order == nullptr) {
        return -ENOMEM;
    }

    upated_order->count = boot_order->count + 1;
    upated_order->options[0] = index;
    memcpy(upated_order->options + 1, boot_order->options, sizeof(uint16_t) * boot_order->count);

    *out = upated_order;
    return 0;
}

void rk_install(void) {
    __free const load_option_t* debian_option = nullptr;
    try(find_debian_load_option(&debian_option));

    load_option_dump(debian_option);

    __free const char* debian_file = nullptr;
    try(load_option_find_file(debian_option, &debian_file));

    __free const device_partition_t* debian_partition = nullptr;
    try(load_option_find_partition(debian_option, &debian_partition));

    log("Debian load option is valid!");

    __free const load_option_t* rk_option = nullptr;
    try(rk_create_load_option("RK Bootloader", "\\EFI\\app.efi", debian_partition, &rk_option));

    __free const boot_order_t* rk_boot_order = nullptr;
    try(rk_create_boot_order(&rk_boot_order));

    log("Select boot index: %d", rk_boot_order->options[0]);
    load_option_dump(rk_option);

    log("Writing RK load option...");
    try(efivar_load_option_write(rk_boot_order->options[0], rk_option));

    log("Updating boot order...");
    try(efivar_boot_order_write(rk_boot_order));
}
