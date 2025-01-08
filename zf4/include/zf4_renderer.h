#pragma once

#include <glad/glad.h>
#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    constexpr int gk_renderSurfaceLimit = 16; // NOTE: Kind of arbitrary?
    constexpr int gk_renderBatchLimit = 128;
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
        bool init(MemArena* const memArena);
        void clean();

        void render(const Vec3D& bgColor, const InternalShaderProgs& internalShaderProgs);

        bool add_surface();

        void begin_writeup();
        void end_writeup();
        void clear(const Vec4D& color = {});
        void set_view_matrix(const Matrix4x4& mat);
        void write_texture(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f);
        void write_str(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign = zf4::StrHorAlign::STR_HOR_ALIGN_CENTER, const StrVerAlign verAlign = zf4::StrVerAlign::STR_VER_ALIGN_CENTER);
        void set_surface(const int index);
        void unset_surface();
        void draw_surface(const int index, const int shaderProgIndex);

    private:
        bool m_initialized;
        bool m_inWriteup;

        List<RenderSurface> m_surfs;
        GLuint m_surfVertArrayGLID;
        GLuint m_surfVertBufGLID;
        GLuint m_surfElemBufGLID;

        RenderBatchPermData* m_batchPermDatas; // Persists for the lifetime of the renderer.
        RenderBatchTransientData* m_batchTransDatas; // Cleared when a writeup begins.
        int m_batchWriteIndex; // Index of the batch we are currently writing to.

        int m_texUnits[gk_texUnitLimit]; // TODO: Rename.

        List<RenderInstr> m_renderInstrs;

        static int add_tex_unit_to_batch(RenderBatchTransientData* const batchTransData, const GLuint glID);
        void write(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha);
    };
}
