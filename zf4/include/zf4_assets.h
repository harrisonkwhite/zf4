#pragma once

#include <cstdio>
#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

namespace zf4 {
    constexpr int gk_texturedQuadShaderProgVertCnt = 11;

    struct Textures {
        GLuint* glIDs;
        Vec2DI* sizes;
        unsigned char** pxDatas;
        int cnt;
    };

    struct Fonts {
        FontArrangementInfo* arrangementInfos;

        GLuint* texGLIDs;
        Vec2DI* texSizes;

        int cnt;
    };

    struct ShaderProgs {
        GLuint* glIDs;
        int cnt;
    };

    struct Sounds {
        GLuint* bufALIDs;
        int cnt;
    };

    struct Music {
        AudioInfo* infos;
        int* sampleDataFilePositions;
        int cnt;
    };

    class AssetManager {
    public:
        AssetManager() = delete;

        static bool load(MemArena* const memArena);
        static void unload();

        static GLuint get_tex_gl_id(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_textures.cnt);
            return s_textures.glIDs[index];
        }

        static Vec2DI get_tex_size(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_textures.cnt);
            return s_textures.sizes[index];
        }

        static const unsigned char* get_tex_px_data(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_textures.cnt);
            return s_textures.pxDatas[index];
        }

        static const FontArrangementInfo& get_font_arrangement_info(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_fonts.cnt);
            return s_fonts.arrangementInfos[index];
        }

        static GLuint get_font_tex_gl_id(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_fonts.cnt);
            return s_fonts.texGLIDs[index];
        }

        static Vec2DI get_font_tex_size(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_fonts.cnt);
            return s_fonts.texSizes[index];
        }

        static GLuint get_shader_prog_gl_id(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_shaderProgs.cnt);
            return s_shaderProgs.glIDs[index];
        }

        static GLuint get_sound_buf_al_id(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_sounds.cnt);
            return s_sounds.bufALIDs[index];
        }

        static const AudioInfo& get_music_info(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_music.cnt);
            return s_music.infos[index];
        }

        static int get_music_sample_data_file_pos(const int index) {
            assert(s_loaded);
            assert(index >= 0 && index < s_music.cnt);
            return s_music.sampleDataFilePositions[index];
        }

    private:
        inline static bool s_loaded;

        inline static Textures s_textures;
        inline static Fonts s_fonts;
        inline static ShaderProgs s_shaderProgs;
        inline static Sounds s_sounds;
        inline static Music s_music;
    };

    struct TexturedQuadShaderProg {
        GLuint glID;
        int projUniLoc;
        int viewUniLoc;
        int texturesUniLoc;
    };

    struct TestShaderProg {
        GLuint glID;
    };

    struct InternalShaderProgs {
        TexturedQuadShaderProg texturedQuad;
        TestShaderProg test;
    };

    bool load_textures(Textures* const textures, MemArena* const memArena, FILE* const fs);
    bool load_fonts(Fonts* const fonts, MemArena* const memArena, FILE* const fs);
    bool load_shader_progs(ShaderProgs* const progs, MemArena* const memArena, FILE* const fs);
    bool load_sounds(Sounds* const snds, MemArena* const memArena, FILE* const fs);
    bool load_music(Music* const music, MemArena* const memArena, FILE* const fs);

    InternalShaderProgs load_internal_shader_progs();
    void unload_internal_shader_progs(InternalShaderProgs* const progs);
}
