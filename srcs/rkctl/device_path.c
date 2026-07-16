#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "device_path.h"
#include "utils.h"
#include "wstr.h"

const device_partition_t* device_path_get_partition(const device_path_t* path) {
    if (path->type != 0x04 || path->sub_type != 0x01) {
        return nullptr;
    }

    auto partition = (device_partition_t*)malloc(sizeof(device_partition_t));
    if (partition == nullptr) {
        return nullptr;
    }

    memcpy(partition, path->data, sizeof(device_partition_t));
    return partition;
}

const char* device_path_get_file(const device_path_t* path) {
    if (path->type != 0x04 || path->sub_type != 0x04) {
        return nullptr;
    }

    return wstr_to_cstr((wchar*)path->data);
}

const device_path_t* device_path_create_partition(const device_partition_t* partition) {
    const uint16_t size = sizeof(device_path_t) + sizeof(device_partition_t);

    auto path = (device_path_t*)malloc(size);
    if (path == nullptr) {
        return nullptr;
    }

    path->type = 0x04;
    path->sub_type = 0x01;
    path->length = size;

    memcpy(path->data, partition, sizeof(device_partition_t));

    return path;
}

const device_path_t* device_path_create_file(const char* file) {
    __free const auto w_file = wstr_from_cstr(file);
    if (w_file == nullptr) {
        return nullptr;
    }

    const uint16_t size = sizeof(device_path_t) + (uint16_t)wstr_size(w_file);

    auto path = (device_path_t*)malloc(size);
    if (path == nullptr) {
        return nullptr;
    }

    path->type = 0x04;
    path->sub_type = 0x04;
    path->length = size;
    memcpy(path->data, w_file, wstr_size(w_file));

    return path;
}

const device_path_t* device_path_create_end() {
    auto path = (device_path_t*)malloc(sizeof(device_path_t));
    if (path == nullptr) {
        return nullptr;
    }

    path->type = 0x7f;
    path->sub_type = 0xff;
    path->length = sizeof(device_path_t);

    return path;
}

