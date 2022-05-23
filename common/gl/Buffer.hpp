#pragma once

#include <glad/glad.h>

namespace gl {

    class Buffer {
    public:
        Buffer() = default;
        Buffer(const GLsizeiptr& size, const GLenum& flags = 0, const void* data = nullptr) { Create(size, flags, data); }

        void Create(const GLsizeiptr& size, const GLenum& flags = 0, const void* data = nullptr) {
           glCreateBuffers(1, &m_RendererID);
           glNamedBufferStorage(m_RendererID, size, data, flags);
        }

        inline void Destroy() { glDeleteBuffers(1, &m_RendererID); }

        void SetData(const GLsizeiptr& size, const void* data, const GLintptr& offset = 0) {
           glNamedBufferSubData(m_RendererID, offset, size, data);
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

        void BufferRange(const GLuint& index, const GLintptr& offset, const GLsizeiptr& size) { glBindBufferRange(target, index, m_RendererID, offset, size); }
        void BufferBase(const GLuint& index) { glBindBufferBase(target, index, m_RendererID); }
    };

    using UniformBuffer = IndexedBuffer<GL_UNIFORM_BUFFER>;
}