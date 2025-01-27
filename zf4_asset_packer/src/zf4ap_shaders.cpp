#include "zf4ap.h"

namespace zf4 {
    bool PackShaders(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_progs) {
        const cJSON* cj_prog = nullptr;

        cJSON_ArrayForEach(cj_prog, cj_progs) {
            const cJSON* const cj_vert_shader_rel_file_path = cJSON_GetObjectItem(cj_prog, "vs_rel_file_path");
            const cJSON* const cj_frag_shader_rel_file_path = cJSON_GetObjectItem(cj_prog, "fs_rel_file_path");

            if (!cJSON_IsString(cj_vert_shader_rel_file_path) || !cJSON_IsString(cj_frag_shader_rel_file_path)) {
                LogError("Invalid shader program entry in packing instructions JSON file!");
                return false;
            }

            for (int i = 0; i < 2; ++i) {
                // Get the relative path of the shader.
                const char* const shader_rel_file_path = i == 0 ? cj_vert_shader_rel_file_path->valuestring : cj_frag_shader_rel_file_path->valuestring;

                if (!CompleteAssetFilePath(src_asset_file_path_buf, src_asset_file_path_start_len, shader_rel_file_path)) {
                    return false;
                }

                // Load and write out shader source.
                int shader_src_len;
                char* const shader_src = GetFileContents(src_asset_file_path_buf, &shader_src_len);

                if (!shader_src) {
                    LogError("Failed to load contents of shader file with path \"%s\"!", src_asset_file_path_buf);
                    return false;
                }

                if (shader_src_len > g_shader_src_len_limit) {
                    LogError("Shader source exceeds the length limit of %d!", g_shader_src_len_limit);
                    free(shader_src);
                    return false;
                }

                fwrite(&shader_src_len, sizeof(shader_src_len), 1, output_fs);
                fwrite(shader_src, sizeof(*shader_src), shader_src_len, output_fs); // We don't write the '\0' here.

                free(shader_src);

                Log("Packed shader with file path \"%s\".", src_asset_file_path_buf);
            }
        }

        return true;
    }
}
