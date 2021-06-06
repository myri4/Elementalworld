#ifndef CUBE_MAP_HPP
#define CUBE_MAP_HPP

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>
namespace gl {
class Cubemap {
public:
	Cubemap() {}
	~Cubemap() { glDeleteTextures(1, &m_RendererID); }
	void Create(const char** faces) {
		if (!m_RendererID) {
			int32_t width, height;
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

			for (uint32_t i = 0; i < 6; i++) {
				auto* data = stbi_load(faces[i], &width, &height, &nrComponents, 0);
				if (data) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetFormat(), width, height, 0, GetFormat(), GL_UNSIGNED_BYTE, data);
				else WC_ERROR("Cubemap texture failed to load at path: {0}", faces[i]);

				stbi_image_free(data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
	}
	void Bind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
	}
	void Unbind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	operator uint32_t() const { return m_RendererID; }
private:
	uint32_t GetFormat() {
		uint32_t format = 0;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3)	format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;
		return format;
	}
	uint32_t m_RendererID = 0;
	int32_t nrComponents = 1;
};
}
#endif