#pragma once

#include <miniaudio.h>
#include <zf4_mem.h>

namespace zf4 {
    using a_snd_file_path_loader = const char* (*)(const int index);

    class s_audio_system {
    public:
        bool Init(const int snd_cnt, const a_snd_file_path_loader snd_file_path_loader, s_mem_arena& mem_arena);
        void Clean();
        void PlaySnd(const int index, const float vol = 1.0f, const float pitch = 1.0f);

    private:
        bool m_initialized = false;
        ma_engine m_engine = {};
        s_array<ma_sound> m_snds;
    };
}
