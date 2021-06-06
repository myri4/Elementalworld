#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <glad/glad.h>

namespace gl {

    class VertexArray {
    public:
        VertexArray() {}
        ~VertexArray() { Destroy(); }

        void Create() {
            glGenVertexArrays(1, &m_RendererID);
            glBindVertexArray(m_RendererID);
        }

        void Bind() const { glBindVertexArray(m_RendererID); }

        void Unind() const { glBindVertexArray(0); }

        void Destroy() { glDeleteVertexArrays(1, &m_RendererID); }

        operator GLuint() const { return m_RendererID; }
    private:
        GLuint m_RendererID = 0;
    };
}
#endif