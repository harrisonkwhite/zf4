#include <zf4_renderer.h>

#define QUAD_LIMIT ZF4_MAX(ZF4_SPRITE_BATCH_SLOT_LIMIT, ZF4_CHAR_BATCH_SLOT_LIMIT)
#define QUAD_INDICES_LEN (6 * QUAD_LIMIT)

#define SPRITE_BATCH_SLOT_VERT_CNT (ZF4_SPRITE_QUAD_SHADER_PROG_VERT_CNT * 4)
#define SPRITE_BATCH_SLOT_VERTS_SIZE (sizeof(float) * SPRITE_BATCH_SLOT_VERT_CNT)

#define CHAR_BATCH_SLOT_VERTS_CNT (ZF4_CHAR_QUAD_SHADER_PROG_VERT_CNT * 4)
#define CHAR_BATCH_SLOT_VERTS_SIZE (sizeof(float) * CHAR_BATCH_SLOT_VERTS_CNT)

static ZF4QuadBuf gen_quad_buf(const int quadCnt, const bool sprite) {
    assert(quadCnt > 0);

    ZF4QuadBuf buf = {0};

    const int vertCnt = sprite ? ZF4_SPRITE_QUAD_SHADER_PROG_VERT_CNT : ZF4_CHAR_QUAD_SHADER_PROG_VERT_CNT;

    // Generate vertex array.
    glGenVertexArrays(1, &buf.vertArrayGLID);
    glBindVertexArray(buf.vertArrayGLID);

    // Generate vertex buffer.
    glGenBuffers(1, &buf.vertBufGLID);
    glBindBuffer(GL_ARRAY_BUFFER, buf.vertBufGLID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertCnt * 4 * quadCnt, NULL, GL_DYNAMIC_DRAW);

    // Generate element buffer.
    {
        static unsigned short quadIndices[QUAD_INDICES_LEN];
        static bool quadIndicesInitialized = false;

        if (!quadIndicesInitialized) {
            for (int i = 0; i < QUAD_LIMIT; i++) {
                quadIndices[(i * 6) + 0] = (i * 4) + 0;
                quadIndices[(i * 6) + 1] = (i * 4) + 1;
                quadIndices[(i * 6) + 2] = (i * 4) + 2;
                quadIndices[(i * 6) + 3] = (i * 4) + 2;
                quadIndices[(i * 6) + 4] = (i * 4) + 3;
                quadIndices[(i * 6) + 5] = (i * 4) + 0;
            }

            quadIndicesInitialized = true;
        }

        glGenBuffers(1, &buf.elemBufGLID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf.elemBufGLID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6 * quadCnt, quadIndices, GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int vertsStride = sizeof(float) * vertCnt;

    if (sprite) {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 4));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 6));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 7));
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 8));
        glEnableVertexAttribArray(5);

        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 10));
        glEnableVertexAttribArray(6);
    } else {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertsStride, (const void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);

    return buf;
}

static bool load_render_layer(ZF4RenderLayer* const layer, ZF4MemArena* const memArena, const int spriteBatchCnt) {
    layer->spriteBatchCnt = spriteBatchCnt;

    // Reserve memory in the arena.
    layer->spriteBatchQuadBufs = zf4_push_to_mem_arena(memArena, sizeof(*layer->spriteBatchQuadBufs) * spriteBatchCnt, alignof(ZF4QuadBuf));

    if (!layer->spriteBatchQuadBufs) {
        return false;
    }

    layer->spriteBatchTransients = zf4_push_to_mem_arena(memArena, sizeof(*layer->spriteBatchTransients) * spriteBatchCnt, alignof(ZF4SpriteBatchTransients));

    if (!layer->spriteBatchTransients) {
        return false;
    }

    // Generate quad buffers.
    for (int i = 0; i < spriteBatchCnt; ++i) {
        layer->spriteBatchQuadBufs[i] = gen_quad_buf(ZF4_SPRITE_BATCH_SLOT_LIMIT, true);
    }

    return true;
}

static int add_tex_unit_to_sprite_batch(ZF4SpriteBatchTransients* const batchTransData, const int texIndex) {
    for (int i = 0; i < batchTransData->texUnitsInUse; ++i) {
        if (batchTransData->texUnitTexIDs[i] == texIndex) {
            return i;
        }
    }

    if (batchTransData->texUnitsInUse == ZF4_TEX_UNIT_LIMIT) {
        return -1;
    }

    batchTransData->texUnitTexIDs[batchTransData->texUnitsInUse] = texIndex;

    return batchTransData->texUnitsInUse++;
}

static void init_cam_view_matrix(ZF4Matrix4x4* const mat, const ZF4Camera* const cam) {
    assert(zf4_is_zero(mat, sizeof(*mat)));

    mat->elems[0][0] = cam->scale;
    mat->elems[1][1] = cam->scale;
    mat->elems[3][3] = 1.0f;
    mat->elems[3][0] = (-cam->pos.x * cam->scale) + (zf4_get_window_size().x / 2.0f);
    mat->elems[3][1] = (-cam->pos.y * cam->scale) + (zf4_get_window_size().y / 2.0f);
}

bool zf4_load_renderer(ZF4Renderer* const renderer, ZF4MemArena* const memArena, const int layerCnt, const int* const layerSpriteBatchCnts) {
    assert(zf4_is_zero(renderer, sizeof(*renderer)));
    assert(layerCnt > 0);

    renderer->layers = zf4_push_to_mem_arena(memArena, sizeof(*renderer->layers) * layerCnt, alignof(ZF4RenderLayer));

    if (!renderer->layers) {
        return false;
    }

    renderer->layerCnt = layerCnt;

    for (int i = 0; i < layerCnt; ++i) {
        if (!load_render_layer(&renderer->layers[i], memArena, layerSpriteBatchCnts[i])) {
            return false;
        }
    }

    return true;
}

void zf4_clean_renderer(ZF4Renderer* const renderer) {
    for (int i = 0; i < renderer->layerCnt; ++i) {
        ZF4RenderLayer* const layer = &renderer->layers[i];

        for (int j = 0; j < layer->spriteBatchCnt; ++j) {
            glDeleteVertexArrays(1, &layer->spriteBatchQuadBufs[j].vertArrayGLID);
            glDeleteBuffers(1, &layer->spriteBatchQuadBufs[j].vertBufGLID);
            glDeleteBuffers(1, &layer->spriteBatchQuadBufs[j].elemBufGLID);
        }
    }

    memset(renderer, 0, sizeof(*renderer));
}

void zf4_render_all(const ZF4Renderer* const renderer, const ZF4ShaderProgs* const shaderProgs, const ZF4Assets* const assets) {
    glClearColor(renderer->bgColor.r, renderer->bgColor.g, renderer->bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    static int texUnits[ZF4_TEX_UNIT_LIMIT];
    static bool texUnitsInitialized = false;

    if (!texUnitsInitialized) {
        for (int i = 0; i < ZF4_TEX_UNIT_LIMIT; ++i) {
            texUnits[i] = i;
        }

        texUnitsInitialized = true;
    }

    ZF4Matrix4x4 projMat = {0};
    init_ortho_matrix_4x4(&projMat, 0.0f, zf4_get_window_size().x, zf4_get_window_size().y, 0.0f, -1.0f, 1.0f);

    ZF4Matrix4x4 camViewMat = {0};
    init_cam_view_matrix(&camViewMat, &renderer->cam);

    ZF4Matrix4x4 defaultViewMat = {0};
    init_identity_matrix_4x4(&defaultViewMat);

    for (int i = 0; i < renderer->layerCnt; ++i) {
        glUseProgram(shaderProgs->spriteQuad.glID);

        glUniformMatrix4fv(shaderProgs->spriteQuad.projUniLoc, 1, GL_FALSE, (const float*)projMat.elems);

        const ZF4Matrix4x4* const viewMat = i < renderer->camLayerCnt ? &camViewMat : &defaultViewMat;
        glUniformMatrix4fv(shaderProgs->spriteQuad.viewUniLoc, 1, GL_FALSE, (const float*)viewMat->elems);

        glUniform1iv(shaderProgs->spriteQuad.texturesUniLoc, ZF4_TEX_UNIT_LIMIT, texUnits);

        const ZF4RenderLayer* const layer = &renderer->layers[i];

        for (int j = 0; j < layer->spriteBatchCnt; ++j) {
            const ZF4QuadBuf* const batchQuadBuf = &layer->spriteBatchQuadBufs[j];
            const ZF4SpriteBatchTransients* const batchTransData = &layer->spriteBatchTransients[j];

            glBindVertexArray(batchQuadBuf->vertArrayGLID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchQuadBuf->elemBufGLID);

            for (int k = 0; k < batchTransData->texUnitsInUse; ++k) {
                glActiveTexture(GL_TEXTURE0 + k);
                glBindTexture(GL_TEXTURE_2D, assets->textures.glIDs[batchTransData->texUnitTexIDs[k]]);
            }

            glDrawElements(GL_TRIANGLES, 6 * batchTransData->slotsUsed, GL_UNSIGNED_SHORT, NULL);
        }
    }
}

void zf4_empty_sprite_batches(ZF4Renderer* const renderer) {
    for (int i = 0; i < renderer->layerCnt; ++i) {
        ZF4RenderLayer* const layer = &renderer->layers[i];
        memset(layer->spriteBatchTransients, 0, sizeof(*layer->spriteBatchTransients) * layer->spriteBatchCnt);
        layer->spriteBatchesFilled = 0;
    }
}

void zf4_write_to_sprite_batch(ZF4Renderer* const renderer, const int layerIndex, const int texIndex, const ZF4Vec2D pos, const ZF4Rect* const srcRect, const ZF4Vec2D origin, const float rot, const ZF4Vec2D scale, const float alpha, const ZF4Textures* const textures) {
    assert(layerIndex >= 0 && layerIndex < renderer->layerCnt);

    ZF4RenderLayer* const layer = &renderer->layers[layerIndex];

    const int batchIndex = layer->spriteBatchesFilled;
    ZF4SpriteBatchTransients* const batchTransData = &layer->spriteBatchTransients[batchIndex];

    int texUnit;

    if (batchTransData->slotsUsed == ZF4_SPRITE_BATCH_SLOT_LIMIT || (texUnit = add_tex_unit_to_sprite_batch(batchTransData, texIndex)) == -1) {
        ++layer->spriteBatchesFilled;

        if (layer->spriteBatchesFilled < layer->spriteBatchCnt) {
            zf4_write_to_sprite_batch(renderer, layerIndex, texIndex, pos, srcRect, origin, rot, scale, alpha, textures);
        } else {
            assert(false);
        }

        return;
    }

    const int slotIndex = batchTransData->slotsUsed;
    const ZF4Pt2D texSize = textures->sizes[texIndex];

    const float verts[] = {
        (0.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        srcRect->width,
        srcRect->height,
        rot,
        texUnit,
        (float)srcRect->x / texSize.x,
        (float)srcRect->y / texSize.y,
        alpha,

        (1.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        srcRect->width,
        srcRect->height,
        rot,
        texUnit,
        (float)(srcRect->x + srcRect->width) / texSize.x,
        (float)srcRect->y / texSize.y,
        alpha,

        (1.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        srcRect->width,
        srcRect->height,
        rot,
        texUnit,
        (float)(srcRect->x + srcRect->width) / texSize.x,
        (float)(srcRect->y + srcRect->height) / texSize.y,
        alpha,

        (0.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        srcRect->width,
        srcRect->height,
        rot,
        texUnit,
        (float)srcRect->x / texSize.x,
        (float)(srcRect->y + srcRect->height) / texSize.y,
        alpha
    };

    const ZF4QuadBuf* const batchQuadBuf = &layer->spriteBatchQuadBufs[batchIndex];
    glBindVertexArray(batchQuadBuf->vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batchQuadBuf->vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, slotIndex * sizeof(float) * SPRITE_BATCH_SLOT_VERT_CNT, sizeof(verts), verts);

    ++batchTransData->slotsUsed;
}
