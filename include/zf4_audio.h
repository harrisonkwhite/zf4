#pragma once

#include <miniaudio.h>
#include <zf4_mem.h>

namespace zf4 {
    class s_audio_system {
    public:
        bool Init(const s_array<const char* const> snd_file_paths, s_mem_arena& mem_arena);
        void Clean();
        void PlaySnd(const int index, const float vol = 1.0f, const float pitch = 1.0f);

    private:
        bool m_initialized = false;
        ma_engine m_engine = {};
        s_array<ma_sound> m_snds;
    };
}
