#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/param.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "bitmap.h"

int main(int argc, char *argv[])
{
    /*
     * Parse command-line arguments
     */
    const char *ttf_filepath    = NULL;
    const char *challenge       = NULL; 
    size_t font_size            = 48;

    /* Pixel omission rate [0,100] */
    uint8_t omission = 0;

    /*
     * Radial distortion parameters
     */
    /* View plane */
    bool is_k   = false;
    double k    = 0.0;

    /* Center of projection */
    double z    = 2.0;

    int8_t opt;

    while((opt = getopt(argc, argv, "s:o::k::z::")) != -1)
    {
        switch(opt)
        {
            case 's':

                if(!(font_size = atoi(optarg)))
                {
                    fprintf(stderr, "WARNING: Invalid font size '%s' (-s)\n", optarg);
                    font_size = 48;
                }

                break;

            case 'o':

                if(optarg)
                {
                    omission = MIN(atoi(optarg), 100);
                }

                break;

            case 'k':

                if(optarg)
                {
                    k = atof(optarg);
                }

                is_k = true;
                break;

            case 'z':

                if(optarg)
                {
                    z = atof(optarg);
                }
                break;
        } 
    }

    if(optind + 1 > argc)
    {
        fprintf(stderr, "ERROR: Missing required arguments (TTF_FILEPATH, CHALLENGE)\n");
        return -1;
    }

    if((ttf_filepath = argv[optind]) == NULL)
    {
        fprintf(stderr, "ERROR: Missing required argument (TTF_FILEPATH)\n");
        return -1;
    }

    if((challenge = argv[optind + 1]) == NULL)
    {
        fprintf(stderr, "ERROR: Missing required argument (CHALLENGE)\n");
        return -1;
    }

#ifdef DEBUG
    fprintf(stdout, "FONT_SIZE: %ld; TTF_FILEPATH: %s; CHALLENGE: %s;\n", font_size, ttf_filepath, challenge);
    fprintf(stdout, "OMISSION: %d\n", omission);
#endif

    /*
     * Load and configure font
     */
    FT_Error ft_error;
    FT_Library ft_library;
    
    if((ft_error = FT_Init_FreeType(&ft_library)))
    {
        fprintf(stderr, "ERROR #%d: %s\n", ft_error, FT_Error_String(ft_error));
        return -1;
    }

    FT_Face ft_face;

    if((ft_error = FT_New_Face(ft_library, ttf_filepath, 0, &ft_face)))
    {
        fprintf(stderr, "ERROR #%d: %s\n", ft_error, FT_Error_String(ft_error));
        FT_Done_FreeType(ft_library);
        return -1;
    }

    if((ft_error = FT_Set_Pixel_Sizes(ft_face, font_size, 0)))
    {
        fprintf(stderr, "WARNING #%d: %s\n", ft_error, FT_Error_String(ft_error));
    }

    /*
     * Generate distorted bitmap of challenge
     */
    size_t bitmap_ascender = (double) ft_face->ascender / (double) ft_face->units_per_EM * font_size;
    size_t bitmap_descender = (double) ft_face->descender / (double) ft_face->units_per_EM * font_size;
    size_t bitmap_cols = font_size * (strlen(challenge) + 2);
    size_t bitmap_rows = (bitmap_ascender - bitmap_descender) * 3;
    size_t bitmap_advance_x = font_size;
    size_t bitmap_advance_y = (2 * bitmap_rows / 3) + bitmap_descender;

    uint8_t *bitmap = calloc(bitmap_cols * bitmap_rows, sizeof(uint8_t));

    for(const char *i = challenge; *i != '\0'; i++)
    {
        if((ft_error = FT_Load_Glyph(ft_face, FT_Get_Char_Index(ft_face, *i), FT_LOAD_RENDER)))
        {
            fprintf(stderr, "ERROR #%d: %s\n", ft_error, FT_Error_String(ft_error));
            free(bitmap);
            FT_Done_Face(ft_face);
            FT_Done_FreeType(ft_library);
            return -1;
        }

        FT_GlyphSlot glyph = ft_face->glyph;

        size_t glyph_cols = glyph->bitmap.width;
        size_t glyph_rows = glyph->bitmap.rows;
        size_t glyph_left = glyph->bitmap_left;
        size_t glyph_top = glyph->bitmap_top;

        double center_x = (double) glyph_cols / 2.0;
        double center_y = (double) glyph_rows / 2.0;

        /* Radial distortion model */
        double radius = (double) MAX(glyph_cols, glyph_rows) / 2.0;
        double eye_x = center_x;
        double eye_y = center_y;
        double eye_z = radius * z;

        for(size_t row = 0; row < glyph_rows; row++)
        {
            for(size_t col = 0; col < glyph_cols; col++)
            {
                uint8_t color   = glyph->bitmap.buffer[row * glyph_cols + col] ? 1 : 0;
                size_t x_offset = col + glyph_left + bitmap_advance_x;
                size_t y_offset = row - glyph_top + bitmap_advance_y;

                if(omission && color)
                {
                    color = (rand() % 100) >= omission;
                }

                if(is_k)
                {
                    /* Orthographic projection on glyph from plane z = 0 to sphere */
                    double z  = sqrt(pow(radius, 2.0) - pow(col - eye_x, 2.0) - pow(row - eye_y, 2.0));

                    /* Perspective projection on glyph from sphere to plane z = k * radius */
                    double t  = (eye_z - k * radius) / (eye_z - z);
                    double dx = eye_x - (eye_x - col) * t;
                    double dy = eye_y - (eye_y - row) * t;

                    x_offset = (size_t) MIN(MAX(round(dx) + glyph_left + bitmap_advance_x, 0), bitmap_cols);
                    y_offset = (size_t) MIN(MAX(round(dy) - glyph_top + bitmap_advance_y, 0), bitmap_rows);
                }

                bitmap[y_offset * bitmap_cols + x_offset] = color;
            }
        }  
        
        bitmap_advance_x += glyph->advance.x / 64;
    }

    bitmap_advance_x += font_size;

    FILE *output = fopen("captcha.bmp", "w");
    bmp_save(output, bitmap, bitmap_cols, bitmap_rows, bitmap_advance_x, bitmap_rows);
    
    fclose(output);
    free(bitmap);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);
    return 0;
}