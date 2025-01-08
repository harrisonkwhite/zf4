#include "zf4ap.h"

bool pack_shaders(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjProgs) {
    const cJSON* cjProg = nullptr;

    cJSON_ArrayForEach(cjProg, cjProgs) {
        const cJSON* const cjVertShaderRelFilePath = cJSON_GetObjectItem(cjProg, "vsRelFilePath");
        const cJSON* const cjFragShaderRelFilePath = cJSON_GetObjectItem(cjProg, "fsRelFilePath");

        if (!cJSON_IsString(cjVertShaderRelFilePath) || !cJSON_IsString(cjFragShaderRelFilePath)) {
            zf4::log_error("Invalid shader program entry in packing instructions JSON file!");
            return false;
        }

        for (int i = 0; i < 2; ++i) {
            // Get the relative path of the shader.
            const char* const shaderRelFilePath = i == 0 ? cjVertShaderRelFilePath->valuestring : cjFragShaderRelFilePath->valuestring;
            
            if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, shaderRelFilePath)) {
                return false;
            }

            // Load and write out shader source.
            int shaderSrcLen;
            char* const shaderSrc = zf4::get_file_contents(srcAssetFilePathBuf, &shaderSrcLen);

            if (!shaderSrc) {
                zf4::log_error("Failed to load contents of shader file with path \"%s\"!", srcAssetFilePathBuf);
                return false;
            }

            if (shaderSrcLen > zf4::gk_shaderSrcLenLimit) {
                zf4::log_error("Shader source exceeds the length limit of %d!", zf4::gk_shaderSrcLenLimit);
                free(shaderSrc);
                return false;
            }

            fwrite(&shaderSrcLen, sizeof(shaderSrcLen), 1, outputFS);
            fwrite(shaderSrc, sizeof(*shaderSrc), shaderSrcLen, outputFS); // We don't write the '\0' here.

            free(shaderSrc);

            zf4::log("Packed shader with file path \"%s\".", srcAssetFilePathBuf);
        }
    }

    return true;
    
    /*
    
    // Pack vertex shader then fragment shader.
        for (int i = 0; i < 2; i++)
        {
            const std::filesystem::path shaderFilePath = inputDir / (i == 0 ? packingInfo.vertShaderRFP : packingInfo.fragShaderRFP);

            // Open the shader file.
            std::ifstream shaderIS(shaderFilePath, std::ios::binary | std::ios::ate);

            if (!shaderIS.is_open())
            {
                std::cerr << "ERROR: Failed to open shader " << shaderFilePath << "!" << std::endl;
                return false;
            }

            // Get the shader file size.
            const int shaderFileSize = shaderIS.tellg();
            shaderIS.seekg(0, std::ios::beg);

            // Write shader source size to assets file.
            const int shaderSrcSize = shaderFileSize + 1; // Account for the '\0' we're going to write.
            assetsFileOS.write(reinterpret_cast<const char *>(&shaderSrcSize), sizeof(shaderSrcSize));

            // Allocate buffer for shader content and read it.
            std::unique_ptr<char[]> shaderSrc(new char[shaderSrcSize]);

            shaderIS.read(shaderSrc.get(), shaderFileSize);
            shaderSrc[shaderSrcSize - 1] = '\0';

            // Write the shader content to assets file.
            assetsFileOS.write(shaderSrc.get(), shaderSrcSize);

            std::cout << "Successfully packed shader " << shaderFilePath << "." << std::endl;
        }
    
    */
}
