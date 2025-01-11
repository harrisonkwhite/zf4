#include <zf4_renderer.h>

#include <algorithm>
#include <zf4_window.h>
#include <zf4_assets.h>

namespace zf4 {
    static Vec2DI get_surface_targ_size() {
        return Window::get_size();
    }

    static bool init_and_attach_framebuffer_tex(GLuint* const texGLID, const GLuint fbGLID, const Vec2DI texSize) {
        assert(*texGLID == 0);

        glGenTextures(1, texGLID);
        glBindTexture(GL_TEXTURE_2D, *texGLID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.x, texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // TODO: Handle resizing.
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

    bool Renderer::init(MemArena* const memArena, const int surfCnt, const int batchCnt) {
        assert(is_zero(this));
        assert(surfCnt >= 0);
        assert(batchCnt > 0);

        //
        // Surfaces
        //
        if (surfCnt > 0) {
            if (!m_surfs.init(memArena, surfCnt)) {
                return false;
            }

            for (int i = 0; i < surfCnt; ++i) {
                RenderSurface* const surf = &m_surfs[i];

                glGenFramebuffers(1, &surf->framebufferGLID);

                if (!init_and_attach_framebuffer_tex(&surf->framebufferTexGLID, surf->framebufferGLID, get_surface_targ_size())) {
                    return false;
                }
            }

            // NOTE: The below can exist for the lifetime of the game, so why regenerate it every scene?
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
        }

        //
        // Batches
        //
        m_batchCnt = batchCnt;

        // Reserve memory for batch data.
        m_batchPermDatas = memArena->push<RenderBatchPermData>(batchCnt);

        if (!m_batchPermDatas) {
            return false;
        }

        m_batchTransDatas = memArena->push<RenderBatchTransientData>(batchCnt);

        if (!m_batchTransDatas) {
            return false;
        }

        // Initialise batch indices.
        const int indicesLen = 6 * gk_renderBatchSlotLimit;
        const auto indices = alloc<unsigned short>(indicesLen);

        if (!indices) {
            return false;
        }

        for (int i = 0; i < gk_renderBatchSlotLimit; i++) {
            indices[(i * 6) + 0] = static_cast<unsigned short>((i * 4) + 0);
            indices[(i * 6) + 1] = static_cast<unsigned short>((i * 4) + 1);
            indices[(i * 6) + 2] = static_cast<unsigned short>((i * 4) + 2);
            indices[(i * 6) + 3] = static_cast<unsigned short>((i * 4) + 2);
            indices[(i * 6) + 4] = static_cast<unsigned short>((i * 4) + 3);
            indices[(i * 6) + 5] = static_cast<unsigned short>((i * 4) + 0);
        }

        // Initialise batches.
        for (int i = 0; i < batchCnt; ++i) {
            RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];

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
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indicesLen, indices, GL_STATIC_DRAW);

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

        // Set up texture units.
        for (int i = 0; i < gk_texUnitLimit; ++i) {
            m_texUnits[i] = i;
        }

        //
        // Instructions
        //
        if (!m_renderInstrs.init(memArena, gk_renderInstrLimit)) {
            return false;
        }

        m_initialized = true;

        return true;
    }

    void Renderer::clean() {
        for (int i = 0; i < m_batchCnt; ++i) {
            glDeleteVertexArrays(1, &m_batchPermDatas[i].vertArrayGLID);
            glDeleteBuffers(1, &m_batchPermDatas[i].vertBufGLID);
            glDeleteBuffers(1, &m_batchPermDatas[i].elemBufGLID);
        }

        glDeleteVertexArrays(1, &m_surfVertArrayGLID);
        glDeleteBuffers(1, &m_surfVertBufGLID);
        glDeleteBuffers(1, &m_surfElemBufGLID);

        for (int i = 0; i < m_surfs.get_len(); ++i) {
            glDeleteFramebuffers(1, &m_surfs[i].framebufferGLID);
            glDeleteTextures(1, &m_surfs[i].framebufferTexGLID);
        }

        zero_out(this);
    }

    bool Renderer::render(const InternalShaderProgs& internalShaderProgs, const Assets& assets, MemArena* const scratchSpace) {
        assert(m_initialized);
        assert(!m_inSubmissionPhase);

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

                case RenderInstrType::SetDrawSurfaceShaderProg:
                    surfShaderProgGLID = assets.get_shader_prog_gl_id(instr.data.setDrawSurfaceShaderProg.progIndex);
                    break;

                case RenderInstrType::SetDrawSurfaceShaderUniform:
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

        return true;
    }

    bool Renderer::resize_surfaces() {
        for (int i = 0; i < m_surfs.get_len(); ++i) {
            RenderSurface* const surf = &m_surfs[i];

            // Delete the old texture.
            glDeleteTextures(1, &surf->framebufferTexGLID);
            surf->framebufferTexGLID = 0;

            // Generate a new texture of the desired size and attach it to the framebuffer.
            if (!init_and_attach_framebuffer_tex(&surf->framebufferTexGLID, surf->framebufferGLID, get_surface_targ_size())) {
                return false;
            }
        }

        return true;
    }

    void Renderer::begin_submission_phase() {
        assert(m_initialized);
        assert(!m_inSubmissionPhase);

        zero_out(m_batchTransDatas, m_batchSubmitIndex + 1);

        m_batchSubmitIndex = 0;

        m_renderInstrs.clear();

        m_inSubmissionPhase = true;
    }

    void Renderer::end_submission_phase() {
        assert(m_initialized);
        assert(m_inSubmissionPhase);

        // Submit render batch vertex data to the GPU.
        for (int i = 0; i <= m_batchSubmitIndex; ++i) {
            const RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];
            const RenderBatchTransientData* const batchTransData = &m_batchTransDatas[i];

            glBindVertexArray(batchPermData->vertArrayGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batchPermData->vertBufGLID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batchTransData->verts), batchTransData->verts);
        }

        m_inSubmissionPhase = false;
    }

    void Renderer::submit_texture_to_batch(const int texIndex, const Assets& assets, const Vec2D pos, const RectI& srcRect, const Vec2D origin, const float rot, const Vec2D scale, const float alpha) {
        assert(m_initialized);
        assert(m_inSubmissionPhase);

        const Vec2DI texSize = assets.get_tex_size(texIndex);
        const Rect texCoords = {
            static_cast<float>(srcRect.x) / texSize.x,
            static_cast<float>(srcRect.y) / texSize.y,
            static_cast<float>(srcRect.width) / texSize.x,
            static_cast<float>(srcRect.height) / texSize.y
        };
        submit_to_batch(origin, scale, pos, get_rect_size(srcRect), rot, assets.get_tex_gl_id(texIndex), texCoords, alpha);
    }

    void Renderer::submit_str_to_batch(const char* const str, const int fontIndex, const Assets& assets, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign, const StrVerAlign verAlign) {
        // TODO: Calculate and store the vertex data for a string once.

        assert(m_initialized);
        assert(m_inSubmissionPhase);

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

        const FontArrangementInfo& fontArrangementInfo = assets.get_font_arrangement_info(fontIndex);

        for (int i = 0; i < strLen; i++) {
            if (str[i] == '\n') {
                strLineWidths[strLineCnter] = charDrawPosPen.x;

                if (!strFirstLineMinOffsUpdated) {
                    strFirstLineMinOffs = fontArrangementInfo.chars.verOffsets[0];
                    strFirstLineMinOffsUpdated = true;
                }

                strLastLineMaxHeight = fontArrangementInfo.chars.verOffsets[0] + fontArrangementInfo.chars.srcRects[0].height;
                strLastLineMaxHeightUpdated = false;

                strLineCnter++;
                charDrawPosPen.x = 0;
                charDrawPosPen.y += fontArrangementInfo.lineHeight;
                continue;
            }

            const int strCharIndex = str[i] - gk_fontCharRangeBegin;

            if (strLineCnter == 0) {
                if (!strFirstLineMinOffsUpdated) {
                    strFirstLineMinOffs = fontArrangementInfo.chars.verOffsets[strCharIndex];
                    strFirstLineMinOffsUpdated = true;
                } else {
                    strFirstLineMinOffs = std::min(fontArrangementInfo.chars.verOffsets[strCharIndex], strFirstLineMinOffs);
                }
            }

            if (!strLastLineMaxHeightUpdated) {
                strLastLineMaxHeight = fontArrangementInfo.chars.verOffsets[strCharIndex] + fontArrangementInfo.chars.srcRects[strCharIndex].height;
                strLastLineMaxHeightUpdated = true;
            } else {
                strLastLineMaxHeight = std::max(fontArrangementInfo.chars.verOffsets[strCharIndex] + fontArrangementInfo.chars.srcRects[strCharIndex].height, strLastLineMaxHeight);
            }

            if (i > 0) {
                const int strCharIndexLast = str[i - 1] - gk_fontCharRangeBegin;
                charDrawPosPen.x += fontArrangementInfo.chars.kernings[(strCharIndex * gk_fontCharRangeLen) + strCharIndexLast];
            }

            charDrawPositions[i].x = charDrawPosPen.x + fontArrangementInfo.chars.horOffsets[strCharIndex];
            charDrawPositions[i].y = charDrawPosPen.y + fontArrangementInfo.chars.verOffsets[strCharIndex];

            charDrawPosPen.x += fontArrangementInfo.chars.horAdvances[strCharIndex];
        }

        strLineWidths[strLineCnter] = charDrawPosPen.x;

        // Write the characters.
        const int strHeight = strFirstLineMinOffs + charDrawPosPen.y + strLastLineMaxHeight;

        const GLuint fontTexGLID = assets.get_font_tex_gl_id(fontIndex);
        const Vec2DI fontTexSize = assets.get_font_tex_size(fontIndex);

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

            const Vec2D charSize = get_rect_size(fontArrangementInfo.chars.srcRects[charIndex]);

            const Rect charTexCoords = {
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].x) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].y) / fontTexSize.y,
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].width) / fontTexSize.x,
                static_cast<float>(fontArrangementInfo.chars.srcRects[charIndex].height) / fontTexSize.y
            };

            submit_to_batch({}, {1.0f, 1.0f}, charPos, charSize, 0.0f, fontTexGLID, charTexCoords, 1.0f);
        }
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

    void Renderer::move_to_next_batch() {
        assert(m_batchSubmitIndex < m_batchCnt - 1);
        assert(m_batchTransDatas[m_batchSubmitIndex].slotsUsedCnt > 0); // Make sure we aren't wasting the current batch.

        ++m_batchSubmitIndex;
        assert(is_zero(&m_batchTransDatas[m_batchSubmitIndex])); // Make sure the new batch is empty.
    }

    void Renderer::submit_instr(const RenderInstrType type, const RenderInstrData data) {
        assert(m_initialized);
        assert(m_inSubmissionPhase);
        assert(!m_renderInstrs.is_full());

        if (m_batchTransDatas[m_batchSubmitIndex].slotsUsedCnt > 0) {
            if (type == RenderInstrType::SetViewMatrix || type == RenderInstrType::SetSurface || type == RenderInstrType::UnsetSurface) {
                move_to_next_batch();
            }
        }

        m_renderInstrs.push({.type = type, .data = data});
    }

    void Renderer::submit_to_batch(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha) {
        assert(m_initialized);
        assert(m_inSubmissionPhase);

        RenderBatchTransientData* const batchTransData = &m_batchTransDatas[m_batchSubmitIndex];

        // Check if the batch is full or if it cannot support the texture.
        int texUnit;

        if (batchTransData->slotsUsedCnt == gk_renderBatchSlotLimit || (texUnit = add_tex_unit_to_batch(batchTransData, texGLID)) == -1) {
            move_to_next_batch();

            // Submit again but this time to the new batch.
            submit_to_batch(origin, scale, pos, size, rot, texGLID, texCoords, alpha);

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

        if (batchTransData->slotsUsedCnt == 1) {
            // This is the first submission to this batch, so submit an instruction to draw the batch.
            submit_draw_batch_instr(m_batchSubmitIndex);
        }
    }
}
