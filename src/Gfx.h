#pragma once
#include <map>
#include <iostream>
#include <vector>

#include "utils.h"
#include "glad.h"

namespace gfx
{
    struct Shader
    {
        Shader(const Shader&) = delete;
        Shader operator=(const Shader&) = delete;

        unsigned int CompileShader(const char* source, std::string shaderType)
        {
            GLenum type = shaderType == "Vertex" ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
            unsigned int handle = glCreateShader(type);
            glShaderSource(handle, 1, &source, NULL);
            glCompileShader(handle);

            int success = 0;
            glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char infoLog[512];
                glGetShaderInfoLog(handle,
                    512, NULL, infoLog);
                std::cout << shaderType << " compilation failed.\n";
                std::cout << "\t" << infoLog << "\n";
                glDeleteShader(handle);
                handle = GL_ZERO;
            }
            return handle;
        }

        Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
        {
            std::string vertexShader = utils::ReadFile(vertexShaderPath);
            std::string fragmentShader = utils::ReadFile(fragmentShaderPath);

            // Compile
            unsigned int vHandle = CompileShader(vertexShader.c_str(), "Vertex");
            unsigned int fHandle = CompileShader(fragmentShader.c_str(), "Fragment");
            
            // Link
            if (vHandle != 0 && fHandle != 0)
            {
                handle = glCreateProgram();
                glAttachShader(handle, vHandle);
                glAttachShader(handle, fHandle);
                glLinkProgram(handle);

                int success = 0;
                glGetProgramiv(handle,
                    GL_LINK_STATUS, &success);
                if (!success) {
                    char infoLog[512];
                    glGetProgramInfoLog(handle, 512, NULL, infoLog);
                    std::cout << "ERROR: Shader linking failed.\n";
                    std::cout << "\t" << infoLog << "\n";
                    handle = GL_ZERO;
                }
                glDeleteShader(vHandle);
                glDeleteShader(fHandle);
            }

            if (handle)
            {
                // Populate attributes
                int count = 0;
                int length = 0, size = 0;
                char name[256];
                GLenum type;
                glUseProgram(handle);
                glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &count);
                for (int i = 0; i < count; ++i)
                {
                    memset(name, 0, sizeof(name));
                    glGetActiveAttrib(handle, i, sizeof(name), &length, &size, &type, name);
                    int attributeLocation = glGetAttribLocation(handle, name);
                    if (attributeLocation >= 0)
                    {
                        attributes[name] = attributeLocation;
                    }
                }

                // Populate uniforms
                glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &count);
                for (int i = 0; i < count; ++i)
                {
                    memset(name, 0, sizeof(name));
                    glGetActiveUniform(handle, i, sizeof(name), &length, &size, &type, name);
                    int uniformLocation = glGetUniformLocation(handle, name);
                    if (uniformLocation >= 0)
                    {
                        std::string uniformName = name;
                        std::size_t found = uniformName.find('[');
                        if (found != std::string::npos)
                        {
                            uniformName.erase(uniformName.begin() + found, uniformName.end());
                            unsigned int uniformIndex = 0;
                            while (true) {
                                memset(name, 0, sizeof(name));
                                sprintf_s(name, "%s[%d]", uniformName.c_str(), uniformIndex++);
                                int uniformLocation = glGetUniformLocation(handle, name);
                                if
                                    (uniformLocation < 0) {
                                    break;
                                }
                                uniforms[name] = uniformLocation;
                            }
                        }
                        uniforms[uniformName] = uniformLocation;
                    }
                }

                glUseProgram(GL_ZERO);
            }
        }

        ~Shader()
        {
            glDeleteProgram(handle);
        }

        inline void Bind()
        {
            glUseProgram(handle);
        }

        inline void unBind()
        {
            glUseProgram(GL_ZERO);
        }

        int GetAttribute(const std::string& name)
        {
            int location = -1;
            if (auto it = attributes.find(name);
                it != attributes.end())
            {
                location = it->second;
            }
            return location;
        }

        int GetUniform(const std::string& name)
        {
            int location = -1;
            if (auto it = uniforms.find(name);
                it != uniforms.end())
            {
                location = it->second;
            }
            return location;
        }

        unsigned int handle;
        std::map<std::string, unsigned int> attributes;
        std::map<std::string, unsigned int> uniforms;
    };

    template<typename T>
    struct VertexBuffer
    {
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer operator=(const VertexBuffer&) = delete;

        VertexBuffer(const std::vector<T>& data)
        {
            glGenBuffers(1, &handle);
            Upload(data);
        }

        ~VertexBuffer()
        {
            glDeleteBuffers(1, &handle);
        }

        void Upload(const std::vector<T>& data)
        {
            m_data = data;
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glBufferData(GL_ARRAY_BUFFER, sizeof(T) * m_data.size(), m_data.data(), GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
        }

        void SetAttributePointer(unsigned int slot);

        void Bind(unsigned int slot)
        {
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glEnableVertexAttribArray(slot);
            SetAttributePointer(slot);
            glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
        }

        void UnBind(unsigned int slot)
        {
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glDisableVertexAttribArray(slot);
            glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO);
        }

        std::vector<T> m_data;
        unsigned int handle;
    };

    namespace uniform
    {
        template<typename T>
        void Update(unsigned int slot, T* arrData, unsigned int len);

        template<typename T>
        inline void Update(unsigned int slot, const T& value)
        {
            Update(slot, (T*)&value, 1);
        }

        template<typename T>
        void Update(unsigned int slot, const std::vector<T>& data)
        {
            Update(slot, data.data(), data.size());
        }
    };

    struct IndexBuffer
    {
        IndexBuffer(std::vector<unsigned int>& data)
        {
            glGenBuffers(1, &handle);
            Update(data.data(), static_cast<unsigned int>(data.size()));
        }

        void Update(unsigned int* data, unsigned int len)
        {
            count = len;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, len * sizeof(unsigned int), data, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
        }

        unsigned int handle;
        unsigned count;
    };

    namespace draw
    {
        enum class DRAW_MODE
        {
            POINTS,
            LINES,
            LINE_STRIP,
            LINE_LOOP,
            TRIANGLES,
            TRIANGLE_STRIP,
            TRIANGLE_FAN
        };

        void Draw(const IndexBuffer& ib, draw::DRAW_MODE drawMode);
        void Draw(unsigned int vertexCount, draw::DRAW_MODE drawMode);
        void DrawInstanced(const IndexBuffer& ib, draw::DRAW_MODE drawMode, unsigned int instanceCount);
        void DrawInstanced(unsigned int vertexCount, draw::DRAW_MODE drawMode, unsigned instanceCount);
    }

    struct Texture
    {
        Texture(const Texture&) = delete;
        Texture operator=(const Texture&) = delete;
        
        Texture(const char* filePath);
        ~Texture();
        void Bind(unsigned int uniformIndex, unsigned int textureSlot);
        void UnBind(unsigned int textureSlot);

        unsigned int width;
        unsigned int height;
        unsigned int channels;
        unsigned int handle;
    };
}