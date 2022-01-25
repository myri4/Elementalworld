#pragma once

#include <glad/glad.h>

namespace gl {

    class VertexArray {
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

        void VertexAttribIntPointer(const GLuint& index, const int& size, const GLuint& offset, const GLenum& type = GL_INT) {
            glEnableVertexArrayAttrib(m_RendererID, index);
            glVertexArrayAttribIFormat(m_RendererID, index, size, type, offset);
            glVertexArrayAttribBinding(m_RendererID, index, 0);
        }

        void AddVertexBuffer(const GLuint& VBO, const GLuint& stride, const GLuint& offset = 0, const GLuint& binding = 0) {
            glVertexArrayVertexBuffer(m_RendererID, binding, VBO, offset, stride);
        }
        
        void AddVertexBuffers(const GLuint& first, const GLsizei& count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides) {
            glVertexArrayVertexBuffers(m_RendererID, first, count, buffers, offsets, strides);
        }

        void AddIndexBuffer(const GLuint& EBO) {
            glVertexArrayElementBuffer(m_RendererID, EBO);
        }

        void Bind() const { glBindVertexArray(m_RendererID); }

        static void Unbind() { glBindVertexArray(0); }

        void Destroy() { glDeleteVertexArrays(1, &m_RendererID); }

        inline operator GLuint& () { return m_RendererID; }
        inline operator const GLuint& () const { return m_RendererID; }
    private:
        GLuint m_RendererID = 0;
    };
}