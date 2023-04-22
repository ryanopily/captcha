#pragma once
#include <stdio.h>
#include <stdint.h>

#define HEADER_BYTES 14
#define INFO_HEADER_BYTES 40
#define COLOR_TABLE_BYTES 4

#define BI_RGB 0

void bmp_save(FILE *, uint8_t *, size_t, size_t, size_t, size_t);