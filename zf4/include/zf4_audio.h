#ifndef ZF4_AUDIO_H
#define ZF4_AUDIO_H

#include <stdbool.h>
#include <AL/al.h>
#include <zf4c.h>
#include <zf4_assets.h>

#define ZF4_SOUND_SRC_LIMIT 128

typedef struct {
    int index;
    int version;
} ZF4SoundSrcID;

typedef struct {
    ALuint alIDs[ZF4_SOUND_SRC_LIMIT];
    int versions[ZF4_SOUND_SRC_LIMIT];
    ZF4Byte autoReleaseBitset[ZF4_BITS_TO_BYTES(ZF4_SOUND_SRC_LIMIT)]; // Indicates which sources need to be automatically released when finished (due to not them not being referenced).
} ZF4SoundSrcManager;

bool zf4_init_audio_system();
void zf4_clean_audio_system();

void zf4_clean_sound_srcs(ZF4SoundSrcManager* const manager);
void zf4_handle_auto_release_sound_srcs(ZF4SoundSrcManager* const manager);
ZF4SoundSrcID zf4_add_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const ZF4Sounds* const snds);
void zf4_remove_sound_src(ZF4SoundSrcManager* const manager, const ZF4SoundSrcID srcID);
void zf4_play_sound_src(const ZF4SoundSrcManager* const manager, const ZF4SoundSrcID srcID, const float gain, const float pitch);
void zf4_add_and_play_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const float gain, const float pitch, const ZF4Sounds* const snds);

#endif
