#ifndef RENDERER_COMMANDS_HPP
#define RENDERER_COMMANDS_HPP
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace wc {
    namespace Renderer {

        void VertexAttribPointer(const uint32_t& index, const int& size, const GLsizei& stride, const void* pointer, const GLenum& type = GL_FLOAT, const bool& normalized = false) {
            glVertexAttribPointer(index, size, type, normalized, stride, pointer);
            glEnableVertexAttribArray(index);
        }

        void VertexAttribIntPointer(const uint32_t& index, const int& size, const GLsizei& stride, const void* pointer, const GLenum& type = GL_INT) {
            glVertexAttribIPointer(index, size, type, stride, pointer);
            glEnableVertexAttribArray(index);
        }
        
        void DrawIndexed(const uint32_t& IndexCount, const uint32_t& mode = GL_TRIANGLES, const uint32_t& type = GL_UNSIGNED_INT, const void* indices = nullptr){
            glDrawElements(mode, IndexCount, type, indices);
        }

        void DrawArrays(const uint32_t& count, const uint32_t& first = 0, const uint32_t& mode = GL_TRIANGLES) {
            glDrawArrays(mode, first, count);
        }

        void Clear(const GLbitfield& mask = GL_COLOR_BUFFER_BIT) {
            glClear(mask);
        }

        void setClearColor(const glm::vec4& color) {
            glClearColor(color.r, color.g, color.b, color.a);
        }
    }    
}
#endif