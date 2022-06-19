#pragma once

#include <glad/glad.h>

namespace gl {

    class VertexArray {
        GLuint m_RendererID = 0;
    public:
        VertexArray() {}
        //~VertexArray() { Destroy(); }

        void Create() {
            glCreateVertexArrays(1, &m_RendererID);
        }

        void VertexAttribPointer(const GLuint& index, const int& size, const GLuint& offset, const GLenum& type = GL_FLOAT, const bool& normalized = false) {
            glEnableVertexArrayAttrib(m_RendererID, index);
            glVertexArrayAttribFormat(m_RendererID, index, size, type, normalized, offset);
            glVertexArrayAttribBinding(m_RendererID, index, 0);
        }

        void SetVertexBuffer(const GLuint& VBO, const GLuint& stride, const GLuint& offset = 0, const GLuint& binding = 0) {
            glVertexArrayVertexBuffer(m_RendererID, binding, VBO, offset, stride);
        }

        void SetIndexBuffer(const GLuint& EBO) {
            glVertexArrayElementBuffer(m_RendererID, EBO);
        }

        void BindingDivisor(const GLuint& index, const GLuint& divisor) {
            glVertexArrayBindingDivisor(m_RendererID, index, divisor);
        }

        void Bind() const { glBindVertexArray(m_RendererID); }

        void Destroy() { glDeleteVertexArrays(1, &m_RendererID); }

        inline operator GLuint& () { return m_RendererID; }
        inline operator const GLuint& () const { return m_RendererID; }
    };
}