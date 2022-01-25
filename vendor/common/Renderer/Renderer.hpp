#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>

void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
    const char* src;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             src = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     src = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     src = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           src = "Other"; break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        WC_ERROR("[{0}] {1}", src, message);
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        WC_WARN("[{0}] {1}", src, message);
        break;

    case GL_DEBUG_SEVERITY_LOW:
        WC_INFO("[{0}] {1}", src, message);
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
       // WC_TRACE("[{0} {1} TRACE] {2}", src, typeStr, message);
        break;
    }
}

namespace wc {
    namespace Renderer {
        
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
            //if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) 
            {
                // initialize debug output 
                glDebugMessageCallback(OpenGLDebugMessege, nullptr);
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
            }
        }

        void SetLineWidth(const float& width) { glLineWidth(width); }
    }    
}