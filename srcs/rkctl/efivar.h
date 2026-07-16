#pragma once

#include <stddef.h>
#include <stdint.h>

#include "load_option.h"

typedef struct {
    size_t count;
    uint16_t options[];
} boot_order_t;

int efivar_boot_order_read(const boot_order_t** out);

int efivar_boot_order_write(const boot_order_t* boot_order);

int efivar_load_option_read(uint16_t index, const load_option_t** out);

int efivar_load_option_write(uint16_t index, const load_option_t* option);
