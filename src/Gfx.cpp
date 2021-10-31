#pragma once
#include "Gfx.h"
#include "Math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace gfx
{
    template<>
    void VertexBuffer<int>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribIPointer(slot, 1, GL_INT, 0, (void*)0);
    }

    template<>
    void VertexBuffer<math::ivec4>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribIPointer(slot, 4, GL_INT, 0, (void*)0);
    }

    template<>
    void VertexBuffer<math::ivec2>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribIPointer(slot, 2, GL_INT, 0, (void*)0);
    }

    template<>
    void VertexBuffer<float>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribPointer(slot, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    template<>
    void VertexBuffer<math::vec3>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribPointer(slot, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    template<>
    void VertexBuffer<math::vec2>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribPointer(slot, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    template<>
    void VertexBuffer<math::vec4>::SetAttributePointer(unsigned int slot)
    {
        glVertexAttribPointer(slot, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    // Uniform
#define UNIFORM_IMPL(glFunction, templateType, dataType)                                                    \
    template<> void uniform::Update<templateType>(unsigned int slot, templateType* data, unsigned int len)  \
                                            { glFunction(slot, (GLsizei)len, (dataType*)&data[0]);}         \

    UNIFORM_IMPL(glUniform1iv, int, int);
    UNIFORM_IMPL(glUniform4iv, math::ivec4, int);
    UNIFORM_IMPL(glUniform2iv, math::ivec2, int);
    UNIFORM_IMPL(glUniform1fv, float, float);
    UNIFORM_IMPL(glUniform2fv, math::vec2, float);
    UNIFORM_IMPL(glUniform3fv, math::vec3, float);
    UNIFORM_IMPL(glUniform4fv, math::vec4, float);
    UNIFORM_IMPL(glUniform4fv, math::Quaternion, float);

    template<>
    void uniform::Update<math::mat4>(unsigned int slot, math::mat4* data, unsigned int len)
    {
        glUniformMatrix4fv(slot, (GLsizei)len, false, (float*)&data[0]);
    }

    namespace draw {
        // Draw functions
        GLenum MapDrawModeToGLEnum(draw::DRAW_MODE drawMode)
        {
            GLenum result = GL_ZERO;
            switch (drawMode)
            {
            case draw::DRAW_MODE::POINTS:
                result = GL_POINTS;
                break;
            case draw::DRAW_MODE::LINES:
                result = GL_LINES;
                break;
            case draw::DRAW_MODE::LINE_STRIP:
                result = GL_LINE_STRIP;
                break;
            case draw::DRAW_MODE::LINE_LOOP:
                result = GL_LINE_LOOP;
                break;
            case draw::DRAW_MODE::TRIANGLES:
                result = GL_TRIANGLES;
                break;
            case draw::DRAW_MODE::TRIANGLE_STRIP:
                result = GL_TRIANGLE_STRIP;
                break;
            case draw::DRAW_MODE::TRIANGLE_FAN:
                result = GL_TRIANGLE_FAN;
                break;
            }
            return result;
        }

        void Draw(const IndexBuffer& ib, draw::DRAW_MODE drawMode)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.handle);
            glDrawElements(MapDrawModeToGLEnum(drawMode), static_cast<unsigned int>(ib.m_data.size()), GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
        }

        void Draw(unsigned int vertexCount, draw::DRAW_MODE drawMode)
        {
            glDrawArrays(MapDrawModeToGLEnum(drawMode), 0, (GLsizei)vertexCount);
        }

        void DrawInstanced(const IndexBuffer& ib, draw::DRAW_MODE drawMode, unsigned int instanceCount)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.handle);
            glDrawElementsInstanced(MapDrawModeToGLEnum(drawMode), static_cast<unsigned int>(ib.m_data.size()),
                GL_UNSIGNED_INT, 0, instanceCount);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
        }

        void DrawInstanced(unsigned int vertexCount, draw::DRAW_MODE drawMode, unsigned instanceCount)
        {
            glDrawArraysInstanced(MapDrawModeToGLEnum(drawMode), 0, (GLsizei)vertexCount, instanceCount);
        }
    }

    // Textures
#define STB_IMAGE_IMPLEMENTATION

    Texture::Texture(const char* filePath)
    {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);
        int w, h, c;
        unsigned char* data = stbi_load(filePath, &w, &h, &c, 4);
        width = w;
        height = h;
        channels = c;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_ZERO);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &handle);
    }

    void Texture::Bind(unsigned int uniformIndex, unsigned int textureSlot)
    {
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, handle);
        glUniform1i(uniformIndex, textureSlot);
    }

    void Texture::UnBind(unsigned int textureSlot)
    {
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, GL_ZERO);
        glActiveTexture(GL_TEXTURE0);
    }
}