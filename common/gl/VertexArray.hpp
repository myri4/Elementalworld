#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <glad/glad.h>
#include <Utils/Log.hpp>
#include <glm/glm.hpp>

namespace gl {

    class VertexArray {
    public:
        VertexArray() {}
        ~VertexArray() { Destroy(); }

        void Create() {
            if (!m_RendererID) glGenVertexArrays(1, &m_RendererID);
            glBindVertexArray(m_RendererID);
        }

        void Bind() const { glBindVertexArray(m_RendererID); }

        void Unind() const { glBindVertexArray(0); }

        void Destroy() { glDeleteVertexArrays(1, &m_RendererID); }

        uint32_t GetRendererID() const { return m_RendererID; }
    private:
        uint32_t m_RendererID = 0;
    };
}
#endif