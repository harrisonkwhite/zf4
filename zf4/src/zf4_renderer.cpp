#include <zf4_renderer.h>

#include <algorithm>
#include <zf4_window.h>
#include <zf4_assets.h>

namespace zf4 {
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

    static inline GLuint get_active_shader_prog_gl_id() {
        GLint progGLIDSigned;
        glGetIntegerv(GL_CURRENT_PROGRAM, &progGLIDSigned);
        return static_cast<GLuint>(progGLIDSigned);
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

                if (!init_and_attach_framebuffer_tex(&surf->framebufferTexGLID, surf->framebufferGLID, Window::get_size())) {
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
        for (int i = 0; i < batchCnt; ++i) {
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

        // Set up texture units.
        for (int i = 0; i < gk_texUnitLimit; ++i) {
            m_texUnits[i] = i;
        }

        //
        // Instructions
        //
        m_renderInstrs.init(memArena, 256); // TEMP

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

    void Renderer::render(const InternalShaderProgs& internalShaderProgs) {
        // Set the background to black.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Iterate through and execute render instructions.
        Matrix4x4 viewMat = Matrix4x4::create_identity();

        for (int i = 0; i < m_renderInstrs.get_len(); ++i) {
            const RenderInstr instr = m_renderInstrs[i];

            switch (instr.type) {
                case RenderInstrType::Clear:
                    glClearColor(instr.data.clearData.color.x, instr.data.clearData.color.y, instr.data.clearData.color.z, instr.data.clearData.color.w);
                    glClear(GL_COLOR_BUFFER_BIT);
                    break;

                case RenderInstrType::SetViewMatrix:
                    viewMat = instr.data.setViewMatrixData.mat;
                    break;

                case RenderInstrType::DrawBatch:
                    {
                        const int batchIndex = instr.data.drawBatchData.index;
                        const RenderBatchTransientData& batchTransData = m_batchTransDatas[batchIndex];
                        const RenderBatchPermData& batchPermData = m_batchPermDatas[batchIndex];

                        // NOTE: At the moment we do a full reset of the context for each batch. Will optimise this later.
                        glUseProgram(internalShaderProgs.texturedQuad.glID);

                        const Matrix4x4 projMat = Matrix4x4::create_ortho(0.0f, static_cast<float>(Window::get_size().x), static_cast<float>(Window::get_size().y), 0.0f, -1.0f, 1.0f);
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
                    glBindFramebuffer(GL_FRAMEBUFFER, m_surfs[instr.data.setSurfaceData.index].framebufferGLID);
                    break;

                case RenderInstrType::UnsetSurface:
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    break;

                case RenderInstrType::SetSurfaceShaderProg:
                    {
                        const GLuint progGLID = AssetManager::get_shader_prog_gl_id(instr.data.setSurfaceShaderProgData.progIndex);
                        glUseProgram(progGLID);
                    }

                    break;

                case RenderInstrType::SetSurfaceShaderProgUniform:
                    {
                        const GLuint progGLID = get_active_shader_prog_gl_id();
                        const int uniformLoc = glGetUniformLocation(progGLID, instr.data.setSurfaceShaderProgUniformData.name);
                        const ShaderUniformVal uniformVal = instr.data.setSurfaceShaderProgUniformData.val;

                        switch (instr.data.setSurfaceShaderProgUniformData.valType) {
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
                        // TODO: Check that a shader program has been set?

                        glActiveTexture(GL_TEXTURE0);
                        const RenderSurface& surf = m_surfs[instr.data.drawSurfaceData.index];
                        glBindTexture(GL_TEXTURE_2D, surf.framebufferTexGLID);

                        glBindVertexArray(m_surfVertArrayGLID);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_surfElemBufGLID);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
                    }

                    break;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // TEMP?
    }

    bool Renderer::resize_surfaces(const Vec2DI size) {
        for (int i = 0; i < m_surfs.get_len(); ++i) {
            RenderSurface* const surf = &m_surfs[i];

            // Delete the old texture.
            glDeleteTextures(1, &surf->framebufferTexGLID);
            surf->framebufferTexGLID = 0;

            // Generate a new texture of the desired size and attach it to the framebuffer.
            if (!init_and_attach_framebuffer_tex(&surf->framebufferTexGLID, surf->framebufferGLID, size)) {
                return false;
            }
        }

        return true;
    }

    void Renderer::begin_draw() {
        assert(m_initialized);
        assert(!m_drawActive);

        zero_out(m_batchTransDatas, m_batchWriteIndex + 1);
        m_batchWriteIndex = 0;

        m_renderInstrs.clear();

        m_drawActive = true;
    }

    void Renderer::end_draw() {
        assert(m_initialized);
        assert(m_drawActive);

        // Submit render batch vertex data to the GPU.
        for (int i = 0; i <= m_batchWriteIndex; ++i) {
            const RenderBatchPermData* const batchPermData = &m_batchPermDatas[i];
            const RenderBatchTransientData* const batchTransData = &m_batchTransDatas[i];

            glBindVertexArray(batchPermData->vertArrayGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batchPermData->vertBufGLID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batchTransData->verts), batchTransData->verts);
        }

        m_drawActive = false;
    }

    void Renderer::clear(const Vec4D& color) {
        assert(m_initialized);
        assert(m_drawActive);

        if (!m_renderInstrs.is_full()) {
            RenderInstr instr = {
                .type = RenderInstrType::Clear
            };
            instr.data.clearData.color = color;
            m_renderInstrs.add(instr);
        } else {
            assert(false);
        }
    }

    void Renderer::set_view_matrix(const Matrix4x4& mat) {
        assert(m_initialized);
        assert(m_drawActive);

        if (!m_renderInstrs.is_full() && m_batchWriteIndex < m_batchCnt - 1) {
            RenderInstr instr = {
                .type = RenderInstrType::SetViewMatrix
            };
            instr.data.setViewMatrixData.mat = mat;
            m_renderInstrs.add(instr);
            ++m_batchWriteIndex;
        } else {
            assert(false);
        }
    }

    void Renderer::draw_texture(const int texIndex, const Vec2D pos, const RectI& srcRect, const Vec2D origin, const float rot, const Vec2D scale, const float alpha) {
        assert(m_initialized);
        assert(m_drawActive);

        const Vec2DI texSize = AssetManager::get_tex_size(texIndex);
        const Rect texCoords = {
            static_cast<float>(srcRect.x) / texSize.x,
            static_cast<float>(srcRect.y) / texSize.y,
            static_cast<float>(srcRect.width) / texSize.x,
            static_cast<float>(srcRect.height) / texSize.y
        };
        write(origin, scale, pos, get_rect_size(srcRect), rot, AssetManager::get_tex_gl_id(texIndex), texCoords, alpha);
    }

    void Renderer::draw_str(const char* const str, const int fontIndex, const Vec2D pos, MemArena* const scratchSpace, const StrHorAlign horAlign, const StrVerAlign verAlign) {
        assert(m_initialized);
        assert(m_drawActive);

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

        const FontArrangementInfo& fontArrangementInfo = AssetManager::get_font_arrangement_info(fontIndex);

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

        const GLuint fontTexGLID = AssetManager::get_font_tex_gl_id(fontIndex);
        const Vec2DI fontTexSize = AssetManager::get_font_tex_size(fontIndex);

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

            write({}, {1.0f, 1.0f}, charPos, charSize, 0.0f, fontTexGLID, charTexCoords, 1.0f);
        }
    }

    void Renderer::set_surface(const int index) {
        assert(m_initialized);
        assert(m_drawActive);
        assert(index >= 0 && index < m_surfs.get_len());

        if (!m_renderInstrs.is_full() && m_batchWriteIndex < m_batchCnt - 1) {
            RenderInstr instr = {
                .type = RenderInstrType::SetSurface
            };
            instr.data.setSurfaceData.index = index;
            m_renderInstrs.add(instr);
            ++m_batchWriteIndex;
        } else {
            assert(false);
        }
    }

    void Renderer::unset_surface() {
        assert(m_initialized);
        assert(m_drawActive);

        if (!m_renderInstrs.is_full() && m_batchWriteIndex < m_batchCnt - 1) {
            m_renderInstrs.add({.type = RenderInstrType::UnsetSurface});
            ++m_batchWriteIndex;
        } else {
            assert(false);
        }
    }

    void Renderer::set_surface_shader_prog(const int progIndex) {
        assert(m_initialized);
        assert(m_drawActive);

        if (!m_renderInstrs.is_full()) {
            RenderInstr instr = {
                .type = RenderInstrType::SetSurfaceShaderProg
            };

            instr.data.setSurfaceShaderProgData.progIndex = progIndex;

            m_renderInstrs.add(instr);
        } else {
            assert(false);
        }
    }

    void Renderer::set_surface_shader_prog_uniform(const char* const name, const ShaderUniformVal val, const ShaderUniformValType valType) {
        assert(m_initialized);
        assert(m_drawActive);

        if (!m_renderInstrs.is_full()) {
            RenderInstr instr = {
                .type = RenderInstrType::SetSurfaceShaderProgUniform
            };

            snprintf(instr.data.setSurfaceShaderProgUniformData.name, sizeof(instr.data.setSurfaceShaderProgUniformData.name), "%s", name);

            instr.data.setSurfaceShaderProgUniformData.val = val;
            instr.data.setSurfaceShaderProgUniformData.valType = valType;

            m_renderInstrs.add(instr);
        } else {
            assert(false);
        }
    }

    void Renderer::draw_surface(const int index) {
        assert(m_initialized);
        assert(m_drawActive);

        if (!m_renderInstrs.is_full()) {
            RenderInstr instr = {
                .type = RenderInstrType::DrawSurface
            };

            instr.data.drawSurfaceData.index = index;

            m_renderInstrs.add(instr);
        } else {
            assert(false);
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

    void Renderer::write(const Vec2D origin, const Vec2D scale, const Vec2D pos, const Vec2D size, const float rot, const GLuint texGLID, const Rect texCoords, const float alpha) {
        assert(m_initialized);
        assert(m_drawActive);

        RenderBatchTransientData* const batchTransData = &m_batchTransDatas[m_batchWriteIndex];

        // Check if the batch is full or if it cannot support the texture.
        int texUnit;

        if (batchTransData->slotsUsedCnt == gk_renderBatchSlotLimit || (texUnit = add_tex_unit_to_batch(batchTransData, texGLID)) == -1) {
            if (m_batchWriteIndex < m_batchCnt - 1) {
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

        // NOTE: Kind of hacky - change?
        if (batchTransData->slotsUsedCnt == 1) {
            RenderInstr instr = {
                .type = RenderInstrType::DrawBatch
            };

            instr.data.drawBatchData.index = m_batchWriteIndex;

            m_renderInstrs.add(instr);
        }
    }
}
