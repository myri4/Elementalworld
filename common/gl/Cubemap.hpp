#pragma once

#include <glad/glad.h>
#include <vector>
#include <stb_image/stb_image.h>
#include <Utilitiess/Log.hpp>

namespace gl {
	class Cubemap{
	public:
        Cubemap() {}
		Cubemap(std::vector<const char*> faces) {Create(faces);}
        ~Cubemap() {
            glDeleteTextures(1, &m_RendererID);
        }
        void Create(std::vector<const char*> faces) {
        int32_t width, height, nrChannels;
            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

            for (uint32_t i = 0; i < faces.size(); i++){
                auto* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
                if (data)
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                
                else
                    WC_ERROR("Cubemap texture failed to load at path: {0}", faces[i]);
                
                    stbi_image_free(data);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
        }
        void Bind() {
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        }
        void Unbind() {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
        uint32_t GetRendererID() { return this->m_RendererID; }
	private:
        uint32_t m_RendererID = 0;
	};
 }