#pragma once

#include <glad/glad.h>

namespace gl {

    class FrameBuffer {
    public:
        FrameBuffer() {}

        //~FrameBuffer() { Destroy(); }
        void Create(const uint32_t& width, const uint32_t& height, const uint32_t& samples = 0) {
            glCreateFramebuffers(1, &m_RendererID);

            // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
            uint32_t rbo;
            glCreateRenderbuffers(1, &rbo);
            if (samples)
                glNamedRenderbufferStorageMultisample(rbo, samples, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
            else
                glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
            glNamedFramebufferRenderbuffer(m_RendererID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

            // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
            if (glCheckNamedFramebufferStatus(m_RendererID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) WC_ERROR("Framebuffer not complete!");
        }

        void addTexture(const uint32_t& texture) {
            glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0 + numTextures, texture, 0);
            numTextures++;
		}

        void setUpDrawBuffers() {
            uint32_t attachments[32];
            for (uint8_t i = 0; i < numTextures; i++) attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            glNamedFramebufferDrawBuffers(m_RendererID, numTextures, attachments);
        }

        void blit(const uint32_t& width, const uint32_t& height) {
            glBlitNamedFramebuffer(m_RendererID, 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        void Destroy() { glDeleteFramebuffers(1, &m_RendererID); numTextures = 0; }

        void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); }

        static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

        inline operator uint32_t& () { return m_RendererID; }
        inline operator const uint32_t& () const { return m_RendererID; }
    private:
        uint32_t m_RendererID = 0, numTextures = 0;
    };
}