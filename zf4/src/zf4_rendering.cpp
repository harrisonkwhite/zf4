#include <zf4_rendering.h>

#include <algorithm>
#include <zf4_window.h>

namespace zf4 {
    static bool init_and_attach_framebuffer_tex(GLuint* const texGLID, const GLuint fbGLID, const Vec2DI texSize) {
        assert(*texGLID == 0);

        glGenTextures(1, texGLID);
        glBindTexture(GL_TEXTURE_2D, *texGLID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.x, texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, fbGLID);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texGLID, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glDeleteTextures(1, texGLID);
            *texGLID = 0;
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    static bool init_render_surface(RenderSurface* const surf, const Vec2DI size) {
        glGenFramebuffers(1, &surf->framebufferGLID);

        if (!init_and_attach_framebuffer_tex(&surf->framebufferTexGLID, surf->framebufferGLID, size)) {
            glDeleteFramebuffers(1, &surf->framebufferGLID);
            return false;
        }

        return true;
    }

    static void clean_render_surface(RenderSurface* const surf) {
        glDeleteFramebuffers(1, &surf->framebufferGLID);
        glDeleteTextures(1, &surf->framebufferTexGLID);

        zero_out(surf);
    }

    static bool resize_render_surface(RenderSurface* const surf, const Vec2DI size) {
        // Delete the old texture.
        glDeleteTextures(1, &surf->framebufferTexGLID);
        surf->framebufferTexGLID = 0;

        // Generate a new texture of the desired size and attach it to the framebuffer.
        if (!init_and_attach_framebuffer_tex(&surf->framebufferTexGLID, surf->framebufferGLID, size)) {
            return false;
        }

        return false;
    }

    static int add_tex_unit_to_render_batch(RenderBatchTransientData* const batchTransData, const GLuint glID) {
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

    bool attempt_render_batch_submission(RenderBatchTransientData* const batchTransData, const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect srcRect, const float alpha) {
        // Check if the batch is full or if it cannot support the texture.
        int texUnit;

        if (batchTransData->slotsUsedCnt == gk_renderBatchSlotLimit || (texUnit = add_tex_unit_to_render_batch(batchTransData, texGLID)) == -1) {
            return false;
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
        slotVerts[8] = srcRect.x;
        slotVerts[9] = srcRect.y;
        slotVerts[10] = alpha;

        slotVerts[11] = (1.0f - origin.x) * scale.x;
        slotVerts[12] = (0.0f - origin.y) * scale.y;
        slotVerts[13] = pos.x;
        slotVerts[14] = pos.y;
        slotVerts[15] = size.x;
        slotVerts[16] = size.y;
        slotVerts[17] = rot;
        slotVerts[18] = static_cast<float>(texUnit);
        slotVerts[19] = get_rect_right(srcRect);
        slotVerts[20] = srcRect.y;
        slotVerts[21] = alpha;

        slotVerts[22] = (1.0f - origin.x) * scale.x;
        slotVerts[23] = (1.0f - origin.y) * scale.y;
        slotVerts[24] = pos.x;
        slotVerts[25] = pos.y;
        slotVerts[26] = size.x;
        slotVerts[27] = size.y;
        slotVerts[28] = rot;
        slotVerts[29] = static_cast<float>(texUnit);
        slotVerts[30] = get_rect_right(srcRect);
        slotVerts[31] = get_rect_bottom(srcRect);
        slotVerts[32] = alpha;

        slotVerts[33] = (0.0f - origin.x) * scale.x;
        slotVerts[34] = (1.0f - origin.y) * scale.y;
        slotVerts[35] = pos.x;
        slotVerts[36] = pos.y;
        slotVerts[37] = size.x;
        slotVerts[38] = size.y;
        slotVerts[39] = rot;
        slotVerts[40] = static_cast<float>(texUnit);
        slotVerts[41] = srcRect.x;
        slotVerts[42] = get_rect_bottom(srcRect);
        slotVerts[43] = alpha;

        ++batchTransData->slotsUsedCnt;

        return true;
    }

    static bool load_str_render_info(StrRenderInfo* const renderInfo, MemArena* const memArena, const char* const str, const int fontIndex, const Assets& assets) {
        const auto strLen = static_cast<int>(strlen(str));
        assert(strLen > 0);

        const FontArrangementInfo& fontArrangementInfo = assets.get_font_arrangement_info(fontIndex);

        // Reserve memory for some render information.
        renderInfo->charDrawPositions = memArena->push<Vec2DI>(strLen);

        if (!renderInfo->charDrawPositions) {
            return false;
        }

        renderInfo->lineWidths = memArena->push<int>(strLen + 1); // Maximised for the case where all characters are newlines.

        if (!renderInfo->lineWidths) {
            return false;
        }

        // Iterate through each character and set draw positions, meanwhile determining line widths, the overall text height, etc.
        const int spaceCharIndex = ' ' - gk_fontCharRangeBegin; // We use this character for defaulting in some cases.

        Vec2DI charDrawPosPen = {};

        int lineIndex = 0;

        int firstLineMinVerOffs;
        bool firstLineMinVerOffsDefined = false;

        const int lastLineMaxHeightDefault = fontArrangementInfo.chars.verOffsets[spaceCharIndex] + fontArrangementInfo.chars.srcRects[spaceCharIndex].height;
        int lastLineMaxHeight = lastLineMaxHeightDefault;
        bool lastLineMaxHeightUpdated = false; // We want to let the max height initially be overwritten regardless of the default.

        for (int i = 0; i < strLen; i++) {
            const int charIndex = str[i] - gk_fontCharRangeBegin;

            if (str[i] == '\n') {
                renderInfo->lineWidths[lineIndex] = charDrawPosPen.x; // The width of this line is where we've horizontally drawn up to.

                // Reset the last line max height (this didn't turn out to be the last line).
                lastLineMaxHeight = lastLineMaxHeightDefault;
                lastLineMaxHeightUpdated = false; // Let the max height be overwritten by the first character of the next line.

                // Move the pen to the next line.
                charDrawPosPen.x = 0;
                charDrawPosPen.y += fontArrangementInfo.lineHeight;

                ++lineIndex;

                continue;
            }

            // For all characters after the first, apply kerning.
            if (i > 0) {
                const int strCharIndexLast = str[i - 1] - gk_fontCharRangeBegin;
                const int kerningIndex = (charIndex * gk_fontCharRangeLen) + strCharIndexLast;
                charDrawPosPen.x += fontArrangementInfo.chars.kernings[kerningIndex];
            }

            // Set the top-left position to draw this character.
            renderInfo->charDrawPositions[i].x = charDrawPosPen.x + fontArrangementInfo.chars.horOffsets[charIndex];
            renderInfo->charDrawPositions[i].y = charDrawPosPen.y + fontArrangementInfo.chars.verOffsets[charIndex];

            // Move to the next character.
            charDrawPosPen.x += fontArrangementInfo.chars.horAdvances[charIndex];

            // If this is the first line, update the minimum vertical offset.
            if (lineIndex == 0) {
                if (!firstLineMinVerOffsDefined) {
                    firstLineMinVerOffs = fontArrangementInfo.chars.verOffsets[charIndex];
                    firstLineMinVerOffsDefined = true;
                } else {
                    firstLineMinVerOffs = std::min(fontArrangementInfo.chars.verOffsets[charIndex], firstLineMinVerOffs);
                }
            }

            // Update the maximum height. Note that we aren't sure if this is the last line.
            const int height = fontArrangementInfo.chars.verOffsets[charIndex] + fontArrangementInfo.chars.srcRects[charIndex].height;

            if (!lastLineMaxHeightUpdated) {
                lastLineMaxHeight = height;
                lastLineMaxHeightUpdated = true;
            } else {
                lastLineMaxHeightUpdated = std::max(height, lastLineMaxHeight);
            }
        }

        // Set the width of the final line.
        renderInfo->lineWidths[lineIndex] = charDrawPosPen.x;

        // If the minimum vertical offset of the first line wasn't set (because the first line was empty), just use the space character.
        // Note that we don't want this vertical offset to affect the minimum calculation, hence why it was not assigned as default.
        if (!firstLineMinVerOffsDefined) {
            firstLineMinVerOffs = fontArrangementInfo.chars.verOffsets[spaceCharIndex];
            firstLineMinVerOffsDefined = true;
        }

        renderInfo->height = firstLineMinVerOffs + charDrawPosPen.y + lastLineMaxHeight;

        return true;
    }

    bool Renderer::init(MemArena* const memArena, const int batchLimit, const int batchLifeMax) {
        assert(is_zero(this));

        //
        // Surfaces
        //
        if (!m_surfs.init(memArena, gk_renderSurfaceLimit)) {
            return false;
        }

        glGenVertexArrays(1, &m_surfVertArrayGLID);
        glBindVertexArray(m_surfVertArrayGLID);

        glGenBuffers(1, &m_surfVertBufGLID);
        glBindBuffer(GL_ARRAY_BUFFER, m_surfVertBufGLID);

        {
            const float verts[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 1.0f
            };

            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        }

        glGenBuffers(1, &m_surfElemBufGLID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_surfElemBufGLID);

        {
            const unsigned short indices[] = {0, 1, 2, 2, 3, 0};
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        }

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 0));

        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        //
        // Batches
        //
        m_batchLimit = batchLimit;
        m_batchLifeMax = batchLifeMax;

        // Reserve memory for batch data.
        m_batchPermDatas = memArena->push<RenderBatchPermData>(batchLimit);

        if (!m_batchPermDatas) {
            return false;
        }

        m_batchTransDatas = memArena->push<RenderBatchTransientData>(batchLimit);

        if (!m_batchTransDatas) {
            return false;
        }

        m_batchLifes = memArena->push<int>(batchLimit);

        if (!m_batchLifes) {
            return false;
        }

        // Set up texture units.
        for (int i = 0; i < gk_texUnitLimit; ++i) {
            m_texUnits[i] = i;
        }

        // Generate batch indices.
        const int indicesLen = 6 * gk_renderBatchSlotLimit;
        m_batchIndices = memArena->push<unsigned short>(indicesLen);

        if (!m_batchIndices) {
            return false;
        }

        for (int i = 0; i < gk_renderBatchSlotLimit; i++) {
            m_batchIndices[(i * 6) + 0] = static_cast<unsigned short>((i * 4) + 0);
            m_batchIndices[(i * 6) + 1] = static_cast<unsigned short>((i * 4) + 1);
            m_batchIndices[(i * 6) + 2] = static_cast<unsigned short>((i * 4) + 2);
            m_batchIndices[(i * 6) + 3] = static_cast<unsigned short>((i * 4) + 2);
            m_batchIndices[(i * 6) + 4] = static_cast<unsigned short>((i * 4) + 3);
            m_batchIndices[(i * 6) + 5] = static_cast<unsigned short>((i * 4) + 0);
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indicesLen, m_batchIndices, GL_STATIC_DRAW);

        //
        // Instructions
        //
        if (!m_renderInstrs.init(memArena, gk_renderInstrLimit)) {
            return false;
        }

        m_state = RendererState::Initialized;

        return true;
    }

    void Renderer::clean() {
        for (int i = 0; i < m_batchLimit; ++i) {
            glDeleteVertexArrays(1, &m_batchPermDatas[i].vertArrayGLID);
            glDeleteBuffers(1, &m_batchPermDatas[i].vertBufGLID);
            glDeleteBuffers(1, &m_batchPermDatas[i].elemBufGLID);
        }

        glDeleteVertexArrays(1, &m_surfVertArrayGLID);
        glDeleteBuffers(1, &m_surfVertBufGLID);
        glDeleteBuffers(1, &m_surfElemBufGLID);

        for (int i = 0; i < m_surfs.get_len(); ++i) {
            if (m_surfs.is_active(i)) {
                clean_render_surface(&m_surfs[i]);
            }
        }

        zero_out(this);
    }

    RenderSurfaceID Renderer::add_surface() {
        assert(m_state == RendererState::Initialized);

        const int surfIndex = m_surfs.get_first_inactive_index();

        if (surfIndex != -1) {
            init_render_surface(&m_surfs[surfIndex], Window::get_size());
        }

        return surfIndex;
    }

    void Renderer::remove_surface(const RenderSurfaceID surfID) {
        assert(m_state == RendererState::Initialized);

        m_surfs.deactivate(surfID);
        clean_render_surface(&m_surfs[surfID]);
    }

    void Renderer::resize_surfaces() {
        assert(m_state == RendererState::Initialized);

        for (int i = 0; i < m_surfs.get_len(); ++i) {
            if (m_surfs.is_active(i)) {
                resize_render_surface(&m_surfs[i], Window::get_size());
            }
        }
    }

    void Renderer::begin_submission_phase() {
        assert(m_state == RendererState::Initialized);

        m_state = RendererState::Submitting;

        zero_out(m_batchTransDatas, m_batchSubmitIndex + 1);

        m_batchSubmitIndex = 0;

        m_renderInstrs.clear();
    }

    void Renderer::end_submission_phase() {
        assert(m_state == RendererState::Submitting);

        // Submit render batch vertex data to the GPU.
        for (int i = 0; i <= m_batchSubmitIndex; ++i) {
            const RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];
            const RenderBatchTransientData* const batchTransData = &m_batchTransDatas[i];

            glBindVertexArray(batchPermData->vertArrayGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batchPermData->vertBufGLID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batchTransData->verts), batchTransData->verts);
        }

        // Decrement all batch life values and deactivate the batches if necessary.
        for (int i = 0; i < m_batchLimit; ++i) {
            if (m_batchLifes[i] > 0) {
                --m_batchLifes[i];

                if (m_batchLifes[i] == 0) {
                    // Deactivate the batch.
                    glDeleteVertexArrays(1, &m_batchPermDatas[i].vertArrayGLID);
                    glDeleteBuffers(1, &m_batchPermDatas[i].vertBufGLID);
                    glDeleteBuffers(1, &m_batchPermDatas[i].elemBufGLID);
                }
            }
        }

        m_state = RendererState::Initialized;
    }

    bool Renderer::submit_texture(const int texIndex, const Assets& assets, const Vec2D pos, const RectI& srcRect, const Vec2D origin, const float rot, const Vec2D scale, const float alpha) {
        assert(m_state == RendererState::Submitting);

        while (!attempt_render_batch_submission(&m_batchTransDatas[m_batchSubmitIndex], origin, scale, pos, assets.get_tex_size(texIndex), rot, assets.get_tex_gl_id(texIndex), srcRect, alpha)) {
            if (!move_to_next_batch()) {
                return false;
            }
        }

        return true;
    }

    bool Renderer::submit_str(const char* const str, const int fontIndex, const Assets& assets, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign, const StrVerAlign verAlign) {
        const FontArrangementInfo& fontArrangementInfo = assets.get_font_arrangement_info(fontIndex);
        const GLuint fontTexGLID = assets.get_font_tex_gl_id(fontIndex);
        const Vec2DI fontTexSize = assets.get_font_tex_size(fontIndex);

        StrRenderInfo strRenderInfo;

        if (!load_str_render_info(&strRenderInfo, scratchSpace, str, fontIndex, assets)) {
            return false;
        }

        int lineIndex = 0;

        for (int i = 0; str[i]; ++i) {
            if (str[i] == '\n') {
                ++lineIndex;
                continue;
            }

            if (str[i] == ' ') {
                continue;
            }

            const int charIndex = str[i] - gk_fontCharRangeBegin;

            // Note that the character position is offset based on alignment.
            // Middle alignment (1) corresponds to 50% multiplier on current line width and text height, right alignment (2) is 100%.
            const Vec2D charPos = {
                pos.x + strRenderInfo.charDrawPositions[i].x - (strRenderInfo.lineWidths[lineIndex] * horAlign * 0.5f),
                pos.y + strRenderInfo.charDrawPositions[i].y - (strRenderInfo.height * verAlign * 0.5f)
            };

            const Vec2D charSize = get_rect_size(fontArrangementInfo.chars.srcRects[charIndex]);

            const Rect charTexCoords = {
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].x) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].y) / fontTexSize.y,
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].width) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].height) / fontTexSize.y
            };

            while (!attempt_render_batch_submission(&m_batchTransDatas[m_batchSubmitIndex], {}, {1.0f, 1.0f}, charPos, charSize, 0.0f, fontTexGLID, charTexCoords, 1.0f)) {
                if (!move_to_next_batch()) {
                    return false;
                }
            }
        }

        return false;
    }

    bool Renderer::render(const InternalShaderProgs& internalShaderProgs, const Assets& assets, MemArena* const scratchSpace) {
        assert(m_state == RendererState::Initialized);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Set the background to black.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Iterate through and execute render instructions.
        const Matrix4x4 projMat = Matrix4x4::create_ortho(0.0f, static_cast<float>(Window::get_size().x), static_cast<float>(Window::get_size().y), 0.0f, -1.0f, 1.0f);
        Matrix4x4 viewMat = Matrix4x4::create_identity(); // Whenever we draw a batch, the shader view matrix uniform is assigned to whatever this is.

        GLuint surfShaderProgGLID = 0;

        Stack<int> surfIndexes = {};

        if (m_surfs.get_len() > 0) {
            if (!surfIndexes.init(scratchSpace, m_surfs.get_len())) {
                return false;
            }
        }

        for (int i = 0; i < m_renderInstrs.get_len(); ++i) {
            const RenderInstr instr = m_renderInstrs[i];

            switch (instr.type) {
                case RenderInstrType::Clear:
                    glClearColor(instr.data.clear.color.x, instr.data.clear.color.y, instr.data.clear.color.z, instr.data.clear.color.w);
                    glClear(GL_COLOR_BUFFER_BIT);
                    break;

                case RenderInstrType::SetViewMatrix:
                    viewMat = instr.data.setViewMatrix.mat;
                    break;

                case RenderInstrType::DrawBatch:
                    {
                        const int batchIndex = instr.data.drawBatch.index;
                        const RenderBatchTransientData& batchTransData = m_batchTransDatas[batchIndex];
                        const RenderBatchPermData& batchPermData = m_batchPermDatas[batchIndex];

                        glUseProgram(internalShaderProgs.texturedQuad.glID);

                        glUniformMatrix4fv(internalShaderProgs.texturedQuad.projUniLoc, 1, false, reinterpret_cast<const float*>(&projMat));
                        glUniformMatrix4fv(internalShaderProgs.texturedQuad.viewUniLoc, 1, false, reinterpret_cast<const float*>(&viewMat));

                        glUniform1iv(internalShaderProgs.texturedQuad.texturesUniLoc, gk_texUnitLimit, m_texUnits);

                        for (int j = 0; j < batchTransData.texUnitsInUseCnt; ++j) {
                            glActiveTexture(GL_TEXTURE0 + j);
                            glBindTexture(GL_TEXTURE_2D, batchTransData.texUnitTexGLIDs[j]);
                        }

                        glBindVertexArray(batchPermData.vertArrayGLID);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchPermData.elemBufGLID);
                        glDrawElements(GL_TRIANGLES, 6 * batchTransData.slotsUsedCnt, GL_UNSIGNED_SHORT, nullptr);
                    }

                    break;

                case RenderInstrType::SetSurface:
                    surfIndexes.push(instr.data.setSurface.index);
                    glBindFramebuffer(GL_FRAMEBUFFER, m_surfs[surfIndexes.peek()].framebufferGLID);
                    break;

                case RenderInstrType::UnsetSurface:
                    surfIndexes.pop();
                    glBindFramebuffer(GL_FRAMEBUFFER, surfIndexes.is_empty() ? 0 : m_surfs[surfIndexes.peek()].framebufferGLID);
                    break;

                case RenderInstrType::SetSurfaceShaderProg:
                    surfShaderProgGLID = assets.get_shader_prog_gl_id(instr.data.setDrawSurfaceShaderProg.progIndex);
                    break;

                case RenderInstrType::SetSurfaceShaderUniform:
                    {
                        assert(surfShaderProgGLID); // Make sure the surface shader program has been set prior to this instruction.

                        glUseProgram(surfShaderProgGLID);

                        const int uniformLoc = glGetUniformLocation(surfShaderProgGLID, instr.data.setDrawSurfaceShaderUniform.name);
                        assert(uniformLoc != -1 && "Could not find the surface shader uniform with the given name!");

                        const ShaderUniformVal uniformVal = instr.data.setDrawSurfaceShaderUniform.val;

                        switch (instr.data.setDrawSurfaceShaderUniform.valType) {
                            case ShaderUniformValType::Float:
                                glUniform1f(uniformLoc, uniformVal.floatVal);
                                break;

                            case ShaderUniformValType::Int:
                                glUniform1i(uniformLoc, uniformVal.intVal);
                                break;

                            case ShaderUniformValType::Vec2D:
                                glUniform2fv(uniformLoc, 1, reinterpret_cast<const float*>(&uniformVal.vec2DVal));
                                break;

                            case ShaderUniformValType::Vec3D:
                                glUniform3fv(uniformLoc, 1, reinterpret_cast<const float*>(&uniformVal.vec3DVal));
                                break;

                            case ShaderUniformValType::Vec4D:
                                glUniform4fv(uniformLoc, 1, reinterpret_cast<const float*>(&uniformVal.vec4DVal));
                                break;

                            case ShaderUniformValType::Matrix4x4:
                                glUniformMatrix4fv(uniformLoc, 1, false, reinterpret_cast<const float*>(&uniformVal.mat4x4Val));
                                break;
                        }
                    }

                    break;

                case RenderInstrType::DrawSurface:
                    {
                        assert(surfShaderProgGLID); // Make sure the surface shader program has been set prior to this instruction.

                        glUseProgram(surfShaderProgGLID);

                        glActiveTexture(GL_TEXTURE0);
                        const RenderSurface& surf = m_surfs[instr.data.drawSurface.index];
                        glBindTexture(GL_TEXTURE_2D, surf.framebufferTexGLID);

                        glBindVertexArray(m_surfVertArrayGLID);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_surfElemBufGLID);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

                        surfShaderProgGLID = 0; // Reset the surface shader program.
                    }

                    break;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    bool Renderer::move_to_next_batch() {
        if (m_batchSubmitIndex == m_batchLimit - 1) {
            // We're out of batches!
            return false;
        }

        ++m_batchSubmitIndex;

        if (m_batchLifes[m_batchSubmitIndex] == 0) {
            //
            // New Batch Generation
            //
            RenderBatchPermData* const batchPermData = &m_batchPermDatas[m_batchSubmitIndex];

            // Generate vertex array.
            glGenVertexArrays(1, &batchPermData->vertArrayGLID);
            glBindVertexArray(batchPermData->vertArrayGLID);

            // Generate vertex buffer.
            glGenBuffers(1, &batchPermData->vertBufGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batchPermData->vertBufGLID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gk_renderBatchSlotVertCnt * gk_renderBatchSlotLimit, nullptr, GL_DYNAMIC_DRAW);

            // Generate element buffer.
            glGenBuffers(1, &batchPermData->elemBufGLID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchPermData->elemBufGLID);

            const int indicesLen = 6 * gk_renderBatchSlotLimit;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indicesLen, m_batchIndices, GL_STATIC_DRAW);

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

        m_batchLifes[m_batchSubmitIndex] = m_batchLifeMax;

        // Make sure the new batch is zero.
        assert(is_zero(&m_batchTransDatas[m_batchSubmitIndex]));

        return true;
    }

    bool Renderer::submit_instr(const RenderInstrType type, const RenderInstrData data) {
        assert(m_state == RendererState::Submitting);
        assert(!m_renderInstrs.is_full());

        if (m_batchTransDatas[m_batchSubmitIndex].slotsUsedCnt > 0) {
            if (type == RenderInstrType::SetViewMatrix || type == RenderInstrType::SetSurface || type == RenderInstrType::UnsetSurface) {
                if (!move_to_next_batch()) {
                    return false; // NOTE: Error handling for every instruction submission? Seriously?
                }
            }
        }

        m_renderInstrs.push({.type = type, .data = data});

        return true;
    }
}
