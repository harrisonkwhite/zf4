#include "zf4ap.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct FontData {
    zf4::FontArrangementInfo arrangementInfo;
    zf4::Vec2DI texSize;
    zf4::Byte texPxData[zf4::gk_texPxDataSizeLimit];
};

static inline int get_line_height(const FT_Face ftFace) {
    return ftFace->size->metrics.height >> 6;
}

static int calc_largest_bitmap_width(const FT_Face ftFace) {
    int width = 0;

    for (int i = 0; i < zf4::gk_fontCharRangeLen; i++) {
        FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, zf4::gk_fontCharRangeBegin + i), FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        if ((int)ftFace->glyph->bitmap.width > width) {
            width = ftFace->glyph->bitmap.width;
        }
    }

    return width;
}

static zf4::Vec2DI calc_font_tex_size(const FT_Face ftFace) {
    const int largestGlyphBitmapWidth = calc_largest_bitmap_width(ftFace);
    const int idealTexWidth = largestGlyphBitmapWidth * zf4::gk_fontCharRangeLen;

    zf4::Vec2DI texSize;
    texSize.x = idealTexWidth < zf4::gk_texSizeLimit.x ? idealTexWidth : zf4::gk_texSizeLimit.x;
    texSize.y = get_line_height(ftFace) * ((idealTexWidth / zf4::gk_texSizeLimit.x) + 1);
    return texSize;
}

static bool load_font_data(FontData* const fd, const FT_Library ftLib, const char* const filePath, const int ptSize) {
    assert(zf4::is_zero(fd));

    FT_Face ftFace;

    if (FT_New_Face(ftLib, filePath, 0, &ftFace)) {
        zf4::log_error("Failed to create a FreeType face object for font with file path %s.", filePath);
        return false;
    }

    FT_Set_Char_Size(ftFace, ptSize << 6, 0, 96, 0);

    fd->arrangementInfo.lineHeight = get_line_height(ftFace);

    fd->texSize = calc_font_tex_size(ftFace);

    if (fd->texSize.y > zf4::gk_texSizeLimit.y) {
        zf4::log_error("Font texture size is too large!");
        FT_Done_Face(ftFace);
        return false;
    }

    const int texPxDataSize = zf4::gk_texChannelCnt * fd->texSize.x * fd->texSize.y;

    for (int i = 0; i < texPxDataSize; i += zf4::gk_texChannelCnt) {
        // Initialise to transparent white.
        fd->texPxData[i + 0] = 255;
        fd->texPxData[i + 1] = 255;
        fd->texPxData[i + 2] = 255;
        fd->texPxData[i + 3] = 0;
    }

    zf4::Vec2DI charDrawPos = {}; // Where we are in the font texture.

    for (int i = 0; i < zf4::gk_fontCharRangeLen; i++) {
        FT_UInt ftCharIndex = FT_Get_Char_Index(ftFace, zf4::gk_fontCharRangeBegin + i);

        FT_Load_Glyph(ftFace, ftCharIndex, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        // If we cannot horizontally fit this character's texture pixel data, move to a new line.
        if (charDrawPos.x + ftFace->glyph->bitmap.width > zf4::gk_texSizeLimit.x) {
            charDrawPos.x = 0;
            charDrawPos.y += fd->arrangementInfo.lineHeight;
        }

        // Get general character arrangement information.
        fd->arrangementInfo.chars.horOffsets[i] = ftFace->glyph->metrics.horiBearingX >> 6;
        fd->arrangementInfo.chars.verOffsets[i] = (ftFace->size->metrics.ascender - ftFace->glyph->metrics.horiBearingY) >> 6;
        fd->arrangementInfo.chars.horAdvances[i] = ftFace->glyph->metrics.horiAdvance >> 6;

        fd->arrangementInfo.chars.srcRects[i].x = charDrawPos.x;
        fd->arrangementInfo.chars.srcRects[i].y = charDrawPos.y;
        fd->arrangementInfo.chars.srcRects[i].width = ftFace->glyph->bitmap.width;
        fd->arrangementInfo.chars.srcRects[i].height = ftFace->glyph->bitmap.rows;

        // Get kernings for all character pairings.
        for (int j = 0; j < zf4::gk_fontCharRangeLen; j++) {
            FT_Vector ftKerning;
            FT_Get_Kerning(ftFace, FT_Get_Char_Index(ftFace, zf4::gk_fontCharRangeBegin + j), ftCharIndex, FT_KERNING_DEFAULT, &ftKerning);
            fd->arrangementInfo.chars.kernings[(zf4::gk_fontCharRangeLen * i) + j] = ftKerning.x >> 6;
        }

        // Set the pixel data (alpha values only) for the character.
        for (int y = 0; y < fd->arrangementInfo.chars.srcRects[i].height; y++) {
            for (int x = 0; x < fd->arrangementInfo.chars.srcRects[i].width; x++) {
                unsigned char pxAlpha = ftFace->glyph->bitmap.buffer[(y * ftFace->glyph->bitmap.width) + x];

                if (pxAlpha > 0) {
                    int pxX = fd->arrangementInfo.chars.srcRects[i].x + x;
                    int pxY = fd->arrangementInfo.chars.srcRects[i].y + y;
                    int pxDataIndex = (pxY * fd->texSize.x * zf4::gk_texChannelCnt) + (pxX * zf4::gk_texChannelCnt);

                    fd->texPxData[pxDataIndex + 3] = pxAlpha;
                }
            }
        }

        charDrawPos.x += fd->arrangementInfo.chars.srcRects[i].width;
    }

    FT_Done_Face(ftFace);

    return true;
}

bool pack_fonts(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjFonts) {
    FT_Library ftLib;

    if (FT_Init_FreeType(&ftLib)) {
        zf4::log_error("Failed to initialise FreeType!");
        return false;
    }

    const auto fontData = zf4::alloc<FontData>(); // A buffer reused for all fonts.

    if (!fontData) {
        zf4::log_error("Failed to allocate memory for font data!");
        FT_Done_FreeType(ftLib);
        return false;
    }

    bool success = true;

    const cJSON* cjFont = nullptr;

    cJSON_ArrayForEach(cjFont, cjFonts) {
        const cJSON* const cjRelFilePath = cJSON_GetObjectItem(cjFont, "relFilePath");
        const cJSON* const cjPtSize = cJSON_GetObjectItem(cjFont, "ptSize");

        if (!cJSON_IsString(cjRelFilePath) || !cJSON_IsNumber(cjPtSize)) {
            zf4::log_error("Invalid font entry in packing instructions JSON file!");
            success = false;
            break;
        }

        // Get the relative path of the font.
        if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, cjRelFilePath->valuestring)) {
            success = false;
            break;
        }

        memset(fontData, 0, sizeof(*fontData));

        if (!load_font_data(fontData, ftLib, srcAssetFilePathBuf, cjPtSize->valueint)) {
            success = false;
            break;
        }

        fwrite(fontData, sizeof(*fontData), 1, outputFS);

        zf4::log("Packed font with file path \"%s\" and point size %d.", srcAssetFilePathBuf, cjPtSize->valueint);
    }

    free(fontData);
    FT_Done_FreeType(ftLib);

    return success;
}
