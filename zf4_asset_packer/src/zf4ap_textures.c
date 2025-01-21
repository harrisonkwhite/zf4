#include "zf4ap.h"

#include <stb_image.h>

bool PackTextures(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_textures) {
    const cJSON* cj_tex_rel_file_path = NULL;

    cJSON_ArrayForEach(cj_tex_rel_file_path, cj_textures) {
        if (!cJSON_IsString(cj_tex_rel_file_path)) {
            LogError("Failed to get texture file path from packing instructions JSON file!");
            return false;
        }

        if (!CompleteAssetFilePath(src_asset_file_path_buf, src_asset_file_path_start_len, cj_tex_rel_file_path->valuestring)) {
            return false;
        }

        // Load and write the size and pixel data of the texture.
        s_vec_2d_i tex_size;
        stbi_uc* const tex_px_data = stbi_load(src_asset_file_path_buf, &tex_size.x, &tex_size.y, NULL, TEXTURE_CHANNEL_CNT);

        if (!tex_px_data) {
            LogError("Failed to load texture with file path \"%s\"!", src_asset_file_path_buf);
            return false;
        }

        fwrite(&tex_size, sizeof(tex_size), 1, output_fs);
        fwrite(tex_px_data, sizeof(*tex_px_data), tex_size.x * tex_size.y * TEXTURE_CHANNEL_CNT, output_fs);

        Log("Packed texture with file path \"%s\".", src_asset_file_path_buf);

        stbi_image_free(tex_px_data);
    }

    return true;
}
