#pragma once

#include <glad/glad.h>

namespace gl {

    template<GLenum target>
    class Buffer {
    public:
        Buffer() = default;
        Buffer(const void* data, const GLsizeiptr& size, const GLenum& flags) { Create(data, size, flags); }
        //~Buffer() { Destroy(); }

        inline virtual void Bind() const { glBindBuffer(target, m_RendererID); }
        static void Unbind() { glBindBuffer(target, 0); }

        virtual void Create(const void* data, const GLsizeiptr& size, GLbitfield flags) {
           glCreateBuffers(1, &m_RendererID);
           glNamedBufferStorage(m_RendererID, size, data, flags);
        }

        inline virtual void Destroy() { glDeleteBuffers(1, &m_RendererID); }

        virtual void SetData(const GLintptr& offset, const GLsizeiptr& size, const void* data) {
           glNamedBufferSubData(m_RendererID, offset, size, data);
        }

        virtual void* GetData(const GLintptr& offset, const GLsizeiptr& size) {
            void* data = nullptr;
            glGetNamedBufferSubData(m_RendererID, offset, size, data);
            return data;
        }

        void* Map(const GLenum& access) {
            return glMapNamedBuffer(m_RendererID, access);
        }

        bool UnMap() {
            return glUnmapNamedBuffer(m_RendererID);
        }

        inline operator GLuint&() { return m_RendererID; }
        inline operator GLuint&() const { return m_RendererID; }
    protected:
        GLuint m_RendererID = 0;
    };

    using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
    using IndexBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;

    class UniformBuffer : public Buffer<GL_UNIFORM_BUFFER> {
    public:
        void BufferRange(const GLuint& index, const GLintptr& offset, const GLsizeiptr& size) { glBindBufferRange(GL_UNIFORM_BUFFER, index, m_RendererID, offset, size); }
        void BufferBase(const GLuint& index) { glBindBufferBase(GL_UNIFORM_BUFFER, index, m_RendererID); }
    };

    class ShaderStorageBuffer : public Buffer<GL_SHADER_STORAGE_BUFFER> {
    public:
        void BufferBase(const GLuint& index) { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_RendererID); }
    };
}