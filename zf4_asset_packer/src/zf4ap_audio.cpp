#include "zf4ap.h"

#include <sndfile.h>

namespace zf4 {
    static bool WriteAudioFileData(FILE* const output_fs, zf4::ta_audio_sample* const samples, const char* const file_path, const bool snd) {
        SF_INFO sf_info;
        SNDFILE* const sf = sf_open(file_path, SFM_READ, &sf_info);

        if (!sf) {
            LogError("Failed to open audio file with path \"%s\"!", file_path);
            return false;
        }

        const zf4::s_audio_info audio_info = {
            .channel_cnt = sf_info.channels,
            .sample_cnt_per_channel = sf_info.frames,
            .sample_rate = sf_info.samplerate
        };

        if (audio_info.channel_cnt != 1 && audio_info.channel_cnt != 2) {
            zf4::LogError("Audio file with path \"%s\" has an unsupported channel count of %d.", file_path, audio_info.channel_cnt);
            sf_close(sf);
            return false;
        }

        if (snd && audio_info.sample_cnt_per_channel * audio_info.channel_cnt > g_sound_sample_limit) {
            zf4::LogError("Sound file with path \"%s\" exceeds the sample limit of %d!", file_path, g_sound_sample_limit);
            sf_close(sf);
            return false;
        }

        fwrite(&audio_info, sizeof(audio_info), 1, output_fs);

        sf_count_t samples_read;

        while ((samples_read = sf_read_float(sf, samples, g_audio_samples_per_chunk)) > 0) {
            fwrite(samples, sizeof(*samples), samples_read, output_fs);

            if (samples_read < g_audio_samples_per_chunk) {
                break;
            }
        }

        sf_close(sf);

        return true;
    }

    bool PackSounds(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_snds) {
        const auto samples = (ta_audio_sample*)malloc(sizeof(ta_audio_sample) * g_audio_samples_per_chunk);

        if (!samples) {
            LogError("Failed to allocate memory for sound samples!");
            return false;
        }

        bool success = true;

        const cJSON* cj_snd_rel_file_path = nullptr;

        cJSON_ArrayForEach(cj_snd_rel_file_path, cj_snds) {
            if (!cJSON_IsString(cj_snd_rel_file_path)) {
                LogError("Failed to get sound file path from packing instructions JSON file!");
                success = false;
                break;
            }

            if (!CompleteAssetFilePath(src_asset_file_path_buf, src_asset_file_path_start_len, cj_snd_rel_file_path->valuestring)) {
                success = false;
                break;
            }

            if (!WriteAudioFileData(output_fs, samples, src_asset_file_path_buf, true)) {
                success = false;
                break;
            }

            Log("Packed sound with file path \"%s\".", src_asset_file_path_buf);
        }

        free(samples);

        return success;
    }

    bool PackMusic(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_music) {
        const auto samples = (ta_audio_sample*)malloc(sizeof(ta_audio_sample) * g_audio_samples_per_chunk);

        if (!samples) {
            LogError("Failed to allocate memory for music samples!");
            return false;
        }

        bool success = true;

        const cJSON* cj_music_rel_file_path = nullptr;

        cJSON_ArrayForEach(cj_music_rel_file_path, cj_music) {
            if (!cJSON_IsString(cj_music_rel_file_path)) {
                LogError("Failed to get music file path from packing instructions JSON file!");
                success = false;
                break;
            }

            if (!CompleteAssetFilePath(src_asset_file_path_buf, src_asset_file_path_start_len, cj_music_rel_file_path->valuestring)) {
                success = false;
                break;
            }

            if (!WriteAudioFileData(output_fs, samples, src_asset_file_path_buf, false)) {
                success = false;
                break;
            }

            Log("Packed music with file path \"%s\".", src_asset_file_path_buf);
        }

        free(samples);

        return success;
    }
}
