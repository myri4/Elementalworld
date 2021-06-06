#ifndef INDEXBUFFER_HPP
#define INDEXBUFFER_HPP

#include <glad/glad.h>

namespace gl {
    class IndexBuffer {
    public:
        IndexBuffer() {}
        ~IndexBuffer() { Destroy(); }

        void Destroy() { glDeleteBuffers(1, &m_RendererID); }

        void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); }

        void Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

        operator uint32_t() const { return m_RendererID; }

        void Create(const uint32_t* data, const uint32_t& size, const uint32_t& mode = GL_STATIC_DRAW) {
           glGenBuffers(1, &m_RendererID);
           glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
           glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, mode);            
        }

        void Update(const GLintptr& offset, const uint32_t& size, const uint32_t* data) {
           glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
           glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);            
        }
    private:
        uint32_t m_RendererID = 0;
    };
}

#endif