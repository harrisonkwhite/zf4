#include "zf4_audio.h"

#include <AL/alext.h>
#include <dr_wav.h>
#include <zf4_io.h>

namespace zf4 {
    static s_array<const a_al_id> LoadSounds(const s_array<const char* const> audio_file_paths, s_mem_arena& mem_arena) {
        assert(audio_file_paths.IsInitialized());
        assert(mem_arena.IsInitialized());

        const int mem_arena_init_offs = mem_arena.Offset();
        const auto buf_al_ids = mem_arena.PushArray<a_al_id>(audio_file_paths.Len());

        if (buf_al_ids.IsInitialized()) {
            alGenBuffers(buf_al_ids.Len(), buf_al_ids.Raw());

            bool err = false;

            for (int i = 0; i < audio_file_paths.Len(); ++i) {
                unsigned int channels;
                unsigned int sample_rate;
                drwav_uint64 frame_cnt;

                const auto samples = drwav_open_file_and_read_pcm_frames_f32(audio_file_paths[i], &channels, &sample_rate, &frame_cnt, nullptr);

                if (!samples) {
                    err = true;
                    break;
                }

                const ALenum format = channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
                alBufferData(buf_al_ids[i], format, samples, sizeof(*samples) * channels * frame_cnt, sample_rate);

                drwav_free(samples, nullptr);
            }

            if (err) {
                alDeleteBuffers(buf_al_ids.Len(), buf_al_ids.Raw());
                mem_arena.Rewind(mem_arena_init_offs);
                return {};
            }
        }

        return buf_al_ids;
    }

    bool s_audio_system::Init(const s_array<const char* const> audio_file_paths, s_mem_arena& mem_arena) {
        assert(!m_initialized);
        assert(audio_file_paths.IsInitialized()); // TEMP?
        assert(mem_arena.IsInitialized());

        // Open a playback device for OpenAL.
        m_al_device = alcOpenDevice(nullptr);

        if (!m_al_device) {
            LogError("Failed to open a device for OpenAL!");
            Clean();
            return false;
        }

        // Create an OpenAL context.
        m_al_context = alcCreateContext(m_al_device, nullptr);

        if (!m_al_context) {
            LogError("Failed to create an OpenAL context!");
            Clean();
            return false;
        }

        alcMakeContextCurrent(m_al_context);

        // Verify necessary extensions are supported.
        if (!alIsExtensionPresent("AL_EXT_FLOAT32")) {
            LogError("AL_EXT_FLOAT32 is not supported by your OpenAL implementation.");
            Clean();
            return false;
        }

        // Load sounds.
        m_snd_buf_al_ids = LoadSounds(audio_file_paths, mem_arena);

        if (!m_snd_buf_al_ids.IsInitialized()) {
            Clean();
            return false;
        }

        m_initialized = true;

        return true;
    }

    void s_audio_system::Clean() {
        if (m_snd_buf_al_ids.IsInitialized()) {
            alDeleteBuffers(m_snd_buf_al_ids.Len(), m_snd_buf_al_ids.Raw());
        }

        if (m_al_context) {
            alcDestroyContext(m_al_context);
        }

        if (m_al_device) {
            alcCloseDevice(m_al_device);
        }

        *this = {};
    }
}
