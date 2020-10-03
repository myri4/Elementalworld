#pragma once

#include <wclibs/pch.hpp>

namespace gl {
    class IndexBuffer {
    public:
        IndexBuffer() {}
        IndexBuffer(const uint32_t* data, uint32_t size, uint32_t mode = GL_STATIC_DRAW) { Create(data, size, mode); }
        ~IndexBuffer() { glDeleteBuffers(1, &EBO); }

        void Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); }
        void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

        void Create(const uint32_t* data, uint32_t size, uint32_t mode = GL_STATIC_DRAW) {
            if (!EBO){
                glGenBuffers(1, &EBO);
            Bind();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, mode);
            Unbind();
            }
        }
        uint32_t GetEBO() {
            return EBO;
        }
    private:
        uint32_t EBO;
    };
}
