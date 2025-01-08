#pragma once

#include <zf4c.h>
#include <zf4_shader_progs.h>

namespace zf4 {
    constexpr int gk_texUnitLimit = 16;

    constexpr int gk_renderBatchLimit = 128;
    constexpr int gk_renderBatchSlotLimit = 4096;
    constexpr int gk_renderBatchSlotVertCnt = gk_texturedQuadShaderProgVertCnt * 4;

    constexpr int gk_viewMatrixLimit = 16;

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

    struct RendererViewMatrixInfo {
        Matrix4x4 mat;
        int beginBatchIndex;
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

        void render(const Vec3D& bgColor, const ShaderProgs* const shaderProgs);

        void begin_writeup();
        void end_writeup();
        void write_texture(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f);
        void write_str(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign = zf4::StrHorAlign::STR_HOR_ALIGN_CENTER, const StrVerAlign verAlign = zf4::StrVerAlign::STR_VER_ALIGN_CENTER);
        void update_writeup_view_matrix(const Matrix4x4& mat);

    private:
        bool m_initialized;
        bool m_inWriteup;

        int m_texUnits[gk_texUnitLimit]; // TODO: Rename.

        RenderBatchPermData* m_batchPermDatas; // Persists for the lifetime of the renderer.
        RenderBatchTransientData* m_batchTransDatas; // Cleared when a writeup begins.
        int m_batchWriteIndex; // Index of the batch we are currently writing to.

        List<RendererViewMatrixInfo> m_viewMatrixInfos;

        static int add_tex_unit_to_batch(RenderBatchTransientData* const batchTransData, const GLuint glID);

        void write(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha);
    };
}
