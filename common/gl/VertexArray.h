#pragma once

#include <glad/glad.h>
#include <stdint.h>

namespace gl {

    class VertexBuffer {
    public:
        VertexBuffer() {}
        VertexBuffer(const void* data, uint32_t size, uint32_t mode = GL_STATIC_DRAW) { Create(data, size, mode); }
        ~VertexBuffer() { Destroy(); }

        void Bind() {
            glBindVertexArray(VAO);
        }
        void BindVBO() {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
        }
        void Unbind() { glBindVertexArray(0); }

        void Create(const void* data, uint32_t size, uint32_t mode = GL_STATIC_DRAW) {
            if (!VAO){
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glBufferData(GL_ARRAY_BUFFER, size, data, mode);
            }
        }

        void Reload(const void* data, uint32_t size, uint32_t offset = 0) {
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
        uint32_t VAO, VBO;
    };
    void VertexAttribPointer(uint32_t index, int size, int stride, const void* pointer, uint32_t type = GL_FLOAT, bool normalized = false) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }
}