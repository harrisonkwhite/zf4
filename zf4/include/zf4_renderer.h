#pragma once

#include <glad/glad.h>
#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    constexpr int gk_renderBatchSlotLimit = 4096;
    constexpr int gk_renderBatchSlotVertCnt = gk_texturedQuadShaderProgVertCnt * 4;

    constexpr int gk_texUnitLimit = 16;

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
        DrawSurface
    };

    struct ClearRenderInstrData {
        Vec4D color;
    };

    struct SetViewMatrixRenderInstrData {
        Matrix4x4 mat;
    };

    struct DrawBatchRenderInstrData {
        int index;
    };

    struct SetSurfaceRenderInstrData {
        int index;
    };

    struct DrawSurfaceRenderInstrData {
        int surfIndex;
        int shaderProgIndex;
    };

    union RenderInstrData {
        ClearRenderInstrData clearData;
        SetViewMatrixRenderInstrData setViewMatrixData;
        DrawBatchRenderInstrData drawBatchData;
        SetSurfaceRenderInstrData setSurfaceData;
        DrawSurfaceRenderInstrData drawSurfaceData;
    };

    struct RenderInstr {
        RenderInstrType type;
        RenderInstrData data;
    };

    enum StrHorAlign {
        STR_HOR_ALIGN_LEFT,
        STR_HOR_ALIGN_CENTER,
        STR_HOR_ALIGN_RIGHT
    };

    enum StrVerAlign {
        STR_VER_ALIGN_TOP,
        STR_VER_ALIGN_CENTER,
        STR_VER_ALIGN_BOTTOM
    };

    class Renderer {
    public:
        bool init(MemArena* const memArena, const int surfCnt, const int batchCnt);
        void clean();

        void render(const InternalShaderProgs& internalShaderProgs);

        // The functions below do not actually perform the operations immediately.
        // They add appropriate instructions to the instruction list, which are executed during the game render phase (distinct from tick).
        void begin_draw();
        void end_draw();
        void clear(const Vec4D& color = {});
        void set_view_matrix(const Matrix4x4& mat);
        void draw_texture(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f);
        void draw_str(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign = zf4::StrHorAlign::STR_HOR_ALIGN_CENTER, const StrVerAlign verAlign = zf4::StrVerAlign::STR_VER_ALIGN_CENTER);
        void set_surface(const int index);
        void unset_surface();
        void draw_surface(const int index, const int shaderProgIndex);

    private:
        bool m_initialized;
        bool m_drawActive;

        Array<RenderSurface> m_surfs;
        GLuint m_surfVertArrayGLID;
        GLuint m_surfVertBufGLID;
        GLuint m_surfElemBufGLID;

        int m_batchCnt;
        RenderBatchPermData* m_batchPermDatas; // Persists for the lifetime of the renderer.
        RenderBatchTransientData* m_batchTransDatas; // Cleared when a writeup begins.
        int m_batchWriteIndex; // Index of the batch we are currently writing to.

        int m_texUnits[gk_texUnitLimit]; // TODO: Rename.

        List<RenderInstr> m_renderInstrs;

        static bool init_surface(RenderSurface* const surf);
        static int add_tex_unit_to_batch(RenderBatchTransientData* const batchTransData, const GLuint glID);

        void write(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha);
    };
}
