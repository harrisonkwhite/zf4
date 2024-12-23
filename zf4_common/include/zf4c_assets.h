#ifndef ZF4C_ASSETS_H
#define ZF4C_ASSETS_H

#include <zf4c_math.h>

#define ZF4_ASSETS_FILE_NAME "assets.dat"

#define ZF4_TEX_WIDTH_LIMIT 2048
#define ZF4_TEX_HEIGHT_LIMIT 2048
#define ZF4_TEX_CHANNEL_CNT 4
#define ZF4_TEX_PX_LIMIT (ZF4_TEX_WIDTH_LIMIT * ZF4_TEX_HEIGHT_LIMIT)
#define ZF4_TEX_PX_DATA_SIZE_LIMIT (ZF4_TEX_CHANNEL_CNT * ZF4_TEX_PX_LIMIT)

#define ZF4_FONT_CHAR_RANGE_BEGIN 32
#define ZF4_FONT_CHAR_RANGE_SIZE 95

#define ZF4_AUDIO_SAMPLES_PER_CHUNK 44100

typedef enum {
    ZF4_TEX_ASSET_TYPE,
    ZF4_FONT_ASSET_TYPE,
    ZF4_SOUND_ASSET_TYPE,
    ZF4_MUSIC_ASSET_TYPE,

    ZF4_ASSET_TYPE_CNT
} ZF4AssetType;

typedef struct {
    int horOffsets[ZF4_FONT_CHAR_RANGE_SIZE];
    int verOffsets[ZF4_FONT_CHAR_RANGE_SIZE];
    int horAdvances[ZF4_FONT_CHAR_RANGE_SIZE];

    ZF4Rect srcRects[ZF4_FONT_CHAR_RANGE_SIZE];

    int kernings[ZF4_FONT_CHAR_RANGE_SIZE * ZF4_FONT_CHAR_RANGE_SIZE];
} ZF4FontCharsArrangementInfo;

typedef struct {
    int lineHeight;
    ZF4FontCharsArrangementInfo chars;
} ZF4FontArrangementInfo;

typedef struct {
    int channelCnt;
    long long sampleCntPerChannel;
    int sampleRate;
} ZF4AudioInfo;

#endif
