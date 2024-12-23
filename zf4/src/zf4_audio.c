#include <zf4_audio.h>

#include <stdlib.h>
#include <zf4c.h>

static ALCdevice* i_alDevice;
static ALCcontext* i_alContext;

static void release_sound_src_by_index(ZF4SoundSrcManager* const manager, const int index) {
    assert(index >= 0 && index < ZF4_SOUND_SRC_LIMIT);
    assert(manager->alIDs[index]);

    alDeleteSources(1, &manager->alIDs[index]);
    manager->alIDs[index] = 0;
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

ZF4SoundSrcID zf4_add_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const ZF4Sounds* const snds) {
    for (int i = 0; i < ZF4_SOUND_SRC_LIMIT; ++i) {
        if (!manager->alIDs[i]) {
            alGenSources(1, &manager->alIDs[i]);
            alSourcei(manager->alIDs[i], AL_BUFFER, snds->bufALIDs[sndIndex]);

            ++manager->versions[i];

            ZF4SoundSrcID srcID = {.index = i, .version = manager->versions[i]};
            return srcID;
        }
    }

    assert(false);

    ZF4SoundSrcID emptyID = {0};
    return emptyID;
}

void zf4_remove_sound_src(ZF4SoundSrcManager* const manager, const ZF4SoundSrcID srcID) {
    assert(srcID.index >= 0 && srcID.index < ZF4_SOUND_SRC_LIMIT);
    assert(manager->versions[srcID.index] == srcID.version);
    assert(manager->alIDs[srcID.index]);

    release_sound_src_by_index(manager, srcID.index);
}

void zf4_play_sound_src(const ZF4SoundSrcManager* const manager, const ZF4SoundSrcID srcID, const float gain, const float pitch) {
    assert(srcID.index >= 0 && srcID.index < ZF4_SOUND_SRC_LIMIT);
    assert(manager->versions[srcID.index] == srcID.version);
    assert(manager->alIDs[srcID.index]);

    alSourceRewind(manager->alIDs[srcID.index]); // Restart if already playing.
    alSourcef(manager->alIDs[srcID.index], AL_GAIN, gain);
    alSourcef(manager->alIDs[srcID.index], AL_PITCH, pitch);
    alSourcePlay(manager->alIDs[srcID.index]);
}

void zf4_add_and_play_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const float gain, const float pitch, const ZF4Sounds* const snds) {
    const ZF4SoundSrcID srcID = zf4_add_sound_src(manager, sndIndex, snds);
    zf4_play_sound_src(manager, srcID, gain, pitch);
    zf4_activate_bit(manager->autoReleaseBitset, srcID.index); // No reference to this source is returned, so it needs to be automatically released once it is detected as finished.
}
