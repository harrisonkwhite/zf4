#include <zf4_audio.h>

#include <algorithm>
#include <cstdlib>
#include <AL/alc.h>
#include <AL/alext.h>

namespace zf4 {
    static ALCdevice* i_alDevice;
    static ALCcontext* i_alContext;

    static void release_sound_src_by_index(SoundSrcManager* const manager, const int index) {
        assert(index >= 0 && index < gk_soundSrcLimit);
        assert(manager->alIDs[index]);

        alDeleteSources(1, &manager->alIDs[index]);
        manager->alIDs[index] = 0;
    }

    static bool load_music_buf_data(MusicSrc* const src, const ALuint bufALID) {
        assert(bufALID);

        const auto buf = alloc<Byte>(gk_musicBufSize);

        if (!buf) {
            return false;
        }

        const AudioInfo& musicInfo = AssetManager::get_music_info(src->musicIndex);

        const long long totalBytesToRead = sizeof(AudioSample) * musicInfo.sampleCntPerChannel * musicInfo.channelCnt;
        const int bytesToRead = std::min(gk_musicBufSize, totalBytesToRead - src->fsBytesRead);
        const int bytesRead = read_from_fs<Byte>(buf, src->fs, bytesToRead);

        if (bytesRead < bytesToRead) {
            if (ferror(src->fs)) {
                free(buf);
                return false;
            }
        }

        src->fsBytesRead += bytesToRead;

        if (src->fsBytesRead == totalBytesToRead) {
            src->fsBytesRead = 0;
            fseek(src->fs, AssetManager::get_music_sample_data_file_pos(src->musicIndex), SEEK_SET);
        }

        const ALenum format = musicInfo.channelCnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
        alBufferData(bufALID, format, buf, bytesToRead, musicInfo.sampleRate);

        free(buf);

        return true;
    }

    static void clean_active_music_src(MusicSrcManager* const manager, const int index) {
        assert(is_bit_active(manager->activityBitset, index));

        alSourceStop(manager->srcs[index].alID);
        alSourcei(manager->srcs[index].alID, AL_BUFFER, 0);

        alDeleteBuffers(gk_musicBufCnt, manager->srcs[index].bufALIDs);
        alDeleteSources(1, &manager->srcs[index].alID);

        fclose(manager->srcs[index].fs);

        zero_out(&manager->srcs[index]);
    }

    bool init_audio_system() {
        assert(!i_alDevice && !i_alContext);

        // Open a playback device for OpenAL.
        i_alDevice = alcOpenDevice(nullptr);

        if (!i_alDevice) {
            log_error("Failed to open a device for OpenAL!");
            return false;
        }

        // Create an OpenAL context.
        i_alContext = alcCreateContext(i_alDevice, nullptr);

        if (!i_alContext) {
            log_error("Failed to create an OpenAL context!");
            return false;
        }

        alcMakeContextCurrent(i_alContext);

        // Verify necessary extensions are supported.
        if (!alIsExtensionPresent("AL_EXT_FLOAT32")) {
            log_error("AL_EXT_FLOAT32 is not supported by your OpenAL implementation.");
            return false;
        }

        return true;
    }

    void clean_audio_system() {
        if (i_alContext) {
            alcDestroyContext(i_alContext);
            i_alContext = nullptr;
        }

        if (i_alDevice) {
            alcCloseDevice(i_alDevice);
            i_alDevice = nullptr;
        }
    }

    void clean_sound_srcs(SoundSrcManager* const manager) {
        for (int i = 0; i < gk_soundSrcLimit; ++i) {
            if (manager->alIDs[i]) {
                alDeleteSources(1, &manager->alIDs[i]);
            }
        }

        zero_out(manager);
    }

    void handle_auto_release_sound_srcs(SoundSrcManager* const manager) {
        for (int i = 0; i < gk_soundSrcLimit; ++i) {
            if (!is_bit_active(manager->autoReleaseBitset, i)) {
                continue;
            }

            ALint srcState;
            alGetSourcei(manager->alIDs[i], AL_SOURCE_STATE, &srcState);

            if (srcState == AL_STOPPED) {
                release_sound_src_by_index(manager, i);
                deactivate_bit(manager->autoReleaseBitset, i);
            }
        }
    }

    AudioSrcID add_sound_src(SoundSrcManager* const manager, const int sndIndex) {
        for (int i = 0; i < gk_soundSrcLimit; ++i) {
            if (!manager->alIDs[i]) {
                alGenSources(1, &manager->alIDs[i]);
                alSourcei(manager->alIDs[i], AL_BUFFER, AssetManager::get_sound_buf_al_id(sndIndex));

                ++manager->versions[i];

                AudioSrcID srcID = {.index = i, .version = manager->versions[i]};
                return srcID;
            }
        }

        assert(false);

        AudioSrcID emptyID = {};
        return emptyID;
    }

    void remove_sound_src(SoundSrcManager* const manager, const AudioSrcID srcID) {
        assert(srcID.index >= 0 && srcID.index < gk_soundSrcLimit);
        assert(manager->versions[srcID.index] == srcID.version);
        assert(manager->alIDs[srcID.index]);

        release_sound_src_by_index(manager, srcID.index);
    }

    void play_sound_src(const SoundSrcManager* const manager, const AudioSrcID srcID, const float gain, const float pitch) {
        assert(srcID.index >= 0 && srcID.index < gk_soundSrcLimit);
        assert(manager->versions[srcID.index] == srcID.version);
        assert(manager->alIDs[srcID.index]);

        alSourceRewind(manager->alIDs[srcID.index]); // Restart if already playing.
        alSourcef(manager->alIDs[srcID.index], AL_GAIN, gain);
        alSourcef(manager->alIDs[srcID.index], AL_PITCH, pitch);
        alSourcePlay(manager->alIDs[srcID.index]);
    }

    void add_and_play_sound_src(SoundSrcManager* const manager, const int sndIndex, const float gain, const float pitch) {
        const AudioSrcID srcID = add_sound_src(manager, sndIndex);
        play_sound_src(manager, srcID, gain, pitch);
        activate_bit(manager->autoReleaseBitset, srcID.index); // No reference to this source is returned, so it needs to be automatically released once it is detected as finished.
    }

    void clean_music_srcs(MusicSrcManager* const manager) {
        for (int i = 0; i < gk_musicSrcLimit; ++i) {
            if (is_bit_active(manager->activityBitset, i)) {
                clean_active_music_src(manager, i);
            }
        }

        zero_out(manager);
    }

    bool refresh_music_src_bufs(MusicSrcManager* const manager) {
        for (int i = 0; i < gk_musicSrcLimit; ++i) {
            if (!is_bit_active(manager->activityBitset, i)) {
                continue;
            }

            MusicSrc* const src = &manager->srcs[i];

            // Retrieve all processed buffers, fill them with new data and queue them again.
            int processedBufCnt;
            alGetSourcei(src->alID, AL_BUFFERS_PROCESSED, &processedBufCnt);

            while (processedBufCnt > 0) {
                ALuint bufALID;
                alSourceUnqueueBuffers(src->alID, 1, &bufALID);

                if (!load_music_buf_data(src, bufALID)) {
                    return false;
                }

                alSourceQueueBuffers(src->alID, 1, &bufALID);

                processedBufCnt--;

                log("Refreshed source buffer with OpenAL ID %d for music source %d.", bufALID, i);
            }
        }

        return true;
    }

    AudioSrcID add_music_src(MusicSrcManager* const manager, const int musicIndex) {
        const int srcIndex = get_first_inactive_bit_index(manager->activityBitset, bits_to_bytes(gk_musicSrcLimit));
        assert(srcIndex != -1);

        MusicSrc* const src = &manager->srcs[srcIndex];
        src->musicIndex = musicIndex;

        alGenSources(1, &src->alID);
        alGenBuffers(gk_musicBufCnt, src->bufALIDs);

        activate_bit(manager->activityBitset, srcIndex);
        ++manager->versions[srcIndex];

        AudioSrcID id = {srcIndex, manager->versions[srcIndex]};
        return id;
    }

    void remove_music_src(MusicSrcManager* const manager, const AudioSrcID id) {
        assert(id.index >= 0 && id.index < gk_musicSrcLimit);
        assert(manager->versions[id.index] == id.version);
        assert(is_bit_active(manager->activityBitset, id.index));

        clean_active_music_src(manager, id.index);
        deactivate_bit(manager->activityBitset, id.index);
    }

    bool play_music_src(MusicSrcManager* const manager, const AudioSrcID id, const float gain) {
        assert(id.index >= 0 && id.index < gk_musicSrcLimit);
        assert(manager->versions[id.index] == id.version);
        assert(is_bit_active(manager->activityBitset, id.index));

        MusicSrc* const src = &manager->srcs[id.index];

        src->fs = fopen(gk_assetsFileName, "rb");

        if (!src->fs) {
            return false;
        }

        fseek(src->fs, AssetManager::get_music_sample_data_file_pos(src->musicIndex), SEEK_SET);

        for (int i = 0; i < gk_musicBufCnt; ++i) {
            if (!load_music_buf_data(src, src->bufALIDs[i])) {
                return false;
            }
        }

        alSourceQueueBuffers(src->alID, gk_musicBufCnt, src->bufALIDs);

        alSourcef(src->alID, AL_GAIN, gain);
        alSourcePlay(src->alID);

        return true;
    }
}
