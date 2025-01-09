#pragma once

#include <glad/glad.h>
#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    constexpr int gk_renderBatchSlotLimit = 4096;
    constexpr int gk_renderBatchSlotVertCnt = gk_texturedQuadShaderProgVertCnt * 4;

    constexpr int gk_texUnitLimit = 16;

    constexpr int gk_uniformNameLenLimit = 63;

    enum class ShaderUniformValType {
        Float,
        Int,
        Vec2D,
        Vec3D,
        Vec4D,
        Matrix4x4
    };

    union ShaderUniformVal {
        float floatVal;
        int intVal;
        Vec2D vec2DVal;
        Vec3D vec3DVal;
        Vec4D vec4DVal;
        Matrix4x4 mat4x4Val;
    };

    struct RenderSurface {
        GLuint framebufferGLID;
        GLuint framebufferTexGLID;
    };

    struct RenderBatchPermData {
        GLuint vertArrayGLID;
        GLuint vertBufGLID;
        GLuint elemBufGLID;
    };

    struct RenderBatchTransientData {
        float verts[gk_renderBatchSlotVertCnt * gk_renderBatchSlotLimit];

        int slotsUsedCnt;

        GLuint texUnitTexGLIDs[gk_texUnitLimit];
        int texUnitsInUseCnt;
    };

    enum class RenderInstrType {
        Clear,
        SetViewMatrix,
        DrawBatch,
        SetSurface,
        UnsetSurface,
        SetSurfaceShaderProg,
        SetSurfaceShaderProgUniform,
        DrawSurface
    };

    struct RenderInstrData_Clear {
        Vec4D color;
    };

    struct RenderInstrData_SetViewMatrix {
        Matrix4x4 mat;
    };

    struct RenderInstrData_DrawBatch {
        int index;
    };

    struct RenderInstrData_SetSurface {
        int index;
    };

    struct RenderInstrData_SetSurfaceShaderProg {
        int progIndex;
    };

    struct RenderInstrData_SetSurfaceShaderProgUniform {
        char name[gk_uniformNameLenLimit + 1];

        ShaderUniformVal val;
        ShaderUniformValType valType;
    };

    struct RenderInstrData_DrawSurface {
        int index;
    };

    union RenderInstrData {
        RenderInstrData_Clear clearData;
        RenderInstrData_SetViewMatrix setViewMatrixData;
        RenderInstrData_DrawBatch drawBatchData;
        RenderInstrData_SetSurface setSurfaceData;
        RenderInstrData_SetSurfaceShaderProg setSurfaceShaderProgData;
        RenderInstrData_SetSurfaceShaderProgUniform setSurfaceShaderProgUniformData;
        RenderInstrData_DrawSurface drawSurfaceData;
    };

    struct RenderInstr {
        RenderInstrType type;
        RenderInstrData data;
    };

    enum StrHorAlign {
        StrHorAlign_Left,
        StrHorAlign_Center,
        StrHorAlign_Right
    };

    enum StrVerAlign {
        StrVerAlign_Top,
        StrVerAlign_Center,
        StrVerAlign_Bottom
    };

    class Renderer {
    public:
        bool init(MemArena* const memArena, const int surfCnt, const int batchCnt);
        void clean();

        bool render(const InternalShaderProgs& internalShaderProgs, MemArena* const scratchSpace);

        bool resize_surfaces(const Vec2DI size);

        // The functions below do not actually perform the operations immediately.
        // They add appropriate instructions to the instruction list, which are executed during the game render phase (distinct from tick).
        void begin_draw();
        void end_draw();
        void clear(const Vec4D& color = {});
        void set_view_matrix(const Matrix4x4& mat);
        void draw_texture(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f);
        void draw_str(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign = zf4::StrHorAlign::StrHorAlign_Center, const StrVerAlign verAlign = zf4::StrVerAlign::StrVerAlign_Center);
        void set_surface(const int index);
        void unset_surface();
        void set_surface_shader_prog(const int progIndex);
        void set_surface_shader_prog_uniform(const char* const name, const ShaderUniformVal val, const ShaderUniformValType valType); // NOTE: Could use templates here (as part of the public interface only) for extra safety.
        void draw_surface(const int index);

    private:
        bool m_initialized;
        bool m_drawActive;

        Array<RenderSurface> m_surfs;
        GLuint m_surfVertArrayGLID;
        GLuint m_surfVertBufGLID;
        GLuint m_surfElemBufGLID;
        Stack<int> m_surfIndexes;

        int m_batchCnt;
        RenderBatchPermData* m_batchPermDatas; // Persists for the lifetime of the renderer.
        RenderBatchTransientData* m_batchTransDatas; // Cleared when a writeup begins.
        int m_batchWriteIndex; // Index of the batch we are currently writing to.

        int m_texUnits[gk_texUnitLimit]; // TODO: Rename.

        List<RenderInstr> m_renderInstrs;

        static int add_tex_unit_to_batch(RenderBatchTransientData* const batchTransData, const GLuint glID);

        void write(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha);
    };
}
