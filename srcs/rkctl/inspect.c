#include <stdlib.h>

#include "efivar.h"
#include "inspect.h"
#include "load_option.h"
#include "utils.h"

void rk_inspect(void) {
    __free const boot_order_t* boot_order = nullptr;
    try(efivar_boot_order_read(&boot_order));

    for (size_t i = 0; i < boot_order->count; i++) {
        const auto index = boot_order->options[i];

        __free const load_option_t* option;
        try(efivar_load_option_read(index, &option));

        __free const auto description = load_option_get_description(option);
        log("Boot%04d: %s", index, description);

        load_option_for_path(option, list) {
            __free const auto file = device_path_get_file(list.head);
            if (file != nullptr) {
                log("-> file: %s", file);
                continue;
            }

            __free const auto partition = device_path_get_partition(list.head);
            if (partition != nullptr) {
                log("-> partition: number %u, format 0x%02x", partition->number, partition->format);
                continue;
            }
        }
    }
}
