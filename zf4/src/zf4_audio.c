#include <zf4_audio.h>

#include <stdlib.h>
#include <zf4c.h>

static ALCdevice* i_alDevice;
static ALCcontext* i_alContext;

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
