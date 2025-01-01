#include "zf4ap.h"

#include <sndfile.h>

static bool write_audio_file_data(FILE* const outputFS, float* const samples, const char* const filePath, const bool snd) {
    SF_INFO sfInfo;
    SNDFILE* const sf = sf_open(filePath, SFM_READ, &sfInfo);

    if (!sf) {
        zf4::log_error("Failed to open audio file with path \"%s\"!", filePath);
        return false;
    }

    const zf4::AudioInfo audioInfo = {
        .channelCnt = sfInfo.channels,
        .sampleCntPerChannel = sfInfo.frames,
        .sampleRate = sfInfo.samplerate
    };

    if (audioInfo.channelCnt != 1 && audioInfo.channelCnt != 2) {
        zf4::log_error("Audio file with path \"%s\" has an unsupported channel count of %d.", filePath, audioInfo.channelCnt);
        sf_close(sf);
        return false;
    }

    if (snd && audioInfo.sampleCntPerChannel * audioInfo.channelCnt > zf4::gk_soundSampleLimit) {
        zf4::log_error("Sound file with path \"%s\" exceeds the sample limit of %d!", filePath, zf4::gk_soundSampleLimit);
        sf_close(sf);
        return false;
    }

    fwrite(&audioInfo, sizeof(audioInfo), 1, outputFS);

    sf_count_t samplesRead;

    while ((samplesRead = sf_read_float(sf, samples, zf4::gk_audioSamplesPerChunk)) > 0) {
        fwrite(samples, sizeof(*samples), samplesRead, outputFS);

        if (samplesRead < zf4::gk_audioSamplesPerChunk) {
            break;
        }
    }

    sf_close(sf);

    return true;
}

bool pack_sounds(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjSnds) {
    const auto samples = zf4::alloc<float>(zf4::gk_audioSamplesPerChunk);

    if (!samples) {
        zf4::log_error("Failed to allocate memory for sound samples!");
        return false;
    }

    bool success = true;

    const cJSON* cjSndRelFilePath = nullptr;

    cJSON_ArrayForEach(cjSndRelFilePath, cjSnds) {
        if (!cJSON_IsString(cjSndRelFilePath)) {
            zf4::log_error("Failed to get sound file path from packing instructions JSON file!");
            success = false;
            break;
        }

        if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, cjSndRelFilePath->valuestring)) {
            success = false;
            break;
        }

        if (!write_audio_file_data(outputFS, samples, srcAssetFilePathBuf, true)) {
            success = false;
            break;
        }

        zf4::log("Packed sound with file path \"%s\".", srcAssetFilePathBuf);
    }

    free(samples);

    return success;
}

bool pack_music(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjMusic) {
    const auto samples = zf4::alloc<float>(zf4::gk_audioSamplesPerChunk);

    if (!samples) {
        zf4::log_error("Failed to allocate memory for music samples!");
        return false;
    }

    bool success = true;

    const cJSON* cjMusicRelFilePath = nullptr;

    cJSON_ArrayForEach(cjMusicRelFilePath, cjMusic) {
        if (!cJSON_IsString(cjMusicRelFilePath)) {
            zf4::log_error("Failed to get music file path from packing instructions JSON file!");
            success = false;
            break;
        }

        if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, cjMusicRelFilePath->valuestring)) {
            success = false;
            break;
        }

        if (!write_audio_file_data(outputFS, samples, srcAssetFilePathBuf, false)) {
            success = false;
            break;
        }

        zf4::log("Packed music with file path \"%s\".", srcAssetFilePathBuf);
    }

    free(samples);

    return success;
}
