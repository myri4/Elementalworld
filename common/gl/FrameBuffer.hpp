#pragma once
#include <glad/glad.h>

namespace gl{
enum class FrameBufferStatus {
    OK, FRAMEBUFFER_NOT_COMPLETE
};

class FrameBuffer {
public:
    FrameBuffer() {

    }
    FrameBuffer(unsigned int width, unsigned int height) { Create(width, height); }

    ~FrameBuffer() {
        glDeleteFramebuffers(1, &m_RendererID);
    }
    FrameBufferStatus Create(unsigned int width, unsigned int height) {
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
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)  return FrameBufferStatus::FRAMEBUFFER_NOT_COMPLETE;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return FrameBufferStatus::OK;
    }
    void Update(int x, int y) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RendererID);
        glBlitFramebuffer(0, 0, x, y, 0, 0, x, y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    void Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void BindTexture() {
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);	// use the color attachment texture as the texture of the quad plane
    }
    unsigned int GetRendererID() {
        return m_RendererID;
    }
    unsigned int GetColorAttachment() {
        return m_ColorAttachment;
    }
private:
    unsigned int m_RendererID, m_ColorAttachment;
};
}