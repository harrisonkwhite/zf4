#pragma once

#include <type_traits>
#include <glad/glad.h>
#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    constexpr int gk_renderBatchSlotLimit = 4096;
    constexpr int gk_renderBatchSlotVertCnt = gk_texturedQuadShaderProgVertCnt * 4;

    constexpr int gk_texUnitLimit = 16;

    constexpr int gk_renderInstrLimit = 1024;

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
        SetDrawSurfaceShaderProg,
        SetDrawSurfaceShaderUniform,
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

    struct RenderInstrData_SetDrawSurfaceShaderProg {
        int progIndex;
    };

    struct RenderInstrData_SetDrawSurfaceShaderUniform {
        char name[gk_uniformNameLenLimit + 1];

        ShaderUniformVal val;
        ShaderUniformValType valType;
    };

    struct RenderInstrData_DrawSurface {
        int index;
    };

    union RenderInstrData {
        RenderInstrData_Clear clear;
        RenderInstrData_SetViewMatrix setViewMatrix;
        RenderInstrData_DrawBatch drawBatch;
        RenderInstrData_SetSurface setSurface;
        RenderInstrData_SetDrawSurfaceShaderProg setDrawSurfaceShaderProg;
        RenderInstrData_SetDrawSurfaceShaderUniform setDrawSurfaceShaderUniform;
        RenderInstrData_DrawSurface drawSurface;
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

        bool resize_surfaces();

        void begin_submission_phase();
        void end_submission_phase();
        void submit_texture_to_batch(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f);
        void submit_str_to_batch(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign = zf4::StrHorAlign::StrHorAlign_Center, const StrVerAlign verAlign = zf4::StrVerAlign::StrVerAlign_Center);

        void submit_clear_instr(const Vec4D color = {}) {
            submit_instr(RenderInstrType::Clear, {.clear = {.color = color}});
        }

        void submit_set_view_matrix_instr(const Matrix4x4& mat) {
            submit_instr(RenderInstrType::SetViewMatrix, {.setViewMatrix = {.mat = mat}});
        }

        void submit_draw_batch_instr(const int index) {
            submit_instr(RenderInstrType::DrawBatch, {.drawBatch = {.index = index}});
        }

        void submit_set_surface_instr(const int index) {
            submit_instr(RenderInstrType::SetSurface, {.setSurface = {.index = index}});
        }

        void submit_unset_surface_instr() {
            submit_instr(RenderInstrType::UnsetSurface, {});
        }

        void submit_set_draw_surface_shader_prog_instr(const int progIndex) {
            submit_instr(RenderInstrType::SetDrawSurfaceShaderProg, {.setDrawSurfaceShaderProg = {.progIndex = progIndex}});
        }

        void submit_set_draw_surface_shader_uniform_instr(const char* const name, const ShaderUniformVal& val, const ShaderUniformValType valType) {
            RenderInstrData_SetDrawSurfaceShaderUniform data = {
                .val = val,
                .valType = valType
            };

            std::strncpy(data.name, name, gk_uniformNameLenLimit);
            data.name[gk_uniformNameLenLimit] = '\0';

            submit_instr(RenderInstrType::SetDrawSurfaceShaderUniform, {.setDrawSurfaceShaderUniform = data});
        }

        template<typename T>
        void submit_set_draw_surface_shader_uniform_instr(const char* const name, const T& val) {
            static_assert(
                std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, Vec2D> ||
                std::is_same_v<T, Vec3D> || std::is_same_v<T, Vec4D> || std::is_same_v<T, Matrix4x4>,
                "Invalid type for surface shader uniform value!"
            );

            if constexpr (std::is_same_v<T, float>) {
                submit_set_draw_surface_shader_uniform_instr(name, {.floatVal = val}, ShaderUniformValType::Float);
            } else if constexpr (std::is_same_v<T, int>) {
                submit_set_draw_surface_shader_uniform_instr(name, {.intVal = val}, ShaderUniformValType::Int);
            } else if constexpr (std::is_same_v<T, Vec2D>) {
                submit_set_draw_surface_shader_uniform_instr(name, {.vec2DVal = val}, ShaderUniformValType::Vec2D);
            } else if constexpr (std::is_same_v<T, Vec3D>) {
                submit_set_draw_surface_shader_uniform_instr(name, {.vec3DVal = val}, ShaderUniformValType::Vec3D);
            } else if constexpr (std::is_same_v<T, Vec4D>) {
                submit_set_draw_surface_shader_uniform_instr(name, {.vec4DVal = val}, ShaderUniformValType::Vec4D);
            } else if constexpr (std::is_same_v<T, Matrix4x4>) {
                submit_set_draw_surface_shader_uniform_instr(name, {.mat4x4Val = val}, ShaderUniformValType::Matrix4x4);
            }
        }

        void submit_draw_surface_instr(const int index) {
            submit_instr(RenderInstrType::DrawSurface, {.drawSurface = {.index = index}});
        }

    private:
        bool m_initialized;
        bool m_inSubmissionPhase;

        Array<RenderSurface> m_surfs;
        GLuint m_surfVertArrayGLID;
        GLuint m_surfVertBufGLID;
        GLuint m_surfElemBufGLID;
        Stack<int> m_surfIndexes;

        int m_batchCnt;
        RenderBatchPermData* m_batchPermDatas; // Persists for the lifetime of the renderer.
        RenderBatchTransientData* m_batchTransDatas; // Cleared when submission phase begins.
        int m_batchSubmitIndex; // Index of the batch we are currently submitting to.

        int m_texUnits[gk_texUnitLimit]; // TODO: Rename.

        List<RenderInstr> m_renderInstrs;

        static int add_tex_unit_to_batch(RenderBatchTransientData* const batchTransData, const GLuint glID);
        void move_to_next_batch();
        void submit_instr(const RenderInstrType type, const RenderInstrData data);
        void submit_to_batch(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha);
    };
}
