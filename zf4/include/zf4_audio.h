#ifndef ZF4_AUDIO_H
#define ZF4_AUDIO_H

#include <stdio.h>
#include <stdbool.h>
#include <AL/al.h>
#include <zf4c.h>
#include <zf4_assets.h>

#define ZF4_SOUND_SRC_LIMIT 128

#define ZF4_MUSIC_SRC_LIMIT 16
#define ZF4_MUSIC_BUF_CNT 4 // Number of buffers that can concurrently hold parts of a music piece's sample data.
#define ZF4_MUSIC_BUF_SAMPLE_CNT 88200
#define ZF4_MUSIC_BUF_SIZE (sizeof(ZF4AudioSample) * ZF4_MUSIC_BUF_SAMPLE_CNT)

typedef struct {
    int index;
    int version;
} ZF4AudioSrcID;

typedef struct {
    ALuint alIDs[ZF4_SOUND_SRC_LIMIT];
    int versions[ZF4_SOUND_SRC_LIMIT];
    ZF4Byte autoReleaseBitset[ZF4_BITS_TO_BYTES(ZF4_SOUND_SRC_LIMIT)]; // Indicates which sources need to be automatically released when finished (due to not them not being referenced).
} ZF4SoundSrcManager;

typedef struct {
    int musicIndex;

    ALuint alID;
    ALuint bufALIDs[ZF4_MUSIC_BUF_CNT];

    FILE* fs;
    int fsBytesRead;
} ZF4MusicSrc;

typedef struct {
    ZF4MusicSrc srcs[ZF4_MUSIC_SRC_LIMIT];
    ZF4Byte activityBitset[ZF4_BITS_TO_BYTES(ZF4_MUSIC_SRC_LIMIT)];
    int versions[ZF4_MUSIC_SRC_LIMIT];
} ZF4MusicSrcManager;

bool zf4_init_audio_system();
void zf4_clean_audio_system();

void zf4_clean_sound_srcs(ZF4SoundSrcManager* const manager);
void zf4_handle_auto_release_sound_srcs(ZF4SoundSrcManager* const manager);
ZF4AudioSrcID zf4_add_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const ZF4Sounds* const snds);
void zf4_remove_sound_src(ZF4SoundSrcManager* const manager, const ZF4AudioSrcID srcID);
void zf4_play_sound_src(const ZF4SoundSrcManager* const manager, const ZF4AudioSrcID srcID, const float gain, const float pitch);
void zf4_add_and_play_sound_src(ZF4SoundSrcManager* const manager, const int sndIndex, const float gain, const float pitch, const ZF4Sounds* const snds);

void zf4_clean_music_srcs(ZF4MusicSrcManager* const manager);
bool zf4_refresh_music_src_bufs(ZF4MusicSrcManager* const manager, const ZF4Music* const music);
ZF4AudioSrcID zf4_add_music_src(ZF4MusicSrcManager* const manager, const int musicIndex);
void zf4_remove_music_src(ZF4MusicSrcManager* const manager, const ZF4AudioSrcID id);
bool zf4_play_music_src(ZF4MusicSrcManager* const manager, const ZF4AudioSrcID id, const float gain, const ZF4Music* const music);

#endif
