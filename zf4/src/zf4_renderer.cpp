#include <zf4_renderer.h>

#include <algorithm>
#include <zf4_window.h>
#include <zf4_assets.h>

namespace zf4 {
    bool Renderer::init(MemArena* const memArena) {
        assert(is_zero(this));

        //
        for (int i = 0; i < gk_texUnitLimit; ++i) {
            m_texUnits[i] = i;
        }

        // Reserve memory for batch data.
        m_batchPermDatas = memArena->push<RenderBatchPermData>(gk_renderBatchLimit);

        if (!m_batchPermDatas) {
            return false;
        }

        m_batchTransDatas = memArena->push<RenderBatchTransientData>(gk_renderBatchLimit);

        if (!m_batchTransDatas) {
            return false;
        }

        //
        if (!m_viewMatrixInfos.init(memArena, gk_viewMatrixLimit)) {
            return false;
        }

        // Initialise indices.
        const auto indices = alloc<unsigned short>(6 * gk_renderBatchSlotLimit);

        if (!indices) {
            return false;
        }

        for (int i = 0; i < gk_renderBatchSlotLimit; i++) {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        // Initialise batches.
        for (int i = 0; i < gk_renderBatchLimit; ++i) {
            RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];

            // Generate vertex array.
            glGenVertexArrays(1, &batchPermData->vertArrayGLID);
            glBindVertexArray(batchPermData->vertArrayGLID);

            // Generate vertex buffer.
            glGenBuffers(1, &batchPermData->vertBufGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batchPermData->vertBufGLID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gk_texturedQuadShaderProgVertCnt * 4 * gk_renderBatchSlotLimit, nullptr, GL_DYNAMIC_DRAW);

            // Generate element buffer.
            glGenBuffers(1, &batchPermData->elemBufGLID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchPermData->elemBufGLID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6 * gk_renderBatchSlotLimit, indices, GL_STATIC_DRAW);

            // Set vertex attribute pointers.
            const int vertsStride = sizeof(float) * gk_texturedQuadShaderProgVertCnt;

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

            glBindVertexArray(0);
        }

        free(indices);

        m_initialized = true;

        return true;
    }

    void Renderer::clean() {
        for (int i = 0; i < gk_renderBatchLimit; ++i) {
            glDeleteVertexArrays(1, &m_batchPermDatas[i].vertArrayGLID);
            glDeleteBuffers(1, &m_batchPermDatas[i].vertBufGLID);
            glDeleteBuffers(1, &m_batchPermDatas[i].elemBufGLID);
        }

        zero_out(this);
    }

    void Renderer::render(const Vec3D& bgColor, const ShaderProgs* const shaderProgs) {
        assert(m_initialized);
        assert(!m_inWriteup);

        // Set the background.
        glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Prepare batch rendering.
        glUseProgram(shaderProgs->texturedQuad.glID);

        const Matrix4x4 projMat = Matrix4x4::create_ortho(0.0f, static_cast<float>(get_window_size().x), static_cast<float>(get_window_size().y), 0.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(shaderProgs->texturedQuad.projUniLoc, 1, false, reinterpret_cast<const float*>(&projMat));

        const Matrix4x4 initViewMat = Matrix4x4::create_identity();
        glUniformMatrix4fv(shaderProgs->texturedQuad.viewUniLoc, 1, false, reinterpret_cast<const float*>(&initViewMat));

        glUniform1iv(shaderProgs->texturedQuad.texturesUniLoc, gk_texUnitLimit, m_texUnits);

        // Render batches.
        int viewMatIndex = 0;

        for (int i = 0; i <= m_batchWriteIndex; ++i) {
            if (viewMatIndex < m_viewMatrixInfos.get_len() && m_viewMatrixInfos[viewMatIndex].beginBatchIndex == i) {
                const Matrix4x4& viewMat = m_viewMatrixInfos[viewMatIndex].mat;
                glUniformMatrix4fv(shaderProgs->texturedQuad.viewUniLoc, 1, false, reinterpret_cast<const float*>(viewMat.elems));
                ++viewMatIndex;
            }

            const RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];
            const RenderBatchTransientData* const batchTransData = &m_batchTransDatas[i];

            for (int j = 0; j < batchTransData->texUnitsInUseCnt; ++j) {
                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, batchTransData->texUnitTexGLIDs[j]);
            }

            glBindVertexArray(batchPermData->vertArrayGLID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchPermData->elemBufGLID);
            glDrawElements(GL_TRIANGLES, 6 * batchTransData->slotsUsedCnt, GL_UNSIGNED_SHORT, nullptr);
        }
    }

    void Renderer::begin_writeup() {
        assert(m_initialized);
        assert(!m_inWriteup);

        zero_out(m_batchTransDatas, m_batchWriteIndex + 1);
        m_batchWriteIndex = 0;

        m_viewMatrixInfos.clear();

        m_inWriteup = true;
    }

    void Renderer::end_writeup() {
        assert(m_initialized);
        assert(m_inWriteup);

        // Submit vertex data to GPU.
        for (int i = 0; i <= m_batchWriteIndex; ++i) {
            const RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];
            const RenderBatchTransientData* const batchTransData = &m_batchTransDatas[i];

            glBindVertexArray(batchPermData->vertArrayGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batchPermData->vertBufGLID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batchTransData->verts), batchTransData->verts);
        }

        m_inWriteup = false;
    }

    void Renderer::write_texture(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin, const float rot, const Vec2D scale, const float alpha) {
        assert(m_initialized);
        assert(m_inWriteup);

        const Vec2DI texSize = get_textures()->sizes[texIndex];
        const Rect texCoords = {
            static_cast<float>(srcRect.x) / texSize.x,
            static_cast<float>(srcRect.y) / texSize.y,
            static_cast<float>(srcRect.width) / texSize.x,
            static_cast<float>(srcRect.height) / texSize.y
        };
        write(origin, scale, pos, get_rect_size(srcRect), rot, get_textures()->glIDs[texIndex], texCoords, alpha);
    }

    void Renderer::write_str(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign, const StrVerAlign verAlign) {
        assert(m_initialized);
        assert(m_inWriteup);

        const int strLen = static_cast<int>(strlen(str));
        assert(strLen > 0);

        const auto charDrawPositions = scratchSpace->push<Vec2DI>(strLen);

        if (!charDrawPositions) {
            return;
        }

        Vec2DI charDrawPosPen = {}; // The top-left position of the character being drawn (not relative to the given text write position).

        const auto strLineWidths = scratchSpace->push<int>(strLen + 1);

        if (!strLineWidths) {
            return;
        }

        int strFirstLineMinOffs = 0;
        bool strFirstLineMinOffsUpdated = false;
        int strLastLineMaxHeight = 0;
        bool strLastLineMaxHeightUpdated = false;
        int strLineCnter = 0;

        const FontArrangementInfo* const fontArrangementInfo = &get_fonts()->arrangementInfos[fontIndex];

        for (int i = 0; i < strLen; i++) {
            if (str[i] == '\n') {
                strLineWidths[strLineCnter] = charDrawPosPen.x;

                if (!strFirstLineMinOffsUpdated) {
                    strFirstLineMinOffs = fontArrangementInfo->chars.verOffsets[0];
                    strFirstLineMinOffsUpdated = true;
                }

                strLastLineMaxHeight = fontArrangementInfo->chars.verOffsets[0] + fontArrangementInfo->chars.srcRects[0].height;
                strLastLineMaxHeightUpdated = false;

                strLineCnter++;
                charDrawPosPen.x = 0;
                charDrawPosPen.y += fontArrangementInfo->lineHeight;
                continue;
            }

            const int strCharIndex = str[i] - gk_fontCharRangeBegin;

            if (strLineCnter == 0) {
                if (!strFirstLineMinOffsUpdated) {
                    strFirstLineMinOffs = fontArrangementInfo->chars.verOffsets[strCharIndex];
                    strFirstLineMinOffsUpdated = true;
                } else {
                    strFirstLineMinOffs = std::min(fontArrangementInfo->chars.verOffsets[strCharIndex], strFirstLineMinOffs);
                }
            }

            if (!strLastLineMaxHeightUpdated) {
                strLastLineMaxHeight = fontArrangementInfo->chars.verOffsets[strCharIndex] + fontArrangementInfo->chars.srcRects[strCharIndex].height;
                strLastLineMaxHeightUpdated = true;
            } else {
                strLastLineMaxHeight = std::max(fontArrangementInfo->chars.verOffsets[strCharIndex] + fontArrangementInfo->chars.srcRects[strCharIndex].height, strLastLineMaxHeight);
            }

            if (i > 0) {
                const int strCharIndexLast = str[i - 1] - gk_fontCharRangeBegin;
                charDrawPosPen.x += fontArrangementInfo->chars.kernings[(strCharIndex * gk_fontCharRangeLen) + strCharIndexLast];
            }

            charDrawPositions[i].x = charDrawPosPen.x + fontArrangementInfo->chars.horOffsets[strCharIndex];
            charDrawPositions[i].y = charDrawPosPen.y + fontArrangementInfo->chars.verOffsets[strCharIndex];

            charDrawPosPen.x += fontArrangementInfo->chars.horAdvances[strCharIndex];
        }

        strLineWidths[strLineCnter] = charDrawPosPen.x;

        // Write the characters.
        const int strHeight = strFirstLineMinOffs + charDrawPosPen.y + strLastLineMaxHeight;
        const Vec2DI fontTexSize = get_fonts()->texSizes[fontIndex];

        strLineCnter = 0;

        for (int i = 0; i < strLen; i++) {
            if (str[i] == '\n') {
                strLineCnter++;
                continue;
            }

            if (str[i] == ' ') {
                continue;
            }

            const int charIndex = str[i] - gk_fontCharRangeBegin;

            const Vec2D charPos = {
                pos.x + charDrawPositions[i].x - (strLineWidths[strLineCnter] * horAlign * 0.5f),
                pos.y + charDrawPositions[i].y - (strHeight * verAlign * 0.5f)
            };

            const Vec2D charSize = get_rect_size(fontArrangementInfo->chars.srcRects[charIndex]);

            const Rect charTexCoords = {
                static_cast<float>(fontArrangementInfo->chars.srcRects[charIndex].x) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo->chars.srcRects[charIndex].y) / fontTexSize.y,
                static_cast<float>(fontArrangementInfo->chars.srcRects[charIndex].width) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo->chars.srcRects[charIndex].height) / fontTexSize.y
            };

            write({}, {1.0f, 1.0f}, charPos, charSize, 0.0f, get_fonts()->texGLIDs[fontIndex], charTexCoords, 1.0f);
        }
    }

    void Renderer::update_writeup_view_matrix(const Matrix4x4& mat) {
        assert(m_initialized);
        assert(m_inWriteup);
        assert(m_batchWriteIndex < gk_renderBatchLimit - 1);

        ++m_batchWriteIndex;

        const RendererViewMatrixInfo info = {
            .mat = mat,
            .beginBatchIndex = m_batchWriteIndex
        };

        m_viewMatrixInfos.add(info);
    }

    int Renderer::add_tex_unit_to_batch(RenderBatchTransientData* const batchTransData, const GLuint glID) {
        // Check if the texture GL ID is already in use.
        for (int i = 0; i < batchTransData->texUnitsInUseCnt; ++i) {
            if (batchTransData->texUnitTexGLIDs[i] == glID) {
                return i;
            }
        }

        // Check if we can't support another texture.
        if (batchTransData->texUnitsInUseCnt == gk_texUnitLimit) {
            return -1;
        }

        // Use a new texture unit, since the texture GL ID is not in use.
        batchTransData->texUnitTexGLIDs[batchTransData->texUnitsInUseCnt] = glID;
        return batchTransData->texUnitsInUseCnt++;
    }

    void Renderer::write(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha) {
        assert(m_initialized);
        assert(m_inWriteup);

        RenderBatchTransientData* const batchTransData = &m_batchTransDatas[m_batchWriteIndex];

        // Check if the batch is full or if it cannot support the texture.
        int texUnit;

        if (batchTransData->slotsUsedCnt == gk_renderBatchSlotLimit || (texUnit = add_tex_unit_to_batch(batchTransData, texGLID)) == -1) {
            if (m_batchWriteIndex < gk_renderBatchLimit - 1) {
                // Move to a new batch.
                ++m_batchWriteIndex;

                // The new batch should be empty.
                assert(is_zero(&m_batchTransDatas[m_batchWriteIndex]));

                // Perform this write but on the new batch.
                write(origin, scale, pos, size, rot, texGLID, texCoords, alpha);
            } else {
                // No more batches to use!
                assert(false);
            }

            return;
        }

        // Write the slot vertex data.
        const int slotIndex = batchTransData->slotsUsedCnt;
        float* const slotVerts = &batchTransData->verts[slotIndex * gk_renderBatchSlotVertCnt];

        slotVerts[0] = (0.0f - origin.x) * scale.x;
        slotVerts[1] = (0.0f - origin.y) * scale.y;
        slotVerts[2] = pos.x;
        slotVerts[3] = pos.y;
        slotVerts[4] = size.x;
        slotVerts[5] = size.y;
        slotVerts[6] = rot;
        slotVerts[7] = static_cast<float>(texUnit);
        slotVerts[8] = texCoords.x;
        slotVerts[9] = texCoords.y;
        slotVerts[10] = alpha;

        slotVerts[11] = (1.0f - origin.x) * scale.x;
        slotVerts[12] = (0.0f - origin.y) * scale.y;
        slotVerts[13] = pos.x;
        slotVerts[14] = pos.y;
        slotVerts[15] = size.x;
        slotVerts[16] = size.y;
        slotVerts[17] = rot;
        slotVerts[18] = static_cast<float>(texUnit);
        slotVerts[19] = get_rect_right(texCoords);
        slotVerts[20] = texCoords.y;
        slotVerts[21] = alpha;

        slotVerts[22] = (1.0f - origin.x) * scale.x;
        slotVerts[23] = (1.0f - origin.y) * scale.y;
        slotVerts[24] = pos.x;
        slotVerts[25] = pos.y;
        slotVerts[26] = size.x;
        slotVerts[27] = size.y;
        slotVerts[28] = rot;
        slotVerts[29] = static_cast<float>(texUnit);
        slotVerts[30] = get_rect_right(texCoords);
        slotVerts[31] = get_rect_bottom(texCoords);
        slotVerts[32] = alpha;

        slotVerts[33] = (0.0f - origin.x) * scale.x;
        slotVerts[34] = (1.0f - origin.y) * scale.y;
        slotVerts[35] = pos.x;
        slotVerts[36] = pos.y;
        slotVerts[37] = size.x;
        slotVerts[38] = size.y;
        slotVerts[39] = rot;
        slotVerts[40] = static_cast<float>(texUnit);
        slotVerts[41] = texCoords.x;
        slotVerts[42] = get_rect_bottom(texCoords);
        slotVerts[43] = alpha;

        ++batchTransData->slotsUsedCnt;
    }
}
