#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl {

    class VertexBuffer {
    public:
        VertexBuffer() {}
        VertexBuffer(const void* data, const GLsizeiptr& size, const GLenum& mode = GL_STATIC_DRAW) { Create(data, size, mode); }
        ~VertexBuffer() { Destroy(); }

        void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); }
        void Unbind() const { glBindVertexArray(0); }

        void Create(const void* data, const GLsizeiptr& size, const GLenum& mode = GL_STATIC_DRAW) {
            if (!m_RendererID) {
                glGenBuffers(1, &m_RendererID);

                glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
                glBufferData(GL_ARRAY_BUFFER, size, data, mode);
            }
        }

        void Destroy() { glDeleteBuffers(1, &m_RendererID); }

        void Update(const GLintptr& offset, const GLsizeiptr& size, const void* data) {
            if (m_RendererID) {
                glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
                glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
            }
        }

        uint32_t GetRendererID() const { return m_RendererID; }
    private:
        uint32_t m_RendererID = 0;
    };
}
#endif