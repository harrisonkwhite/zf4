#ifndef ZF4_RENDERER_H
#define ZF4_RENDERER_H

#include <zf4c.h>
#include <zf4_assets.h>
#include <zf4_shader_progs.h>

#define ZF4_SPRITE_BATCH_SLOT_LIMIT 4096
#define ZF4_CHAR_BATCH_SLOT_LIMIT 1024
#define ZF4_TEX_UNIT_LIMIT 16

typedef enum {
    ZF4_FONT_HOR_ALIGN_LEFT,
    ZF4_FONT_HOR_ALIGN_CENTER,
    ZF4_FONT_HOR_ALIGN_RIGHT
} ZF4FontHorAlign;

typedef enum {
    ZF4_FONT_VER_ALIGN_TOP,
    ZF4_FONT_VER_ALIGN_CENTER,
    ZF4_FONT_VER_ALIGN_BOTTOM
} ZF4FontVerAlign;

typedef struct {
    GLuint vertArrayGLID;
    GLuint vertBufGLID;
    GLuint elemBufGLID;
} ZF4QuadBuf;

typedef struct {
    int slotsUsed;
    int texUnitTexIDs[ZF4_TEX_UNIT_LIMIT];
    int texUnitsInUse;
} ZF4SpriteBatchTransients;

typedef struct {
    int texIndex;
    ZF4Vec2D pos;
    ZF4Rect srcRect;
    ZF4Vec2D origin;
    float rot;
    ZF4Vec2D scale;
    float alpha;
} ZF4SpriteBatchWriteInfo;

typedef struct {
    int fontIndex;
    ZF4Vec2D pos;
    float rot;
    ZF4Vec4D blend;
} ZF4CharBatchDisplayProps;

typedef struct {
    ZF4QuadBuf quadBuf;
    int slotCnt;
    ZF4CharBatchDisplayProps displayProps;
} ZF4CharBatch;

typedef struct {
    int layerIndex;
    int batchIndex;
} ZF4CharBatchID;

typedef struct {
    int spriteBatchCnt;
    int charBatchCnt;
} ZF4RenderLayerProps;

typedef void (*ZF4RenderLayerPropsInitializer)(ZF4RenderLayerProps* const props, const int layerIndex);

typedef struct {
    ZF4QuadBuf* spriteBatchQuadBufs;
    ZF4SpriteBatchTransients* spriteBatchTransients;
    int spriteBatchesFilled;

    ZF4CharBatch* charBatches;
    ZF4Byte* charBatchActivity;

    ZF4RenderLayerProps props;
} ZF4RenderLayer;

typedef struct {
    ZF4Vec2D pos;
    float scale;
} ZF4Camera;

typedef struct {
    ZF4RenderLayer* layers;
    int layerCnt;
    int camLayerCnt;
    ZF4Vec3D bgColor;
    ZF4Camera cam;
} ZF4Renderer;

bool zf4_load_renderer(ZF4Renderer* const renderer, ZF4MemArena* const memArena, const int layerCnt, const int camLayerCnt, const ZF4RenderLayerPropsInitializer layerPropsInitializer);
void zf4_clean_renderer(ZF4Renderer* const renderer);
void zf4_render_all(const ZF4Renderer* const renderer, const ZF4ShaderProgs* const shaderProgs, const ZF4Assets* const assets);

void zf4_empty_sprite_batches(ZF4Renderer* const renderer);
void zf4_write_to_sprite_batch(ZF4Renderer* const renderer, const int layerIndex, const ZF4SpriteBatchWriteInfo* const info, const ZF4Textures* const textures);

ZF4CharBatchID zf4_activate_any_char_batch(ZF4Renderer* const renderer, const int layerIndex, const int slotCnt, const int fontIndex, const ZF4Vec2D pos);
void zf4_deactivate_char_batch(ZF4Renderer* const renderer, const ZF4CharBatchID id);
void zf4_write_to_char_batch(ZF4Renderer* const renderer, const ZF4CharBatchID id, const char* const text, const ZF4FontHorAlign horAlign, const ZF4FontVerAlign verAlign, const ZF4Fonts* const fonts);
void zf4_clear_char_batch(const ZF4Renderer* const renderer, const ZF4CharBatchID id);

inline ZF4Vec2D zf4_get_cam_size(const ZF4Camera* const cam) {
    return (ZF4Vec2D) { zf4_get_window_size().x / cam->scale, zf4_get_window_size().y / cam->scale };
}

inline ZF4Vec2D zf4_get_cam_top_left(const ZF4Camera* const cam) {
    const ZF4Vec2D camSize = zf4_get_cam_size(cam);
    return (ZF4Vec2D) { cam->pos.x - (camSize.x / 2.0f), cam->pos.y - (camSize.y / 2.0f) };
}

inline ZF4Vec2D zf4_get_cam_bottom_right(const ZF4Camera* const cam) {
    const ZF4Vec2D camSize = zf4_get_cam_size(cam);
    return (ZF4Vec2D) { cam->pos.x + (camSize.x / 2.0f), cam->pos.y + (camSize.y / 2.0f) };
}

#endif
