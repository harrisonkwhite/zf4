#include "zf4ap.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct font_data {
    s_font_arrangement_info arrangement_info;
    s_vec_2d_i tex_size;
    ta_byte tex_px_data[TEXTURE_PX_DATA_SIZE_LIMIT];
} s_font_data;

static inline int GetLineHeight(const FT_Face ft_face) {
    return ft_face->size->metrics.height >> 6;
}

static int CalcLargestBitmapWidth(const FT_Face ft_face) {
    int width = 0;

    for (int i = 0; i < FONT_CHAR_RANGE_LEN; i++) {
        FT_Load_Glyph(ft_face, FT_Get_Char_Index(ft_face, FONT_CHAR_RANGE_BEGIN + i), FT_LOAD_DEFAULT);
        FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

        if ((int)ft_face->glyph->bitmap.width > width) {
            width = ft_face->glyph->bitmap.width;
        }
    }

    return width;
}

static s_vec_2d_i CalcFontTexSize(const FT_Face ft_face) {
    const int largest_glyph_bitmap_width = CalcLargestBitmapWidth(ft_face);
    const int ideal_tex_width = largest_glyph_bitmap_width * FONT_CHAR_RANGE_LEN;

    const s_vec_2d_i size = {
        .x = ZF4_MIN(ideal_tex_width, TEXTURE_WIDTH_LIMIT),
        .y = GetLineHeight(ft_face) * ((ideal_tex_width / TEXTURE_WIDTH_LIMIT) + 1)
    };

    return size;
}

static bool LoadFontData(s_font_data* const fd, const FT_Library ft_lib, const char* const file_path, const int pt_size) {
    assert(IsClear(fd, sizeof(*fd)));

    FT_Face ft_face;

    if (FT_New_Face(ft_lib, file_path, 0, &ft_face)) {
        LogError("Failed to create a FreeType face object for font with file path %s.", file_path);
        return false;
    }

    FT_Set_Char_Size(ft_face, pt_size << 6, 0, 96, 0);

    fd->arrangement_info.line_height = GetLineHeight(ft_face);

    fd->tex_size = CalcFontTexSize(ft_face);

    if (fd->tex_size.y > TEXTURE_HEIGHT_LIMIT) {
        LogError("Font texture size is too large!");
        FT_Done_Face(ft_face);
        return false;
    }

    const int tex_px_data_size = TEXTURE_CHANNEL_CNT * fd->tex_size.x * fd->tex_size.y;

    for (int i = 0; i < tex_px_data_size; i += TEXTURE_CHANNEL_CNT) {
        // Initialise to transparent white.
        fd->tex_px_data[i + 0] = 255;
        fd->tex_px_data[i + 1] = 255;
        fd->tex_px_data[i + 2] = 255;
        fd->tex_px_data[i + 3] = 0;
    }

    s_vec_2d_i char_draw_pos = {0}; // Where we are in the font texture.

    for (int i = 0; i < FONT_CHAR_RANGE_LEN; i++) {
        const FT_UInt ft_char_index = FT_Get_Char_Index(ft_face, FONT_CHAR_RANGE_BEGIN + i);

        FT_Load_Glyph(ft_face, ft_char_index, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

        // If we cannot horizontally fit this character's texture pixel data, move to a new line.
        if (char_draw_pos.x + ft_face->glyph->bitmap.width > TEXTURE_WIDTH_LIMIT) {
            char_draw_pos.x = 0;
            char_draw_pos.y += fd->arrangement_info.line_height;
        }

        // Get general character arrangement information.
        fd->arrangement_info.chars.hor_offsets[i] = ft_face->glyph->metrics.horiBearingX >> 6;
        fd->arrangement_info.chars.ver_offsets[i] = (ft_face->size->metrics.ascender - ft_face->glyph->metrics.horiBearingY) >> 6;
        fd->arrangement_info.chars.hor_advances[i] = ft_face->glyph->metrics.horiAdvance >> 6;

        fd->arrangement_info.chars.src_rects[i].x = char_draw_pos.x;
        fd->arrangement_info.chars.src_rects[i].y = char_draw_pos.y;
        fd->arrangement_info.chars.src_rects[i].width = ft_face->glyph->bitmap.width;
        fd->arrangement_info.chars.src_rects[i].height = ft_face->glyph->bitmap.rows;

        // Get kernings for all character pairings.
        for (int j = 0; j < FONT_CHAR_RANGE_LEN; j++) {
            FT_Vector ft_kerning;
            FT_Get_Kerning(ft_face, FT_Get_Char_Index(ft_face, FONT_CHAR_RANGE_BEGIN + j), ft_char_index, FT_KERNING_DEFAULT, &ft_kerning);
            fd->arrangement_info.chars.kernings[(FONT_CHAR_RANGE_LEN * i) + j] = ft_kerning.x >> 6;
        }

        // Set the pixel data (alpha values only) for the character.
        for (int y = 0; y < fd->arrangement_info.chars.src_rects[i].height; y++) {
            for (int x = 0; x < fd->arrangement_info.chars.src_rects[i].width; x++) {
                const unsigned char px_alpha = ft_face->glyph->bitmap.buffer[(y * ft_face->glyph->bitmap.width) + x];

                if (px_alpha > 0) {
                    const int px_x = fd->arrangement_info.chars.src_rects[i].x + x;
                    const int px_y = fd->arrangement_info.chars.src_rects[i].y + y;
                    const int px_data_index = (px_y * fd->tex_size.x * TEXTURE_CHANNEL_CNT) + (px_x * TEXTURE_CHANNEL_CNT);

                    fd->tex_px_data[px_data_index + 3] = px_alpha;
                }
            }
        }

        char_draw_pos.x += fd->arrangement_info.chars.src_rects[i].width;
    }

    FT_Done_Face(ft_face);

    return true;
}

bool PackFonts(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_fonts) {
    FT_Library ft_lib;

    if (FT_Init_FreeType(&ft_lib)) {
        LogError("Failed to initialise FreeType!");
        return false;
    }

    s_font_data* const font_data = malloc(sizeof(*font_data)); // A buffer reused for all fonts.

    if (!font_data) {
        LogError("Failed to allocate memory for font data!");
        FT_Done_FreeType(ft_lib);
        return false;
    }

    bool success = true;

    const cJSON* cj_font = NULL;

    cJSON_ArrayForEach(cj_font, cj_fonts) {
        const cJSON* const cj_rel_file_path = cJSON_GetObjectItem(cj_font, "rel_file_path");
        const cJSON* const cj_pt_size = cJSON_GetObjectItem(cj_font, "pt_size");

        if (!cJSON_IsString(cj_rel_file_path) || !cJSON_IsNumber(cj_pt_size)) {
            LogError("Invalid font entry in packing instructions JSON file!");
            success = false;
            break;
        }

        // Get the relative path of the font.
        if (!CompleteAssetFilePath(src_asset_file_path_buf, src_asset_file_path_start_len, cj_rel_file_path->valuestring)) {
            success = false;
            break;
        }

        Clear(font_data, sizeof(*font_data));

        if (!LoadFontData(font_data, ft_lib, src_asset_file_path_buf, cj_pt_size->valueint)) {
            success = false;
            break;
        }

        fwrite(font_data, sizeof(*font_data), 1, output_fs);

        Log("Packed font with file path \"%s\" and point size %d.", src_asset_file_path_buf, cj_pt_size->valueint);
    }

    free(font_data);
    FT_Done_FreeType(ft_lib);

    return success;
}
