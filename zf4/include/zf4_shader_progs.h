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

    struct TestShaderProg {
        GLuint glID;
    };

    struct ShaderProgs {
        TexturedQuadShaderProg texturedQuad;
        TestShaderProg test;
    };

    void load_shader_progs(ShaderProgs* const progs);
    void unload_shader_progs(ShaderProgs* const progs);
}
