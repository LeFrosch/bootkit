#pragma once

#include <stdint.h>

#include "device_path.h"

typedef struct {
    uint32_t attributes;
    uint16_t file_path_list_length;

    // --- variable length fields ---

    // uint16_t description[];
    // uint8_t file_path_list[];
    // uint8_t optional_data[];

    uint8_t data[];
} __attribute__((packed)) load_option_t;

typedef struct {
    size_t size;
    const device_path_t* head;
} device_path_list_t;

const char* load_option_get_description(const load_option_t* option);

device_path_list_t load_option_get_path_list(const load_option_t* option);

size_t load_option_get_size(const load_option_t* option);

const load_option_t* load_option_create(const char* description, device_path_list_t path_list);

void load_option_dump(const load_option_t* option);

device_path_list_t device_path_list_next(device_path_list_t path_list);

#define load_option_for_path(option, iter)                                                                             \
    for (device_path_list_t iter = load_option_get_path_list(option); iter.size > 0; iter = device_path_list_next(iter))
