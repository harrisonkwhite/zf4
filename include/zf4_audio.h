#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <zf4_mem.h>

namespace zf4 {
    using a_al_id = ALuint;

    struct s_sound_src_id {
        int index = 0;
        int version = 0;
    };

    struct s_audio_system {
    public:
        bool Init(const s_array<const char* const> audio_file_paths, s_mem_arena& mem_arena);
        void Clean();

#if 0
        s_sound_src_id AddSoundSrc(const int snd_index);
        void RemoveSoundSrc(const s_sound_src_id id);
        void PlaySoundSrc(const s_sound_src_id id, const float gain, const float pitch);
#endif

    private:
        bool m_initialized = false;

        ALCdevice* m_al_device = nullptr;
        ALCcontext* m_al_context = nullptr;

        s_array<const a_al_id> m_snd_buf_al_ids;

        s_static_array<a_al_id, 32> m_snd_src_al_ids;
    };
}
