#pragma once
#include <glad/glad.h>
#include <Utils/Log.hpp>

void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
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
    glDebugMessageCallback(OpenGLDebugMessege, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}

void DisableGLDebuging() {
    glDisable(GL_DEBUG_OUTPUT);
    glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}