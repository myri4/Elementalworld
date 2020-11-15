#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glad/glad.h>
#include <stdint.h>
#include <glm/glm.hpp>

namespace gl {

    class VertexBuffer {
    public:
        VertexBuffer() {}
        VertexBuffer(const void* data, const GLsizeiptr& size, const GLenum& mode = GL_STATIC_DRAW) { Create(data, size, mode); }
        ~VertexBuffer() { Destroy(); }

        //void BindVAO() {
        //    glBindVertexArray(VAO);
        //}
        void Bind() {
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        }
        void Unbind() { glBindVertexArray(0); }

        void Create(const void* data, const GLsizeiptr& size, const GLenum& mode = GL_STATIC_DRAW) {
            //if(!VAO) glGenVertexArrays(1, &VAO);
            if (!m_RendererID){
                glGenBuffers(1, &m_RendererID);

                //glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

                glBufferData(GL_ARRAY_BUFFER, size, data, mode);
            }
        }

        void Destroy() {
            //glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &m_RendererID);
        }

        void Update(const GLintptr& offset, const GLsizeiptr& size, const void *data) {
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

       //uint32_t GetVAO() {
       //    return VAO;
       //}

        uint32_t GetVBO() {
            return m_RendererID;
        }
    private:
        uint32_t m_RendererID = 0; // VAO = 0
    };

    class VertexArray {
    public:
        VertexArray() {

        }
        ~VertexArray(){
            glDeleteVertexArrays(1, &m_RendererID);
        }

        void Create() {
            if(!m_RendererID) glGenVertexArrays(1, &m_RendererID);
        }

        void VertexAttribPointer(const uint32_t& index, const int& size, const GLsizei& stride, const void* pointer, const GLenum& type = GL_FLOAT, const bool& normalized = false) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        }

        void Bind() {
            glBindVertexArray(m_RendererID);
        }

        void unind() {
            glBindVertexArray(0);
        }

        uint32_t GetRendererID() {
            return m_RendererID;
        }
    private:
        uint32_t m_RendererID = 0;
    };

    class Vertex {
    public:
        glm::vec3 Position = { 0,0,0 };
        glm::vec2 TexCoords = { 0,0 };
        Vertex() {}
        Vertex(const Vertex& vertex) : Position(vertex.Position), TexCoords(vertex.TexCoords) {}
        Vertex(const glm::vec3& pos, const glm::vec2& texCoord) : Position(pos), TexCoords(texCoord) {}
        ~Vertex() {}
    };

    void VertexAttribPointer(const uint32_t& index, const int& size, const GLsizei& stride, const void* pointer, const GLenum& type = GL_FLOAT, const bool& normalized = false) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }
}
#endif