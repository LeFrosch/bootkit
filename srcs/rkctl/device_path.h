#pragma once

#include <stdint.h>

#include "wstr.h"

typedef struct {
    uint8_t type;
    uint8_t sub_type;
    uint16_t length;

    // --- variable length fields ---

    // uint16_t path_name[]; (0x04, 0x04)
    // device_partition_t partition; (0x04, 0x01)

    uint8_t data[];
} __attribute__((packed)) device_path_t;

// represent a partition on a hard drive
typedef struct {
    uint32_t number;
    uint64_t start;
    uint64_t size;
    uint8_t signature[16];
    uint8_t format;
    uint8_t signature_type;
} __attribute__((packed)) device_partition_t;

const device_partition_t* device_path_get_partition(const device_path_t* path);

const char* device_path_get_file(const device_path_t* path);

const device_path_t* device_path_create_partition(const device_partition_t* partition);

const device_path_t* device_path_create_file(const char* file);

const device_path_t* device_path_create_end();
