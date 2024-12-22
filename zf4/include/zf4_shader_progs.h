#ifndef ZF4_SHADER_PROGS_H
#define ZF4_SHADER_PROGS_H

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <glad/glad.h>

#define ZF4_SPRITE_QUAD_SHADER_PROG_VERT_CNT 11
#define ZF4_CHAR_QUAD_SHADER_PROG_VERT_CNT 4

typedef struct {
    GLuint glID;
    int projUniLoc;
    int viewUniLoc;
    int texturesUniLoc;
} ZF4SpriteQuadShaderProg;

typedef struct {
    GLuint glID;
    int projUniLoc;
    int viewUniLoc;
    int posUniLoc;
    int rotUniLoc;
    int blendUniLoc;
} ZF4CharQuadShaderProg;

typedef struct {
    ZF4SpriteQuadShaderProg spriteQuad;
    ZF4CharQuadShaderProg charQuad;
} ZF4ShaderProgs;

void zf4_load_shader_progs(ZF4ShaderProgs* const progs);
void zf4_unload_shader_progs(ZF4ShaderProgs* const progs);

#endif
