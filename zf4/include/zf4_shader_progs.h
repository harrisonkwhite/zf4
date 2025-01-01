#pragma once

#include <cstdbool>
#include <glad/glad.h>

namespace zf4 {
    constexpr int gk_spriteQuadShaderProgVertCnt = 11;
    constexpr int gk_charQuadShaderProgVertCnt = 4;

    struct SpriteQuadShaderProg {
        GLuint glID;
        int projUniLoc;
        int viewUniLoc;
        int texturesUniLoc;
    };

    typedef struct {
        GLuint glID;
        int projUniLoc;
        int viewUniLoc;
        int posUniLoc;
        int rotUniLoc;
        int blendUniLoc;
    } CharQuadShaderProg;

    typedef struct {
        SpriteQuadShaderProg spriteQuad;
        CharQuadShaderProg charQuad;
    } ShaderProgs;

    void load_shader_progs(ShaderProgs* const progs);
    void unload_shader_progs(ShaderProgs* const progs);
}
