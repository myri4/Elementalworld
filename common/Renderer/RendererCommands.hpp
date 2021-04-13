#ifndef RENDERER_COMMANDS_HPP
#define RENDERER_COMMANDS_HPP
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>

static void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
    switch (source)
    {
        //case GL_DEBUG_SOURCE_API:             WC_INFO("Source: API"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   WC_INFO("Source: Window System"); break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: WC_INFO("Source: Shader Compiler"); break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     WC_INFO("Source: Third Party"); break;
    case GL_DEBUG_SOURCE_APPLICATION:     WC_INFO("Source: Application"); break;
    case GL_DEBUG_SOURCE_OTHER:           WC_INFO("Source: Other"); break;
    }
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        WC_ERROR("[OpenGL Debug ERROR] {0}", message);
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        WC_WARN("[OpenGL Debug MEDIUM] {0}", message);
        break;

    case GL_DEBUG_SEVERITY_LOW:
        WC_INFO("[OpenGL Debug LOW] {0}", message);
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        //WC_TRACE("[OpenGL Debug NOTIFICATION] {0}", message);
        break;
    }
}

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

        void enableDebuging() {
            int flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
                // initialize debug output 
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(OpenGLDebugMessege, nullptr);
                //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            }
        }
    }    
}
#endif