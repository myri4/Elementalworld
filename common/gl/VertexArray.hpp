#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <glad/glad.h>

namespace gl {

    class VertexArray {
    public:
        VertexArray() {}
        ~VertexArray() { Destroy(); }

        void Create() {
            glCreateVertexArrays(1, &m_RendererID);
            glBindVertexArray(m_RendererID);
        }

        void Bind() const { glBindVertexArray(m_RendererID); }

        static void Unind() { glBindVertexArray(0); }

        void Destroy() { glDeleteVertexArrays(1, &m_RendererID); }

        inline operator GLuint& () { return m_RendererID; }
        inline operator const GLuint& () const { return m_RendererID; }
    private:
        GLuint m_RendererID = 0;
    };
}
#endif