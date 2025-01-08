#pragma once

#include <glad/glad.h>

namespace zf4 {
    constexpr int gk_texturedQuadShaderProgVertCnt = 11;

    struct TexturedQuadShaderProg {
        GLuint glID;
        int projUniLoc;
        int viewUniLoc;
        int texturesUniLoc;
    };

    struct ShaderProgs {
        TexturedQuadShaderProg texturedQuad;
    };

    void load_shader_progs(ShaderProgs* const progs);
    void unload_shader_progs(ShaderProgs* const progs);
}
