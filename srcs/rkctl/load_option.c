#include <stdint.h>

#include "device_path.h"
#include "load_option.h"
#include "utils.h"
#include "wstr.h"

const char* load_option_get_description(const load_option_t* option) {
    return wstr_to_cstr((uint16_t*)option->data);
}

device_path_list_t load_option_get_path_list(const load_option_t* option) {
    return (device_path_list_t){
        .size = option->file_path_list_length,
        .head = (device_path_t*)(option->data + wstr_size((uint16_t*)option->data)),
    };
}

size_t load_option_get_size(const load_option_t* option) {
    return sizeof(load_option_t) + wstr_size((uint16_t*)option->data) + option->file_path_list_length;
}

const load_option_t* load_option_create(const char* description, device_path_list_t path_list) {
    __free const auto w_description = wstr_from_cstr(description);
    if (w_description == nullptr) {
        return nullptr;
    }

    const auto w_description_size = wstr_size(w_description);

    auto option = (load_option_t*)malloc(sizeof(load_option_t) + w_description_size + path_list.size);
    if (option == nullptr) {
        return nullptr;
    }

    option->attributes = 0x01;
    option->file_path_list_length = (uint16_t)path_list.size;

    memcpy(option->data, w_description, w_description_size);
    memcpy(option->data + w_description_size, path_list.head, path_list.size);

    return option;
}

device_path_list_t device_path_list_next(device_path_list_t path_list) {
    if (path_list.size < sizeof(device_path_t) || path_list.head == nullptr) {
        return (device_path_list_t){0};
    }

    return (device_path_list_t){
        .size = path_list.size - path_list.head->length,
        .head = (device_path_t*)((uint8_t*)path_list.head + path_list.head->length),
    };
}

void load_option_dump(const load_option_t* option) {
    __free const auto description = load_option_get_description(option);
    log("Load Option: %s", description);

    load_option_for_path(option, list) {
        __free const auto file = device_path_get_file(list.head);
        if (file != nullptr) {
            log("-> file: %s", file);
            continue;
        }

        __free const auto partition = device_path_get_partition(list.head);
        if (partition != nullptr) {
            log("-> partition: number %u, format 0x%02x, size: 0x%lx, start: 0x%lx, signature: %d", partition->number, partition->format, partition->size, partition->start, partition->signature_type);
            continue;
        }

        log("-> node: type 0x%02x, sub-type 0x%02x", list.head->type, list.head->sub_type);
    }
}
