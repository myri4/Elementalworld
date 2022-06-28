#pragma once

#include <glad/glad.h>

namespace gl {

    class Buffer {
    protected:
        GLuint m_RendererID = 0;
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

        void* Map(const GLenum& access, const uint32_t& length, const uint32_t& offset = 0) {
            return glMapNamedBufferRange(m_RendererID, offset, length, access);
        }

        bool UnMap() {
            return glUnmapNamedBuffer(m_RendererID);
        }

        inline operator GLuint&() { return m_RendererID; }
        inline operator const GLuint&() const { return m_RendererID; }
    };

    template<GLenum target>
    struct IndexedBuffer : public Buffer {
        IndexedBuffer() = default;
        IndexedBuffer(const void* data, const GLsizeiptr & size, const GLenum & flags) { Create(data, size, flags); }

        void BufferRange(const GLuint& index, const GLintptr& offset, const GLsizeiptr& size) { glBindBufferRange(target, index, m_RendererID, offset, size); }
        void BufferBase(const GLuint& index) { glBindBufferBase(target, index, m_RendererID); }
        void Bind() { glBindBuffer(target, m_RendererID); }
    };

    using UniformBuffer = IndexedBuffer<GL_UNIFORM_BUFFER>;
    using ShaderStorageBuffer = IndexedBuffer<GL_SHADER_STORAGE_BUFFER>;
    using DrawIndirectBuffer = IndexedBuffer<GL_DRAW_INDIRECT_BUFFER>;

    struct DrawElementsIndirectCommand{
        GLuint  count = 0;
        GLuint  instanceCount = 1;
        GLuint  firstIndex = 0;
        GLint  baseVertex = 0;
        GLuint  baseInstance = 1;
    };
}