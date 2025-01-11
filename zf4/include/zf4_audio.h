#pragma once

#include <cstdio>
#include <AL/al.h>
#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    constexpr int gk_soundSrcLimit = 128;

    constexpr int gk_musicSrcLimit = 16;
    constexpr int gk_musicBufCnt = 4; // Number of buffers that can concurrently hold parts of a music piece's sample data.
    constexpr int gk_musicBufSampleCnt = 88200;
    constexpr long long gk_musicBufSize = sizeof(AudioSample) * gk_musicBufSampleCnt;

    struct AudioSrcID {
        int index;
        int version;
    };

    struct SoundSrcManager {
        ALuint alIDs[gk_soundSrcLimit];
        int versions[gk_soundSrcLimit];
        Byte autoReleaseBitset[bits_to_bytes(gk_soundSrcLimit)]; // Indicates which sources need to be automatically released when finished (due to not them not being referenced).
    };

    struct MusicSrc {
        int musicIndex;
        ALuint alID;
        ALuint bufALIDs[gk_musicBufCnt];
        FILE* fs;
        int fsBytesRead;
    };

    struct MusicSrcManager {
        MusicSrc srcs[gk_musicSrcLimit];
        Byte activityBitset[bits_to_bytes(gk_musicSrcLimit)];
        int versions[gk_musicSrcLimit];
    };

    bool init_audio_system();
    void clean_audio_system();

    void clean_sound_srcs(SoundSrcManager* const manager);
    void handle_auto_release_sound_srcs(SoundSrcManager* const manager);
    AudioSrcID add_sound_src(SoundSrcManager* const manager, const int sndIndex, const Assets& assets);
    void remove_sound_src(SoundSrcManager* const manager, const AudioSrcID srcID);
    void play_sound_src(const SoundSrcManager* const manager, const AudioSrcID srcID, const float gain, const float pitch);
    void add_and_play_sound_src(SoundSrcManager* const manager, const int sndIndex, const Assets& assets, const float gain, const float pitch);

    void clean_music_srcs(MusicSrcManager* const manager);
    bool refresh_music_src_bufs(MusicSrcManager* const manager, const Assets& assets);
    AudioSrcID add_music_src(MusicSrcManager* const manager, const int musicIndex);
    void remove_music_src(MusicSrcManager* const manager, const AudioSrcID id);
    bool play_music_src(MusicSrcManager* const manager, const AudioSrcID id, const Assets& assets, const float gain = 1.0f);
}
