#pragma once

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
			int32_t width, height, nrComponents = 1;
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

			for (uint32_t i = 0; i < 6; i++) {
				auto* data = stbi_load(faces[i], &width, &height, &nrComponents, 0);
				if (data) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetFormat(nrComponents), width, height, 0, GetFormat(nrComponents), GL_UNSIGNED_BYTE, data);
				else  
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetFormat(nrComponents), width, height, 0, GetFormat(nrComponents), GL_UNSIGNED_BYTE, nullptr);				

				stbi_image_free(data);
			}
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);		
	}
	void Bind(const uint32_t& unit = 0) {
		glBindTextureUnit(unit, m_RendererID);
	}
	static void Unbind() {
		glBindTextureUnit(0, 0);
	}
	inline operator uint32_t& () { return m_RendererID; }
	inline operator const uint32_t& () const { return m_RendererID; }
private:
	uint32_t GetFormat(const int32_t& nrComponents) {
		uint32_t format = 0;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3)	format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;
		return format;
	}
	uint32_t m_RendererID = 0;
};
}