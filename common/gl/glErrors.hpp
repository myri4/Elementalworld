#ifndef GL_ERRORS_HPP
#define GL_ERRORS_HPP
#include <glad/glad.h>
#include <Utils/Log.hpp>

void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
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

void EnableGLDebuging() {
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT){
        // initialize debug output 
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLDebugMessege, nullptr);
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}
#endif