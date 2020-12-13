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
        virtual void Create(uint32_t width, uint32_t height) {
            glGenFramebuffers(1, &m_RendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

            // create a color attachment texture
            glGenTextures(1, &m_ColorAttachment);
            glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

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
        void Destroy() { glDeleteFramebuffers(1, &m_RendererID); }

        void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); }

        void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

        void BindTexture() { glBindTexture(GL_TEXTURE_2D, m_ColorAttachment); } // use the color attachment texture as the texture of the quad plane

        uint32_t GetRendererID() { return m_RendererID; }

        uint32_t GetColorAttachment() { return m_ColorAttachment; }
    protected:
        uint32_t m_RendererID = 0, m_ColorAttachment = 0;
    };

    class ShadowMap : FrameBuffer {
    public:
        void Create(uint32_t width, uint32_t height) override {
            glGenFramebuffers(1, &m_RendererID);

            glGenTextures(1, &m_ColorAttachment);
            glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ColorAttachment, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) WC_ERROR("Framebuffer not complete!");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };
}
#endif