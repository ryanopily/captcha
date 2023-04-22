#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "bitmap.h"

/*
 * http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
 */
void bmp_save(FILE *bmp, uint8_t *data, size_t width, size_t height, size_t crop_x, size_t crop_y)
{
    size_t header_length = HEADER_BYTES + INFO_HEADER_BYTES + 2 * COLOR_TABLE_BYTES;
    size_t data_length = crop_x * crop_y;
    size_t padding_length = -crop_x % 4;
    size_t file_length = header_length + data_length + crop_y * padding_length; 

    uint8_t padding[padding_length];
    memset(padding, 0x0, padding_length);
    
    uint8_t header[header_length];

    /*
     * Header
     */
    *(uint16_t *)((void *) header + 0x0) = ('M' << __CHAR_BIT__ | 'B');
    *(uint32_t *)((void *) header + 0x2) = (uint32_t) file_length;
    *(uint32_t *)((void *) header + 0x6) = 0;
    *(uint32_t *)((void *) header + 0xA) = header_length;
   
    /*
     * Info header
     */
    *(uint32_t *)((void *) header + 0xE) = 40;
    *(uint32_t *)((void *) header + 0x12) = (uint32_t) crop_x;
    *(uint32_t *)((void *) header + 0x16) = (uint32_t) crop_y;
    *(uint16_t *)((void *) header + 0x1A) = 1;
    *(uint16_t *)((void *) header + 0x1C) = 8;
    *(uint32_t *)((void *) header + 0x1E) = BI_RGB;
    *(uint32_t *)((void *) header + 0x22) = 0;
    *(uint32_t *)((void *) header + 0x26) = 0;
    *(uint32_t *)((void *) header + 0x2A) = 0;
    *(uint32_t *)((void *) header + 0x2E) = 2;
    *(uint32_t *)((void *) header + 0x32) = 0;

    /*
     * Color table
     */
    *((uint32_t *)((void *) header + 0x36) + 0) = 0x00000000;         
    *((uint32_t *)((void *) header + 0x36) + 1) = 0x00FFFFFF;

    fwrite(header, sizeof(uint8_t), header_length, bmp);

    for(int64_t row = crop_y - 1; row >= 0; row--)
    {
        fwrite(data + row * width, sizeof(uint8_t), crop_x, bmp);
        fwrite(padding, sizeof(uint8_t), padding_length, bmp);
    }

    fflush(bmp);
} 