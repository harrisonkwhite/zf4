#pragma once

#include <cstdio>
#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

namespace zf4 {
    constexpr int gk_texturedQuadShaderProgVertCnt = 14;

    struct Textures {
        SafePtr<GLuint> glIDs;
        SafePtr<Vec2DI> sizes;
        SafePtr<SafePtr<unsigned char>> pxDatas;
        int cnt;
    };

    struct Fonts {
        SafePtr<FontArrangementInfo> arrangementInfos;

        SafePtr<GLuint> texGLIDs;
        SafePtr<Vec2DI> texSizes;

        int cnt;
    };

    struct ShaderProgs {
        SafePtr<GLuint> glIDs;
        int cnt;
    };

    struct Sounds {
        SafePtr<GLuint> bufALIDs;
        int cnt;
    };

    struct Music {
        SafePtr<AudioInfo> infos;
        SafePtr<int> sampleDataFilePositions;
        int cnt;
    };

    class Assets {
    public:
        bool load(MemArena* const memArena);
        void clean();

        GLuint get_tex_gl_id(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_textures.cnt);
            return m_textures.glIDs[index];
        }

        Vec2DI get_tex_size(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_textures.cnt);
            return m_textures.sizes[index];
        }

        const SafePtr<unsigned char> get_tex_px_data(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_textures.cnt);
            return m_textures.pxDatas[index];
        }

        const FontArrangementInfo& get_font_arrangement_info(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_fonts.cnt);
            return m_fonts.arrangementInfos[index];
        }

        GLuint get_font_tex_gl_id(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_fonts.cnt);
            return m_fonts.texGLIDs[index];
        }

        Vec2DI get_font_tex_size(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_fonts.cnt);
            return m_fonts.texSizes[index];
        }

        GLuint get_shader_prog_gl_id(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_shaderProgs.cnt);
            return m_shaderProgs.glIDs[index];
        }

        GLuint get_sound_buf_al_id(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_sounds.cnt);
            return m_sounds.bufALIDs[index];
        }

        const AudioInfo& get_music_info(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_music.cnt);
            return m_music.infos[index];
        }

        int get_music_sample_data_file_pos(const int index) const {
            assert(m_loaded);
            assert(index >= 0 && index < m_music.cnt);
            return m_music.sampleDataFilePositions[index];
        }

    private:
        bool m_loaded;

        Textures m_textures;
        Fonts m_fonts;
        ShaderProgs m_shaderProgs;
        Sounds m_sounds;
        Music m_music;
    };

    struct TexturedQuadShaderProg {
        GLuint glID;
        int projUniLoc;
        int viewUniLoc;
        int texturesUniLoc;
    };

    struct InternalShaderProgs {
        TexturedQuadShaderProg texturedQuad;
    };

    bool load_textures_from_fs(Textures* const textures, MemArena* const memArena, FILE* const fs);
    bool load_fonts_from_fs(Fonts* const fonts, MemArena* const memArena, FILE* const fs);
    bool load_shader_progs_from_fs(ShaderProgs* const progs, MemArena* const memArena, FILE* const fs);
    bool load_sounds_from_fs(Sounds* const snds, MemArena* const memArena, FILE* const fs);
    bool load_music_from_fs(Music* const music, MemArena* const memArena, FILE* const fs);

    InternalShaderProgs load_internal_shader_progs();
    void unload_internal_shader_progs(InternalShaderProgs* const progs);
}
