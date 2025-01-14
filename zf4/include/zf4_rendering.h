#pragma once

#include <type_traits>
#include <glad/glad.h>
#include <zf4c.h>
#include <zf4_assets.h>
#include <zf4_sprites.h>

namespace zf4 {
    constexpr int gk_uniformNameLenLimit = 63;

    constexpr int gk_renderBatchSlotLimit = 4096;
    constexpr int gk_renderBatchSlotVertCnt = gk_texturedQuadShaderProgVertCnt * 4;

    constexpr int gk_texUnitLimit = 16;

    constexpr int gk_renderSurfaceLimit = 256;

    constexpr int gk_renderInstrLimit = 1024;

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
        SetSurfaceShaderUniform,
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

    struct RenderInstrData_SetSurfaceShaderUniform {
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
        RenderInstrData_SetSurfaceShaderProg setDrawSurfaceShaderProg;
        RenderInstrData_SetSurfaceShaderUniform setDrawSurfaceShaderUniform;
        RenderInstrData_DrawSurface drawSurface;
    };

    struct RenderInstr {
        RenderInstrType type;
        RenderInstrData data;
    };

    enum class RendererState {
        Uninitialized,
        Initialized,
        Submitting
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

    struct StrRenderInfo {
        SafePtr<Vec2DI> charDrawPositions;
        int charCnt;

        SafePtr<int> lineWidths;
        int lineCnt;

        int height;
    };

    using RenderSurfaceID = int; // NOTE: Maybe remove?

    class Renderer {
    public:
        bool init(MemArena* const memArena, const int batchLimit = 64, const int batchLifeMax = 600); // TODO: Have the defaults be specified maybe in user game information defaults?
        void clean();

        RenderSurfaceID add_surface();
        void remove_surface(const RenderSurfaceID surfID);
        bool resize_surfaces();

        void begin_submission_phase();
        void end_submission_phase();
        bool submit_texture(const int texIndex, const Assets& assets, const Vec2D pos, const RectI& srcRect, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f);
        bool submit_str(const char* const str, const int fontIndex, const Assets& assets, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign = StrHorAlign_Center, const StrVerAlign verAlign = StrVerAlign_Center);

        bool render(const InternalShaderProgs& internalShaderProgs, const Assets& assets, MemArena* const scratchSpace);

        bool submit_sprite(const Sprite& sprite, const int frameIndex, const Vec2D pos, const Assets& assets, const Vec2D origin = {0.5f, 0.5f}, const float rot = 0.0f, const Vec2D scale = {1.0f, 1.0f}, const float alpha = 1.0f) {
            return submit_texture(sprite.texIndex, assets, pos, *sprite.frames.get(frameIndex), origin, rot, scale, alpha);
        }

        // TODO: Change return values for these to allow for error handling. Only a subset needs to have them. Solve this somehow!
        void submit_clear_instr(const Vec4D color = {}) {
            submit_instr(RenderInstrType::Clear, {.clear = {.color = color}});
        }

        void submit_set_view_matrix_instr(const Matrix4x4& mat) {
            submit_instr(RenderInstrType::SetViewMatrix, {.setViewMatrix = {.mat = mat}});
        }

        void submit_set_surface_instr(const int index) {
            submit_instr(RenderInstrType::SetSurface, {.setSurface = {.index = index}});
        }

        void submit_unset_surface_instr() {
            submit_instr(RenderInstrType::UnsetSurface, {});
        }

        void submit_set_surface_shader_prog_instr(const int progIndex) {
            submit_instr(RenderInstrType::SetSurfaceShaderProg, {.setDrawSurfaceShaderProg = {.progIndex = progIndex}});
        }

        template<typename T>
        void submit_set_surface_shader_uniform_instr(const char* const name, const T& val) {
            if constexpr (std::is_same_v<T, float>) {
                submit_set_surface_shader_uniform_instr(name, {.floatVal = val}, ShaderUniformValType::Float);
            } else if constexpr (std::is_same_v<T, int>) {
                submit_set_surface_shader_uniform_instr(name, {.intVal = val}, ShaderUniformValType::Int);
            } else if constexpr (std::is_same_v<T, Vec2D>) {
                submit_set_surface_shader_uniform_instr(name, {.vec2DVal = val}, ShaderUniformValType::Vec2D);
            } else if constexpr (std::is_same_v<T, Vec3D>) {
                submit_set_surface_shader_uniform_instr(name, {.vec3DVal = val}, ShaderUniformValType::Vec3D);
            } else if constexpr (std::is_same_v<T, Vec4D>) {
                submit_set_surface_shader_uniform_instr(name, {.vec4DVal = val}, ShaderUniformValType::Vec4D);
            } else if constexpr (std::is_same_v<T, Matrix4x4>) {
                submit_set_surface_shader_uniform_instr(name, {.mat4x4Val = val}, ShaderUniformValType::Matrix4x4);
            } else {
                static_assert(false, "Unsupported type for shader uniform value!");
            }
        }

        void submit_draw_surface_instr(const int index) {
            submit_instr(RenderInstrType::DrawSurface, {.drawSurface = {.index = index}});
        }

    private:
        RendererState m_state;

        ActivityArray<RenderSurface> m_surfs;
        GLuint m_surfVertArrayGLID;
        GLuint m_surfVertBufGLID;
        GLuint m_surfElemBufGLID;

        int m_batchLimit;
        SafePtr<RenderBatchPermData> m_batchPermDatas; // Persists for the lifetime of the renderer.
        SafePtr<RenderBatchTransientData> m_batchTransDatas; // Cleared when submission phase begins.
        int m_batchSubmitIndex; // Index of the batch we are currently submitting to.
        SafePtr<int> m_batchLifes; // Reset to a maximum whenever the batch is written to, and decrements when not. Batch is deactivated (i.e. freed) once this reaches 0.
        int m_batchLifeMax;
        SafePtr<unsigned short> m_batchIndices; // Reused whenever a batch is generated.

        int m_texUnits[gk_texUnitLimit]; // TODO: Rename.

        Stack<RenderInstr> m_renderInstrs;

        bool submit_to_batch(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha);
        bool move_to_next_batch();
        bool submit_instr(const RenderInstrType type, const RenderInstrData data);

        void submit_draw_batch_instr(const int index) {
            assert(index >= 0 && index <= m_batchSubmitIndex);
            submit_instr(RenderInstrType::DrawBatch, {.drawBatch = {.index = index}});
        }

        void submit_set_surface_shader_uniform_instr(const char* const name, const ShaderUniformVal val, const ShaderUniformValType valType) {
            RenderInstrData_SetSurfaceShaderUniform instrData = {.val = val, .valType = valType};
            std::strncpy(instrData.name, name, sizeof(instrData.name));

            submit_instr(RenderInstrType::SetSurfaceShaderUniform, {.setDrawSurfaceShaderUniform = instrData});
        }
    };
}
