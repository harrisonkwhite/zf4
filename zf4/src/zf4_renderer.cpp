#include <zf4_renderer.h>

#include <algorithm>
#include <cstdlib>
#include <zf4_window.h>

namespace zf4 {
    static constexpr int ik_quadLimit = std::max(gk_spriteBatchSlotLimit, gk_charBatchSlotLimit);
    static constexpr int ik_quadIndicesLen = 6 * ik_quadLimit;

    static constexpr int ik_spriteBatchSlotVertCnt = gk_spriteQuadShaderProgVertCnt * 4;
    static constexpr int ik_spriteBatchSlotVertsSize = sizeof(float) * ik_spriteBatchSlotVertCnt;

    static constexpr int ik_charBatchSlotVertsCnt = gk_charQuadShaderProgVertCnt * 4;
    static constexpr int ik_charBatchSlotVertsSize = sizeof(float) * ik_charBatchSlotVertsCnt;

    static QuadBuf gen_quad_buf(const int quadCnt, const bool sprite) {
        assert(quadCnt > 0);

        QuadBuf buf = {};

        const int vertCnt = sprite ? gk_spriteQuadShaderProgVertCnt : gk_charQuadShaderProgVertCnt;

        // Generate vertex array.
        glGenVertexArrays(1, &buf.vertArrayGLID);
        glBindVertexArray(buf.vertArrayGLID);

        // Generate vertex buffer.
        glGenBuffers(1, &buf.vertBufGLID);
        glBindBuffer(GL_ARRAY_BUFFER, buf.vertBufGLID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertCnt * 4 * quadCnt, nullptr, GL_DYNAMIC_DRAW);

        // Generate element buffer.
        {
            static unsigned short quadIndices[ik_quadIndicesLen];
            static bool quadIndicesInitialized = false;

            if (!quadIndicesInitialized) {
                for (unsigned short i = 0; i < ik_quadLimit; i++) {
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
            glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 0));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 2));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 4));
            glEnableVertexAttribArray(2);

            glVertexAttribPointer(3, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 6));
            glEnableVertexAttribArray(3);

            glVertexAttribPointer(4, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 7));
            glEnableVertexAttribArray(4);

            glVertexAttribPointer(5, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 8));
            glEnableVertexAttribArray(5);

            glVertexAttribPointer(6, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 10));
            glEnableVertexAttribArray(6);
        } else {
            glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 0));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void*>(sizeof(float) * 2));
            glEnableVertexAttribArray(1);
        }

        glBindVertexArray(0);

        return buf;
    }

    static bool load_render_layer(RenderLayer* const layer, MemArena* const memArena, const RenderLayerProps props) {
        layer->props = props;

        // Reserve memory in the arena.
        if (props.spriteBatchCnt > 0) {
            layer->spriteBatchQuadBufs = memArena->push<QuadBuf>(props.spriteBatchCnt);

            if (!layer->spriteBatchQuadBufs) {
                return false;
            }

            layer->spriteBatchTransients = memArena->push<SpriteBatchTransients>(props.spriteBatchCnt);

            if (!layer->spriteBatchTransients) {
                return false;
            }
        }

        if (props.charBatchCnt > 0) {
            layer->charBatches = memArena->push<CharBatch>(props.charBatchCnt);

            if (!layer->charBatches) {
                return false;
            }

            layer->charBatchActivityBitset = memArena->push<Byte>(bits_to_bytes(props.charBatchCnt));

            if (!layer->charBatchActivityBitset) {
                return false;
            }
        }

        // Generate quad buffers.
        for (int i = 0; i < props.spriteBatchCnt; ++i) {
            layer->spriteBatchQuadBufs[i] = gen_quad_buf(gk_spriteBatchSlotLimit, true);
        }

        for (int i = 0; i < props.charBatchCnt; ++i) {
            layer->charBatches[i].quadBuf = gen_quad_buf(gk_charBatchSlotLimit, false);
        }

        return true;
    }

    static int add_tex_unit_to_sprite_batch(SpriteBatchTransients* const batchTransData, const int texIndex) {
        for (int i = 0; i < batchTransData->texUnitsInUse; ++i) {
            if (batchTransData->texUnitTexIDs[i] == texIndex) {
                return i;
            }
        }

        if (batchTransData->texUnitsInUse == gk_texUnitLimit) {
            return -1;
        }

        batchTransData->texUnitTexIDs[batchTransData->texUnitsInUse] = texIndex;

        return batchTransData->texUnitsInUse++;
    }

    static void init_camera_view_matrix(Matrix4x4* const mat, const Camera* const cam) {
        assert(is_zero(mat));

        mat->elems[0][0] = cam->scale;
        mat->elems[1][1] = cam->scale;
        mat->elems[3][3] = 1.0f;
        mat->elems[3][0] = (-cam->pos.x * cam->scale) + (get_window_size().x / 2.0f);
        mat->elems[3][1] = (-cam->pos.y * cam->scale) + (get_window_size().y / 2.0f);
    }

    bool load_renderer(Renderer* const renderer, MemArena* const memArena, const int layerCnt, const int camLayerCnt, const RenderLayerPropsInitializer layerPropsInitializer) {
        assert(is_zero(renderer));
        assert(layerCnt > 0);
        assert(camLayerCnt >= 0 && camLayerCnt <= layerCnt);

        renderer->layers = memArena->push<RenderLayer>(layerCnt);

        if (!renderer->layers) {
            return false;
        }

        renderer->layerCnt = layerCnt;
        renderer->camLayerCnt = camLayerCnt;

        for (int i = 0; i < layerCnt; ++i) {
            RenderLayerProps props = {};
            layerPropsInitializer(&props, i);
            assert(!is_zero(&props));

            if (!load_render_layer(&renderer->layers[i], memArena, props)) {
                return false;
            }
        }

        return true;
    }

    void clean_renderer(Renderer* const renderer) {
        for (int i = 0; i < renderer->layerCnt; ++i) {
            RenderLayer* const layer = &renderer->layers[i];

            for (int j = 0; j < layer->props.spriteBatchCnt; ++j) {
                glDeleteVertexArrays(1, &layer->spriteBatchQuadBufs[j].vertArrayGLID);
                glDeleteBuffers(1, &layer->spriteBatchQuadBufs[j].vertBufGLID);
                glDeleteBuffers(1, &layer->spriteBatchQuadBufs[j].elemBufGLID);
            }
        }

        zero_out(renderer);
    }

    void render_all(const Renderer* const renderer, const ShaderProgs* const shaderProgs) {
        glClearColor(renderer->bgColor.r, renderer->bgColor.g, renderer->bgColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        static int texUnits[gk_texUnitLimit];
        static bool texUnitsInitialized = false;

        if (!texUnitsInitialized) {
            for (int i = 0; i < gk_texUnitLimit; ++i) {
                texUnits[i] = i;
            }

            texUnitsInitialized = true;
        }

        Matrix4x4 projMat = {};
        init_ortho_matrix_4x4(&projMat, 0.0f, get_window_size().x, get_window_size().y, 0.0f, -1.0f, 1.0f);

        Matrix4x4 camViewMat = {};
        init_camera_view_matrix(&camViewMat, &renderer->cam);

        Matrix4x4 defaultViewMat = {};
        init_identity_matrix_4x4(&defaultViewMat);

        for (int i = 0; i < renderer->layerCnt; ++i) {
            //
            // Sprite Batches
            //
            glUseProgram(shaderProgs->spriteQuad.glID);

            glUniformMatrix4fv(shaderProgs->spriteQuad.projUniLoc, 1, false, reinterpret_cast<const float*>(projMat.elems));

            const Matrix4x4* const viewMat = i < renderer->camLayerCnt ? &camViewMat : &defaultViewMat;
            glUniformMatrix4fv(shaderProgs->spriteQuad.viewUniLoc, 1, false, reinterpret_cast<const float*>(viewMat->elems));

            glUniform1iv(shaderProgs->spriteQuad.texturesUniLoc, gk_texUnitLimit, texUnits);

            const RenderLayer* const layer = &renderer->layers[i];

            for (int j = 0; j < layer->props.spriteBatchCnt; ++j) {
                const QuadBuf* const batchQuadBuf = &layer->spriteBatchQuadBufs[j];
                const SpriteBatchTransients* const batchTransData = &layer->spriteBatchTransients[j];

                glBindVertexArray(batchQuadBuf->vertArrayGLID);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchQuadBuf->elemBufGLID);

                for (int k = 0; k < batchTransData->texUnitsInUse; ++k) {
                    glActiveTexture(GL_TEXTURE0 + k);
                    glBindTexture(GL_TEXTURE_2D, get_textures()->glIDs[batchTransData->texUnitTexIDs[k]]);
                }

                glDrawElements(GL_TRIANGLES, 6 * batchTransData->slotsUsed, GL_UNSIGNED_SHORT, nullptr);
            }

            //
            // Character Batches
            //
            glUseProgram(shaderProgs->charQuad.glID);

            glUniformMatrix4fv(shaderProgs->charQuad.projUniLoc, 1, false, (const float*)projMat.elems);
            glUniformMatrix4fv(shaderProgs->charQuad.viewUniLoc, 1, false, (const float*)viewMat->elems);

            for (int j = 0; j < layer->props.charBatchCnt; ++j) {
                if (!is_bit_active(layer->charBatchActivityBitset, j)) {
                    continue;
                }

                const CharBatch* const batch = &layer->charBatches[j];

                glUniform2fv(shaderProgs->charQuad.posUniLoc, 1, (const float*)&batch->displayProps.pos);
                glUniform1f(shaderProgs->charQuad.rotUniLoc, batch->displayProps.rot);
                glUniform4fv(shaderProgs->charQuad.blendUniLoc, 1, (const float*)&batch->displayProps.blend);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, get_fonts()->texGLIDs[batch->displayProps.fontIndex]);

                glBindVertexArray(batch->quadBuf.vertArrayGLID);
                glDrawElements(GL_TRIANGLES, 6 * batch->slotCnt, GL_UNSIGNED_SHORT, nullptr);
            }
        }
    }

    void empty_sprite_batches(Renderer* const renderer) {
        for (int i = 0; i < renderer->layerCnt; ++i) {
            RenderLayer* const layer = &renderer->layers[i];
            zero_out(layer->spriteBatchTransients, layer->props.spriteBatchCnt);
            layer->spriteBatchesFilled = 0;
        }
    }

    void write_to_sprite_batch(Renderer* const renderer, const int layerIndex, const SpriteBatchWriteInfo* const info) {
        assert(layerIndex >= 0 && layerIndex < renderer->layerCnt);

        RenderLayer* const layer = &renderer->layers[layerIndex];

        const int batchIndex = layer->spriteBatchesFilled;
        SpriteBatchTransients* const batchTransData = &layer->spriteBatchTransients[batchIndex];

        int texUnit;

        if (batchTransData->slotsUsed == gk_spriteBatchSlotLimit || (texUnit = add_tex_unit_to_sprite_batch(batchTransData, info->texIndex)) == -1) {
            ++layer->spriteBatchesFilled;

            if (layer->spriteBatchesFilled < layer->props.spriteBatchCnt) {
                write_to_sprite_batch(renderer, layerIndex, info);
            } else {
                assert(false);
            }

            return;
        }

        const int slotIndex = batchTransData->slotsUsed;
        const Pt2D texSize = get_textures()->sizes[info->texIndex];

        const float verts[] = {
            (0.0f - info->origin.x) * info->scale.x,
            (0.0f - info->origin.y) * info->scale.y,
            info->pos.x,
            info->pos.y,
            static_cast<float>(info->srcRect.width),
            static_cast<float>(info->srcRect.height),
            info->rot,
            static_cast<float>(texUnit),
            static_cast<float>(info->srcRect.x) / texSize.x,
            static_cast<float>(info->srcRect.y) / texSize.y,
            info->alpha,

            (1.0f - info->origin.x) * info->scale.x,
            (0.0f - info->origin.y) * info->scale.y,
            info->pos.x,
            info->pos.y,
            static_cast<float>(info->srcRect.width),
            static_cast<float>(info->srcRect.height),
            info->rot,
            static_cast<float>(texUnit),
            static_cast<float>(info->srcRect.x + info->srcRect.width) / texSize.x,
            static_cast<float>(info->srcRect.y) / texSize.y,
            info->alpha,

            (1.0f - info->origin.x) * info->scale.x,
            (1.0f - info->origin.y) * info->scale.y,
            info->pos.x,
            info->pos.y,
            static_cast<float>(info->srcRect.width),
            static_cast<float>(info->srcRect.height),
            info->rot,
            static_cast<float>(texUnit),
            static_cast<float>(info->srcRect.x + info->srcRect.width) / texSize.x,
            static_cast<float>(info->srcRect.y + info->srcRect.height) / texSize.y,
            info->alpha,

            (0.0f - info->origin.x) * info->scale.x,
            (1.0f - info->origin.y) * info->scale.y,
            info->pos.x,
            info->pos.y,
            static_cast<float>(info->srcRect.width),
            static_cast<float>(info->srcRect.height),
            info->rot,
            static_cast<float>(texUnit),
            static_cast<float>(info->srcRect.x) / texSize.x,
            static_cast<float>(info->srcRect.y + info->srcRect.height) / texSize.y,
            info->alpha
        };

        const QuadBuf* const batchQuadBuf = &layer->spriteBatchQuadBufs[batchIndex];
        glBindVertexArray(batchQuadBuf->vertArrayGLID);
        glBindBuffer(GL_ARRAY_BUFFER, batchQuadBuf->vertBufGLID);
        glBufferSubData(GL_ARRAY_BUFFER, slotIndex * sizeof(float) * ik_spriteBatchSlotVertCnt, sizeof(verts), verts);

        ++batchTransData->slotsUsed;
    }

    CharBatchID activate_any_char_batch(Renderer* const renderer, const int layerIndex, const int slotCnt, const int fontIndex) {
        assert(layerIndex >= 0 && layerIndex < renderer->layerCnt);
        assert(slotCnt > 0 && slotCnt < gk_charBatchSlotLimit);

        RenderLayer* const layer = &renderer->layers[layerIndex];

        const int batchIndex = get_first_inactive_bit_index(layer->charBatchActivityBitset, layer->props.charBatchCnt);
        assert(batchIndex != -1);

        activate_bit(layer->charBatchActivityBitset, batchIndex);

        layer->charBatches[batchIndex] = {
            .quadBuf = gen_quad_buf(slotCnt, false),
            .slotCnt = slotCnt
        };

        layer->charBatches[batchIndex].displayProps = {
            .fontIndex = fontIndex,
            .blend = {1.0f, 1.0f, 1.0f, 1.0f}
        };

        return {
            .layerIndex = layerIndex,
            .batchIndex = batchIndex
        };
    }

    void deactivate_char_batch(Renderer* const renderer, const CharBatchID id) {
        RenderLayer* const layer = &renderer->layers[id.layerIndex];
        deactivate_bit(layer->charBatchActivityBitset, id.batchIndex);
    }

    void write_to_char_batch(Renderer* const renderer, const CharBatchID id, const char* const text, const FontHorAlign horAlign, const FontVerAlign verAlign) {
        RenderLayer* const layer = &renderer->layers[id.layerIndex];
        CharBatch* const batch = &layer->charBatches[id.batchIndex];

        const int textLen = static_cast<int>(strlen(text));
        assert(textLen > 0 && textLen <= batch->slotCnt);

        const FontArrangementInfo* const fontArrangementInfo = &get_fonts()->arrangementInfos[batch->displayProps.fontIndex];
        const Pt2D fontTexSize = get_fonts()->texSizes[batch->displayProps.fontIndex];

        Pt2D charDrawPositions[gk_charBatchSlotLimit];
        Pt2D charDrawPosPen = {};

        int textLineWidths[gk_charBatchSlotLimit + 1];
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
                charDrawPosPen.x = 0;
                charDrawPosPen.y += fontArrangementInfo->lineHeight;
                continue;
            }

            const int textCharIndex = text[i] - gk_fontCharRangeBegin;

            if (textLineCnter == 0) {
                if (!textFirstLineMinOffsUpdated) {
                    textFirstLineMinOffs = fontArrangementInfo->chars.verOffsets[textCharIndex];
                    textFirstLineMinOffsUpdated = true;
                } else {
                    textFirstLineMinOffs = std::min(fontArrangementInfo->chars.verOffsets[textCharIndex], textFirstLineMinOffs);
                }
            }

            if (!textLastLineMaxHeightUpdated) {
                textLastLineMaxHeight = fontArrangementInfo->chars.verOffsets[textCharIndex] + fontArrangementInfo->chars.srcRects[textCharIndex].height;
                textLastLineMaxHeightUpdated = true;
            } else {
                textLastLineMaxHeight = std::max(fontArrangementInfo->chars.verOffsets[textCharIndex] + fontArrangementInfo->chars.srcRects[textCharIndex].height, textLastLineMaxHeight);
            }

            if (i > 0) {
                const int textCharIndexLast = text[i - 1] - gk_fontCharRangeBegin;
                charDrawPosPen.x += fontArrangementInfo->chars.kernings[(textCharIndex * gk_fontCharRangeLen) + textCharIndexLast];
            }

            charDrawPositions[i].x = charDrawPosPen.x + fontArrangementInfo->chars.horOffsets[textCharIndex];
            charDrawPositions[i].y = charDrawPosPen.y + fontArrangementInfo->chars.verOffsets[textCharIndex];

            charDrawPosPen.x += fontArrangementInfo->chars.horAdvances[textCharIndex];
        }

        textLineWidths[textLineCnter] = charDrawPosPen.x;
        textLineCnter = 0;

        const int textHeight = textFirstLineMinOffs + charDrawPosPen.y + textLastLineMaxHeight;

        clear_char_batch(renderer, id);

        const int vertsLen = ik_charBatchSlotVertsCnt * textLen;
        const auto verts = alloc_zeroed<float>(vertsLen);

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

            const int charIndex = text[i] - gk_fontCharRangeBegin;

            const Vec2D charDrawPos = {
                charDrawPositions[i].x - (textLineWidths[textLineCnter] * horAlign * 0.5f),
                charDrawPositions[i].y - (textHeight * verAlign * 0.5f)
            };

            const Vec2D charTexCoordsTopLeft = {
                static_cast<float>(fontArrangementInfo->chars.srcRects[charIndex].x) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo->chars.srcRects[charIndex].y) / fontTexSize.y
            };

            const Vec2D charTexCoordsBottomRight = {
                static_cast<float>(get_rect_right(fontArrangementInfo->chars.srcRects[charIndex])) / fontTexSize.x,
                static_cast<float>(get_rect_bottom(fontArrangementInfo->chars.srcRects[charIndex])) / fontTexSize.y
            };

            float* const slotVerts = verts + (i * ik_charBatchSlotVertsCnt);

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

    void clear_char_batch(const Renderer* const renderer, const CharBatchID id) {
        const RenderLayer* const layer = &renderer->layers[id.layerIndex];
        const CharBatch* const batch = &layer->charBatches[id.batchIndex];

        glBindVertexArray(batch->quadBuf.vertArrayGLID);
        glBindBuffer(GL_ARRAY_BUFFER, batch->quadBuf.vertBufGLID);
        glBufferData(GL_ARRAY_BUFFER, ik_charBatchSlotVertsSize * batch->slotCnt, nullptr, GL_DYNAMIC_DRAW);
    }
}
