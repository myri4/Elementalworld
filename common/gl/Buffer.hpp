#pragma once

#include <glad/glad.h>

namespace gl {

    class Buffer {
    public:
        Buffer() = default;
        Buffer(const void* data, const GLsizeiptr& size, const GLenum& flags) { Create(data, size, flags); }

        void Create(const void* data, const GLsizeiptr& size, GLbitfield flags) {
           glCreateBuffers(1, &m_RendererID);
           glNamedBufferStorage(m_RendererID, size, data, flags);
        }

        inline void Destroy() { glDeleteBuffers(1, &m_RendererID); }

        void SetData(const GLintptr& offset, const GLsizeiptr& size, const void* data) {
           glNamedBufferSubData(m_RendererID, offset, size, data);
        }

        void* GetData(const GLintptr& offset, const GLsizeiptr& size) {
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
        inline operator const GLuint&() const { return m_RendererID; }
    protected:
        GLuint m_RendererID = 0;
    };

    template<GLenum target>
    class IndexedBuffer : public Buffer {
    public:
        IndexedBuffer() = default;
        IndexedBuffer(const void* data, const GLsizeiptr & size, const GLenum & flags) { Create(data, size, flags); }

        inline void Bind() const { glBindBuffer(target, m_RendererID); }
        inline void Unbind() const { glBindBuffer(target, 0); }

        void BufferRange(const GLuint& index, const GLintptr& offset, const GLsizeiptr& size) { glBindBufferRange(target, index, m_RendererID, offset, size); }
        void BufferBase(const GLuint& index) { glBindBufferBase(target, index, m_RendererID); }
    };

    using UniformBuffer = IndexedBuffer<GL_UNIFORM_BUFFER>;
    using ShaderStorageBuffer = IndexedBuffer<GL_SHADER_STORAGE_BUFFER>;
}