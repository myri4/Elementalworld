#pragma once
#include <glad/glad.h>
#include <iostream>

void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}
bool GLLogCall(const char* function, const char* file, int line) {

    while (GLenum errorCode = glGetError()) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM";                  break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE";                 break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION";             break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW";                break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW";               break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY";                 break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        //std::cout << "[OpenGL Error] (" << errorCode << "): " << function << " " << file << " " << line << std::endl << "Discription:" << error;
        std::cout << file << "(" << line << "): error " << errorCode << ": " << error << std::endl;
        return false;
    }
    return true;
}

void glCheckError(const char* file, unsigned int line, const char* expression) {
    while (GLenum errorCode = glGetError() != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
}

#define ASSERT(x) if(!(x)) __debugbreak();
#ifdef _DEBUG
#define glCall(x) GLClearError();\
x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__));
#else
#define glCall(x) x;
#endif // DEBUG


