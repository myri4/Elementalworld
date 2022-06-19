#pragma once

#include <glad/glad.h>

namespace gl {

    class Fence {
    protected:
        GLsync m_RendererID = 0;
    public:
        Fence() = default;

        void lock() {
            if (m_RendererID)            
                glDeleteSync(m_RendererID);
            
            m_RendererID = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }

        void wait()
        {
            if (m_RendererID)            
                while (1) {
                    GLenum waitReturn = glClientWaitSync(m_RendererID, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
                    if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) return;
                }            
        }
    };
}