#pragma once
#include <glad/glad.h>
#include <iostream>

void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "ERROR: " << message << "\n";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "WARN: " << message << "\n";
        break;

        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "INFO: " << message << "\n";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            //std::cout << "NOTIFICATION: " << message << "\n";
            
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