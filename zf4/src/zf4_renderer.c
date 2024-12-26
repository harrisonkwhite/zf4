#include <zf4_renderer.h>

#include <stdlib.h>
#include <stdalign.h>
#include <zf4_window.h>

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

static bool load_render_layer(ZF4RenderLayer* const layer, ZF4MemArena* const memArena, const ZF4RenderLayerProps props) {
    layer->props = props;

    // Reserve memory in the arena.
    if (props.spriteBatchCnt > 0) {
        layer->spriteBatchQuadBufs = zf4_push_to_mem_arena(memArena, sizeof(*layer->spriteBatchQuadBufs) * props.spriteBatchCnt, alignof(ZF4QuadBuf));

        if (!layer->spriteBatchQuadBufs) {
            return false;
        }

        layer->spriteBatchTransients = zf4_push_to_mem_arena(memArena, sizeof(*layer->spriteBatchTransients) * props.spriteBatchCnt, alignof(ZF4SpriteBatchTransients));

        if (!layer->spriteBatchTransients) {
            return false;
        }
    }

    if (props.charBatchCnt > 0) {
        layer->charBatches = zf4_push_to_mem_arena(memArena, sizeof(*layer->charBatches) * props.charBatchCnt, alignof(ZF4CharBatch));

        if (!layer->charBatches) {
            return false;
        }

        layer->charBatchActivityBitset = zf4_push_to_mem_arena(memArena, ZF4_BITS_TO_BYTES(props.charBatchCnt), alignof(ZF4Byte));

        if (!layer->charBatchActivityBitset) {
            return false;
        }
    }

    // Generate quad buffers.
    for (int i = 0; i < props.spriteBatchCnt; ++i) {
        layer->spriteBatchQuadBufs[i] = gen_quad_buf(ZF4_SPRITE_BATCH_SLOT_LIMIT, true);
    }

    for (int i = 0; i < props.charBatchCnt; ++i) {
        layer->charBatches[i].quadBuf = gen_quad_buf(ZF4_CHAR_BATCH_SLOT_LIMIT, false);
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

static void init_camera_view_matrix(ZF4Matrix4x4* const mat, const ZF4Camera* const cam) {
    assert(zf4_is_zero(mat, sizeof(*mat)));

    mat->elems[0][0] = cam->scale;
    mat->elems[1][1] = cam->scale;
    mat->elems[3][3] = 1.0f;
    mat->elems[3][0] = (-cam->pos.x * cam->scale) + (zf4_get_window_size().x / 2.0f);
    mat->elems[3][1] = (-cam->pos.y * cam->scale) + (zf4_get_window_size().y / 2.0f);
}

bool zf4_load_renderer(ZF4Renderer* const renderer, ZF4MemArena* const memArena, const int layerCnt, const int camLayerCnt, const ZF4RenderLayerPropsInitializer layerPropsInitializer) {
    assert(zf4_is_zero(renderer, sizeof(*renderer)));
    assert(layerCnt > 0);
    assert(camLayerCnt >= 0 && camLayerCnt <= layerCnt);

    renderer->layers = zf4_push_to_mem_arena(memArena, sizeof(*renderer->layers) * layerCnt, alignof(ZF4RenderLayer));

    if (!renderer->layers) {
        return false;
    }

    renderer->layerCnt = layerCnt;
    renderer->camLayerCnt = camLayerCnt;

    for (int i = 0; i < layerCnt; ++i) {
        ZF4RenderLayerProps props = {0};
        layerPropsInitializer(&props, i);
        assert(!zf4_is_zero(&props, sizeof(props)));

        if (!load_render_layer(&renderer->layers[i], memArena, props)) {
            return false;
        }
    }

    return true;
}

void zf4_clean_renderer(ZF4Renderer* const renderer) {
    for (int i = 0; i < renderer->layerCnt; ++i) {
        ZF4RenderLayer* const layer = &renderer->layers[i];

        for (int j = 0; j < layer->props.spriteBatchCnt; ++j) {
            glDeleteVertexArrays(1, &layer->spriteBatchQuadBufs[j].vertArrayGLID);
            glDeleteBuffers(1, &layer->spriteBatchQuadBufs[j].vertBufGLID);
            glDeleteBuffers(1, &layer->spriteBatchQuadBufs[j].elemBufGLID);
        }
    }

    memset(renderer, 0, sizeof(*renderer));
}

void zf4_render_all(const ZF4Renderer* const renderer, const ZF4ShaderProgs* const shaderProgs) {
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
    zf4_init_ortho_matrix_4x4(&projMat, 0.0f, zf4_get_window_size().x, zf4_get_window_size().y, 0.0f, -1.0f, 1.0f);

    ZF4Matrix4x4 camViewMat = {0};
    init_camera_view_matrix(&camViewMat, &renderer->cam);

    ZF4Matrix4x4 defaultViewMat = {0};
    zf4_init_identity_matrix_4x4(&defaultViewMat);

    for (int i = 0; i < renderer->layerCnt; ++i) {
        //
        // Sprite Batches
        //
        glUseProgram(shaderProgs->spriteQuad.glID);

        glUniformMatrix4fv(shaderProgs->spriteQuad.projUniLoc, 1, GL_FALSE, (const float*)projMat.elems);

        const ZF4Matrix4x4* const viewMat = i < renderer->camLayerCnt ? &camViewMat : &defaultViewMat;
        glUniformMatrix4fv(shaderProgs->spriteQuad.viewUniLoc, 1, GL_FALSE, (const float*)viewMat->elems);

        glUniform1iv(shaderProgs->spriteQuad.texturesUniLoc, ZF4_TEX_UNIT_LIMIT, texUnits);

        const ZF4RenderLayer* const layer = &renderer->layers[i];

        for (int j = 0; j < layer->props.spriteBatchCnt; ++j) {
            const ZF4QuadBuf* const batchQuadBuf = &layer->spriteBatchQuadBufs[j];
            const ZF4SpriteBatchTransients* const batchTransData = &layer->spriteBatchTransients[j];

            glBindVertexArray(batchQuadBuf->vertArrayGLID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchQuadBuf->elemBufGLID);

            for (int k = 0; k < batchTransData->texUnitsInUse; ++k) {
                glActiveTexture(GL_TEXTURE0 + k);
                glBindTexture(GL_TEXTURE_2D, zf4_get_textures()->glIDs[batchTransData->texUnitTexIDs[k]]);
            }

            glDrawElements(GL_TRIANGLES, 6 * batchTransData->slotsUsed, GL_UNSIGNED_SHORT, NULL);
        }

        //
        // Character Batches
        //
        glUseProgram(shaderProgs->charQuad.glID);

        glUniformMatrix4fv(shaderProgs->charQuad.projUniLoc, 1, false, (const float*)projMat.elems);
        glUniformMatrix4fv(shaderProgs->charQuad.viewUniLoc, 1, false, (const float*)viewMat->elems);

        for (int j = 0; j < layer->props.charBatchCnt; ++j) {
            if (!zf4_is_bit_active(layer->charBatchActivityBitset, j)) {
                continue;
            }

            const ZF4CharBatch* const batch = &layer->charBatches[j];

            glUniform2fv(shaderProgs->charQuad.posUniLoc, 1, (const float*)&batch->displayProps.pos);
            glUniform1f(shaderProgs->charQuad.rotUniLoc, batch->displayProps.rot);
            glUniform4fv(shaderProgs->charQuad.blendUniLoc, 1, (const float*)&batch->displayProps.blend);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, zf4_get_fonts()->texGLIDs[batch->displayProps.fontIndex]);

            glBindVertexArray(batch->quadBuf.vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * batch->slotCnt, GL_UNSIGNED_SHORT, NULL);
        }
    }
}

void zf4_empty_sprite_batches(ZF4Renderer* const renderer) {
    for (int i = 0; i < renderer->layerCnt; ++i) {
        ZF4RenderLayer* const layer = &renderer->layers[i];
        memset(layer->spriteBatchTransients, 0, sizeof(*layer->spriteBatchTransients) * layer->props.spriteBatchCnt);
        layer->spriteBatchesFilled = 0;
    }
}

void zf4_write_to_sprite_batch(ZF4Renderer* const renderer, const int layerIndex, const ZF4SpriteBatchWriteInfo* const info) {
    assert(layerIndex >= 0 && layerIndex < renderer->layerCnt);

    ZF4RenderLayer* const layer = &renderer->layers[layerIndex];

    const int batchIndex = layer->spriteBatchesFilled;
    ZF4SpriteBatchTransients* const batchTransData = &layer->spriteBatchTransients[batchIndex];

    int texUnit;

    if (batchTransData->slotsUsed == ZF4_SPRITE_BATCH_SLOT_LIMIT || (texUnit = add_tex_unit_to_sprite_batch(batchTransData, info->texIndex)) == -1) {
        ++layer->spriteBatchesFilled;

        if (layer->spriteBatchesFilled < layer->props.spriteBatchCnt) {
            zf4_write_to_sprite_batch(renderer, layerIndex, info);
        } else {
            assert(false);
        }

        return;
    }

    const int slotIndex = batchTransData->slotsUsed;
    const ZF4Pt2D texSize = zf4_get_textures()->sizes[info->texIndex];

    const float verts[] = {
        (0.0f - info->origin.x) * info->scale.x,
        (0.0f - info->origin.y) * info->scale.y,
        info->pos.x,
        info->pos.y,
        info->srcRect.width,
        info->srcRect.height,
        info->rot,
        texUnit,
        (float)info->srcRect.x / texSize.x,
        (float)info->srcRect.y / texSize.y,
        info->alpha,

        (1.0f - info->origin.x) * info->scale.x,
        (0.0f - info->origin.y) * info->scale.y,
        info->pos.x,
        info->pos.y,
        info->srcRect.width,
        info->srcRect.height,
        info->rot,
        texUnit,
        (float)(info->srcRect.x + info->srcRect.width) / texSize.x,
        (float)info->srcRect.y / texSize.y,
        info->alpha,

        (1.0f - info->origin.x) * info->scale.x,
        (1.0f - info->origin.y) * info->scale.y,
        info->pos.x,
        info->pos.y,
        info->srcRect.width,
        info->srcRect.height,
        info->rot,
        texUnit,
        (float)(info->srcRect.x + info->srcRect.width) / texSize.x,
        (float)(info->srcRect.y + info->srcRect.height) / texSize.y,
        info->alpha,

        (0.0f - info->origin.x) * info->scale.x,
        (1.0f - info->origin.y) * info->scale.y,
        info->pos.x,
        info->pos.y,
        info->srcRect.width,
        info->srcRect.height,
        info->rot,
        texUnit,
        (float)info->srcRect.x / texSize.x,
        (float)(info->srcRect.y + info->srcRect.height) / texSize.y,
        info->alpha
    };

    const ZF4QuadBuf* const batchQuadBuf = &layer->spriteBatchQuadBufs[batchIndex];
    glBindVertexArray(batchQuadBuf->vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batchQuadBuf->vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, slotIndex * sizeof(float) * SPRITE_BATCH_SLOT_VERT_CNT, sizeof(verts), verts);

    ++batchTransData->slotsUsed;
}

ZF4CharBatchID zf4_activate_any_char_batch(ZF4Renderer* const renderer, const int layerIndex, const int slotCnt, const int fontIndex) {
    assert(layerIndex >= 0 && layerIndex < renderer->layerCnt);
    assert(slotCnt > 0 && slotCnt < ZF4_CHAR_BATCH_SLOT_LIMIT);

    ZF4RenderLayer* const layer = &renderer->layers[layerIndex];

    const int batchIndex = zf4_get_first_inactive_bit_index(layer->charBatchActivityBitset, layer->props.charBatchCnt);
    assert(batchIndex != -1);

    zf4_activate_bit(layer->charBatchActivityBitset, batchIndex);

    layer->charBatches[batchIndex] = (ZF4CharBatch) {
        .quadBuf = gen_quad_buf(slotCnt, false),
        .slotCnt = slotCnt,
        .displayProps = {
            .fontIndex = fontIndex,
            .blend = {1.0f, 1.0f, 1.0f, 1.0f}
        }
    };

    return (ZF4CharBatchID) {layerIndex, batchIndex};
}

void zf4_deactivate_char_batch(ZF4Renderer* const renderer, const ZF4CharBatchID id) {
    ZF4RenderLayer* const layer = &renderer->layers[id.layerIndex];
    zf4_deactivate_bit(layer->charBatchActivityBitset, id.batchIndex);
}

void zf4_write_to_char_batch(ZF4Renderer* const renderer, const ZF4CharBatchID id, const char* const text, const ZF4FontHorAlign horAlign, const ZF4FontVerAlign verAlign) {
    ZF4RenderLayer* const layer = &renderer->layers[id.layerIndex];
    ZF4CharBatch* const batch = &layer->charBatches[id.batchIndex];

    const int textLen = strlen(text);
    assert(textLen > 0 && textLen <= batch->slotCnt);

    const ZF4FontArrangementInfo* const fontArrangementInfo = &zf4_get_fonts()->arrangementInfos[batch->displayProps.fontIndex];
    const ZF4Pt2D fontTexSize = zf4_get_fonts()->texSizes[batch->displayProps.fontIndex];

    ZF4Vec2D charDrawPositions[ZF4_CHAR_BATCH_SLOT_LIMIT];
    ZF4Vec2D charDrawPosPen = {0};

    int textLineWidths[ZF4_CHAR_BATCH_SLOT_LIMIT + 1];
    int textFirstLineMinOffs = 0;
    bool textFirstLineMinOffsUpdated = false;
    int textLastLineMaxHeight = 0;
    bool textLastLineMaxHeightUpdated = false;
    int textLineCnter = 0;

    for (int i = 0; i < textLen; i++) {
        if (text[i] == '\n') {
            textLineWidths[textLineCnter] = charDrawPosPen.x;

            if (!textFirstLineMinOffsUpdated) {
                textFirstLineMinOffs = fontArrangementInfo->chars.verOffsets[0];
                textFirstLineMinOffsUpdated = true;
            }

            textLastLineMaxHeight = fontArrangementInfo->chars.verOffsets[0] + fontArrangementInfo->chars.srcRects[0].height;
            textLastLineMaxHeightUpdated = false;

            textLineCnter++;
            charDrawPosPen.x = 0.0f;
            charDrawPosPen.y += fontArrangementInfo->lineHeight;
            continue;
        }

        const int textCharIndex = text[i] - ZF4_FONT_CHAR_RANGE_BEGIN;

        if (textLineCnter == 0) {
            if (!textFirstLineMinOffsUpdated) {
                textFirstLineMinOffs = fontArrangementInfo->chars.verOffsets[textCharIndex];
                textFirstLineMinOffsUpdated = true;
            } else {
                textFirstLineMinOffs = ZF4_MIN(fontArrangementInfo->chars.verOffsets[textCharIndex], textFirstLineMinOffs);
            }
        }

        if (!textLastLineMaxHeightUpdated) {
            textLastLineMaxHeight = fontArrangementInfo->chars.verOffsets[textCharIndex] + fontArrangementInfo->chars.srcRects[textCharIndex].height;
            textLastLineMaxHeightUpdated = true;
        } else {
            textLastLineMaxHeight = ZF4_MAX(fontArrangementInfo->chars.verOffsets[textCharIndex] + fontArrangementInfo->chars.srcRects[textCharIndex].height, textLastLineMaxHeight);
        }

        if (i > 0) {
            const int textCharIndexLast = text[i - 1] - ZF4_FONT_CHAR_RANGE_BEGIN;
            charDrawPosPen.x += fontArrangementInfo->chars.kernings[(textCharIndex * ZF4_FONT_CHAR_RANGE_SIZE) + textCharIndexLast];
        }

        charDrawPositions[i].x = charDrawPosPen.x + fontArrangementInfo->chars.horOffsets[textCharIndex];
        charDrawPositions[i].y = charDrawPosPen.y + fontArrangementInfo->chars.verOffsets[textCharIndex];

        charDrawPosPen.x += fontArrangementInfo->chars.horAdvances[textCharIndex];
    }

    textLineWidths[textLineCnter] = charDrawPosPen.x;
    textLineCnter = 0;

    const int textHeight = textFirstLineMinOffs + charDrawPosPen.y + textLastLineMaxHeight;

    zf4_clear_char_batch(renderer, id);

    const int vertsLen = CHAR_BATCH_SLOT_VERTS_CNT * textLen;
    float* const verts = calloc(vertsLen, sizeof(*verts));

    if (!verts) {
        return;
    }

    for (int i = 0; i < textLen; i++) {
        if (text[i] == '\n') {
            textLineCnter++;
            continue;
        }

        if (text[i] == ' ') {
            continue;
        }

        const int charIndex = text[i] - ZF4_FONT_CHAR_RANGE_BEGIN;

        const ZF4Vec2D charDrawPos = {
            charDrawPositions[i].x - (textLineWidths[textLineCnter] * horAlign * 0.5f),
            charDrawPositions[i].y - (textHeight * verAlign * 0.5f)
        };

        const ZF4Vec2D charTexCoordsTopLeft = {
            (float)fontArrangementInfo->chars.srcRects[charIndex].x / fontTexSize.x,
            (float)fontArrangementInfo->chars.srcRects[charIndex].y / fontTexSize.y
        };

        const ZF4Vec2D charTexCoordsBottomRight = {
            (float)zf4_get_rect_right(&fontArrangementInfo->chars.srcRects[charIndex]) / fontTexSize.x,
            (float)zf4_get_rect_bottom(&fontArrangementInfo->chars.srcRects[charIndex]) / fontTexSize.y
        };

        float* const slotVerts = verts + (i * CHAR_BATCH_SLOT_VERTS_CNT);

        slotVerts[0] = charDrawPos.x;
        slotVerts[1] = charDrawPos.y;
        slotVerts[2] = charTexCoordsTopLeft.x;
        slotVerts[3] = charTexCoordsTopLeft.y;

        slotVerts[4] = charDrawPos.x + fontArrangementInfo->chars.srcRects[charIndex].width;
        slotVerts[5] = charDrawPos.y;
        slotVerts[6] = charTexCoordsBottomRight.x;
        slotVerts[7] = charTexCoordsTopLeft.y;

        slotVerts[8] = charDrawPos.x + fontArrangementInfo->chars.srcRects[charIndex].width;
        slotVerts[9] = charDrawPos.y + fontArrangementInfo->chars.srcRects[charIndex].height;
        slotVerts[10] = charTexCoordsBottomRight.x;
        slotVerts[11] = charTexCoordsBottomRight.y;

        slotVerts[12] = charDrawPos.x;
        slotVerts[13] = charDrawPos.y + fontArrangementInfo->chars.srcRects[charIndex].height;
        slotVerts[14] = charTexCoordsTopLeft.x;
        slotVerts[15] = charTexCoordsBottomRight.y;
    }

    glBindVertexArray(batch->quadBuf.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batch->quadBuf.vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts[0]) * vertsLen, verts);

    free(verts);
}

void zf4_clear_char_batch(const ZF4Renderer* const renderer, const ZF4CharBatchID id) {
    const ZF4RenderLayer* const layer = &renderer->layers[id.layerIndex];
    const ZF4CharBatch* const batch = &layer->charBatches[id.batchIndex];

    glBindVertexArray(batch->quadBuf.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batch->quadBuf.vertBufGLID);
    glBufferData(GL_ARRAY_BUFFER, CHAR_BATCH_SLOT_VERTS_SIZE * batch->slotCnt, NULL, GL_DYNAMIC_DRAW);
}
