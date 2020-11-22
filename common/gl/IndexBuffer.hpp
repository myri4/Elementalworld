#pragma once

#include <wclibs/pch.hpp>

namespace gl {
    class IndexBuffer {
    public:
        IndexBuffer() {}
        IndexBuffer(const void* data, const uint32_t& size, const uint32_t& mode = GL_STATIC_DRAW) { Create(data, size, mode); }
        ~IndexBuffer() { glDeleteBuffers(1, &m_RendererID); }


        void Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); }
        void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

        void Create(const void* data, const uint32_t& size, const uint32_t& mode = GL_STATIC_DRAW) {
            if (!m_RendererID){
                glGenBuffers(1, &m_RendererID);
                Bind();
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, mode);
                Unbind();
            }
        }
        uint32_t GetRendererID() {return m_RendererID;}
    private:
        uint32_t m_RendererID = 0;
    };
}
