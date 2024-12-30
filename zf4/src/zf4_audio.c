#include <zf4_audio.h>

#include <stdlib.h>
#include <AL/alc.h>
#include <AL/alext.h>

static ALCdevice* i_alDevice;
static ALCcontext* i_alContext;

static void release_sound_src_by_index(ZF4SoundSrcManager* const manager, const int index) {
    assert(index >= 0 && index < ZF4_SOUND_SRC_LIMIT);
    assert(manager->alIDs[index]);

    alDeleteSources(1, &manager->alIDs[index]);
    manager->alIDs[index] = 0;
}

static bool load_music_buf_data(ZF4MusicSrc* const src, const ALuint bufALID) {
    assert(bufALID);

    float* const buf = malloc(ZF4_MUSIC_BUF_SIZE);

    if (!buf) {
        return false;
    }

    const ZF4AudioInfo* const musicInfo = &zf4_get_music()->infos[src->musicIndex];

    const long long totalBytesToRead = (long long)(sizeof(ZF4AudioSample) * musicInfo->sampleCntPerChannel * musicInfo->channelCnt);
    const int bytesToRead = (int)ZF4_MIN(ZF4_MUSIC_BUF_SIZE, totalBytesToRead - src->fsBytesRead);
    const int bytesRead = (int)fread(buf, 1, bytesToRead, src->fs);

    if (bytesRead < bytesToRead) {
        if (ferror(src->fs)) {
            free(buf);
            return false;
        }
    }

    src->fsBytesRead += bytesToRead;

    if (src->fsBytesRead == totalBytesToRead) {
        src->fsBytesRead = 0;
        fseek(src->fs, zf4_get_music()->sampleDataFilePositions[src->musicIndex], SEEK_SET);
    }

    const ALenum format = musicInfo->channelCnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
    alBufferData(bufALID, format, buf, bytesToRead, musicInfo->sampleRate);

    free(buf);

    return true;
}

static void clean_active_music_src(ZF4MusicSrcManager* const manager, const int index) {
    assert(zf4_is_bit_active(manager->activityBitset, index));

    alSourceStop(manager->srcs[index].alID);
    alSourcei(manager->srcs[index].alID, AL_BUFFER, 0);

    alDeleteBuffers(ZF4_MUSIC_BUF_CNT, manager->srcs[index].bufALIDs);
    alDeleteSources(1, &manager->srcs[index].alID);

    fclose(manager->srcs[index].fs);

    memset(&manager->srcs[index], 0, sizeof(manager->srcs[index]));
}

bool zf4_init_audio_system() {
    assert(!i_alDevice && !i_alContext);

    // Open a playback device for OpenAL.
    i_alDevice = alcOpenDevice(NULL);

    if (!i_alDevice) {
        zf4_log_error("Failed to open a device for OpenAL!");
        return false;
    }

    // Create an OpenAL context.
    i_alContext = alcCreateContext(i_alDevice, NULL);

    if (!i_alContext) {
        zf4_log_error("Failed to create an OpenAL context!");
        return false;
    }

    alcMakeContextCurrent(i_alContext);

    // Verify necessary extensions are supported.
    if (!alIsExtensionPresent("AL_EXT_FLOAT32")) {
        zf4_log_error("AL_EXT_FLOAT32 is not supported by your OpenAL implementation.");
        return false;
    }

    return true;
}

void zf4_clean_audio_system() {
    if (i_alContext) {
        alcDestroyContext(i_alContext);
        i_alContext = NULL;
    }

    if (i_alDevice) {
        alcCloseDevice(i_alDevice);
        i_alDevice = NULL;
    }
}

void zf4_clean_sound_srcs(ZF4SoundSrcManager* const manager) {
    for (int i = 0; i < ZF4_SOUND_SRC_LIMIT; ++i) {
        if (manager->alIDs[i]) {
            alDeleteSources(1, &manager->alIDs[i]);
        }
    }

    memset(manager, 0, sizeof(*manager));
}

void zf4_handle_auto_release_sound_srcs(ZF4SoundSrcManager* const manager) {
    for (int i = 0; i < ZF4_SOUND_SRC_LIMIT; ++i) {
        if (!zf4_is_bit_active(manager->autoReleaseBitset, i)) {
            continue;
        }

        ALint srcState;
        alGetSourcei(manager->alIDs[i], AL_SOURCE_STATE, &srcState);

        if (srcState == AL_STOPPED) {
            release_sound_src_by_index(manager, i);
            zf4_deactivate_bit(manager->autoReleaseBitset, i);
        }
    }
}

ZF4AudioSrcID zf4_add_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex) {
    for (int i = 0; i < ZF4_SOUND_SRC_LIMIT; ++i) {
        if (!manager->alIDs[i]) {
            alGenSources(1, &manager->alIDs[i]);
            alSourcei(manager->alIDs[i], AL_BUFFER, zf4_get_sounds()->bufALIDs[sndIndex]);

            ++manager->versions[i];

            ZF4AudioSrcID srcID = {.index = i, .version = manager->versions[i]};
            return srcID;
        }
    }

    assert(false);

    ZF4AudioSrcID emptyID = {0};
    return emptyID;
}

void zf4_remove_sound_src(ZF4SoundSrcManager* const manager, const ZF4AudioSrcID srcID) {
    assert(srcID.index >= 0 && srcID.index < ZF4_SOUND_SRC_LIMIT);
    assert(manager->versions[srcID.index] == srcID.version);
    assert(manager->alIDs[srcID.index]);

    release_sound_src_by_index(manager, srcID.index);
}

void zf4_play_sound_src(const ZF4SoundSrcManager* const manager, const ZF4AudioSrcID srcID, const float gain, const float pitch) {
    assert(srcID.index >= 0 && srcID.index < ZF4_SOUND_SRC_LIMIT);
    assert(manager->versions[srcID.index] == srcID.version);
    assert(manager->alIDs[srcID.index]);

    alSourceRewind(manager->alIDs[srcID.index]); // Restart if already playing.
    alSourcef(manager->alIDs[srcID.index], AL_GAIN, gain);
    alSourcef(manager->alIDs[srcID.index], AL_PITCH, pitch);
    alSourcePlay(manager->alIDs[srcID.index]);
}

void zf4_add_and_play_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const float gain, const float pitch) {
    const ZF4AudioSrcID srcID = zf4_add_sound_src(manager, sndIndex);
    zf4_play_sound_src(manager, srcID, gain, pitch);
    zf4_activate_bit(manager->autoReleaseBitset, srcID.index); // No reference to this source is returned, so it needs to be automatically released once it is detected as finished.
}

void zf4_clean_music_srcs(ZF4MusicSrcManager* const manager) {
    for (int i = 0; i < ZF4_MUSIC_SRC_LIMIT; ++i) {
        if (zf4_is_bit_active(manager->activityBitset, i)) {
            clean_active_music_src(manager, i);
        }
    }

    memset(manager, 0, sizeof(*manager));
}

bool zf4_refresh_music_src_bufs(ZF4MusicSrcManager* const manager) {
    for (int i = 0; i < ZF4_MUSIC_SRC_LIMIT; ++i) {
        if (!zf4_is_bit_active(manager->activityBitset, i)) {
            continue;
        }

        ZF4MusicSrc* const src = &manager->srcs[i];

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

            zf4_log("Refreshed source buffer with OpenAL ID %d for music source %d.", bufALID, i);
        }
    }

    return true;
}

ZF4AudioSrcID zf4_add_music_src(ZF4MusicSrcManager* const manager, const int musicIndex) {
    const int srcIndex = zf4_get_first_inactive_bit_index(manager->activityBitset, ZF4_BITS_TO_BYTES(ZF4_MUSIC_SRC_LIMIT));
    assert(srcIndex != -1);

    ZF4MusicSrc* const src = &manager->srcs[srcIndex];
    src->musicIndex = musicIndex;

    alGenSources(1, &src->alID);
    alGenBuffers(ZF4_MUSIC_BUF_CNT, src->bufALIDs);

    zf4_activate_bit(manager->activityBitset, srcIndex);
    ++manager->versions[srcIndex];

    ZF4AudioSrcID id = {srcIndex, manager->versions[srcIndex]};
    return id;
}

void zf4_remove_music_src(ZF4MusicSrcManager* const manager, const ZF4AudioSrcID id) {
    assert(id.index >= 0 && id.index < ZF4_MUSIC_SRC_LIMIT);
    assert(manager->versions[id.index] == id.version);
    assert(zf4_is_bit_active(manager->activityBitset, id.index));

    clean_active_music_src(manager, id.index);
    zf4_deactivate_bit(manager->activityBitset, id.index);
}

bool zf4_play_music_src(ZF4MusicSrcManager* const manager, const ZF4AudioSrcID id, const float gain) {
    assert(id.index >= 0 && id.index < ZF4_MUSIC_SRC_LIMIT);
    assert(manager->versions[id.index] == id.version);
    assert(zf4_is_bit_active(manager->activityBitset, id.index));

    ZF4MusicSrc* const src = &manager->srcs[id.index];

    src->fs = fopen(ZF4_ASSETS_FILE_NAME, "rb");

    if (!src->fs) {
        return false;
    }

    fseek(src->fs, zf4_get_music()->sampleDataFilePositions[src->musicIndex], SEEK_SET);

    for (int i = 0; i < ZF4_MUSIC_BUF_CNT; ++i) {
        if (!load_music_buf_data(src, src->bufALIDs[i])) {
            return false;
        }
    }

    alSourceQueueBuffers(src->alID, ZF4_MUSIC_BUF_CNT, src->bufALIDs);

    alSourcef(src->alID, AL_GAIN, gain);
    alSourcePlay(src->alID);

    return true;
}
