#pragma once

#include <glad/glad.h>
#include <stdint.h>

namespace gl {

    class VertexBuffer {
    public:
        VertexBuffer() {}
        VertexBuffer(const void* data, const uint32_t& size, const uint32_t& mode = GL_STATIC_DRAW) { Create(data, size, mode); }
        ~VertexBuffer() { Destroy(); }

        void Bind() {
            glBindVertexArray(VAO);
        }
        void BindVBO() {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
        }
        void Unbind() { glBindVertexArray(0); }

        void Create(const void* data, const uint32_t& size, const uint32_t& mode = GL_STATIC_DRAW) {
            if (!VAO){
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glBufferData(GL_ARRAY_BUFFER, size, data, mode);
            }
        }

        void Reload(const void* data, const uint32_t& size, const uint32_t& offset = 0) {
            BindVBO();
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, &data);
        }
        void Destroy() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }
        void Update(int offset, size_t size, const void *data) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
        }
        uint32_t GetVAO() {
            return VAO;
        }
        uint32_t GetVBO() {
            return VBO;
        }
    private:
        uint32_t VAO = 0, VBO = 0;
    };
    void VertexAttribPointer(const uint32_t& index, const int& size, const int& stride, const void* pointer, const uint32_t& type = GL_FLOAT, const bool& normalized = false) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }
}