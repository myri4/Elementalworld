#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>

namespace gl {

    class FrameBuffer {
    public:
        FrameBuffer() {}

        ~FrameBuffer() { Destroy(); }
        void Create(uint32_t width, uint32_t height) {
            glGenFramebuffers(1, &m_RendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

            // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
            uint32_t rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

            // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) WC_ERROR("Framebuffer not complete!");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void addTexture(const uint32_t& texture) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + numTextures, GL_TEXTURE_2D, texture, 0);
            numTextures++;
		}

        void setUpDrawBuffers() {
            uint32_t attachments[32];
            for (uint8_t i = 0; i < numTextures; i++) attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            glDrawBuffers(numTextures, attachments);
        }

        void Destroy() { glDeleteFramebuffers(1, &m_RendererID); }

        void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); }

        void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

        operator uint32_t() const { return m_RendererID; }
    private:
        uint32_t m_RendererID = 0, numTextures = 0;
    };
}
#endif