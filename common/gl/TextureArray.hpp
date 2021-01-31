#ifndef TEXTURE_ARRAY_HPP
#define TEXTURE_ARRAY_HPP

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>

namespace gl {
class TextureArray {
public:
	TextureArray() {}

	void Create(const uint32_t& arraySize, const uint32_t& Width, const uint32_t& Height, const uint8_t& NrOfComp) {
		width = Width;
		height = Height;
		nrComponents = NrOfComp;
		MaxTextureSize = arraySize;
		if (!m_RendererID) {
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);

			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GetFormat(), width, height, MaxTextureSize, 0, GetFormat(), GL_UNSIGNED_BYTE, nullptr);

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
		}
	}

	~TextureArray() { glDeleteTextures(1, &m_RendererID); }

	void AddTexture(const void* data) {
		if (m_Textures <= MaxTextureSize) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, m_Textures, width, height, 1, GetFormat(), GL_UNSIGNED_BYTE, data);

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
			m_Textures++;
		}
	}

	void Bind() { glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID); }
	void unbind() { glBindTexture(GL_TEXTURE_2D_ARRAY, 0); }

	uint32_t GetRendererID() { return m_RendererID; }
	uint32_t GetGeneretedTextures() { return m_Textures; }

private:
	uint32_t width = 32, height = 32;
	uint8_t nrComponents = 4;
	uint32_t GetFormat() {
		if (nrComponents == 1) return GL_RED;
		else if (nrComponents == 3)	return GL_RGB;
		else if (nrComponents == 4) return GL_RGBA;

	}

	uint32_t MaxTextureSize = 1;
	uint32_t m_RendererID = 0;
	uint32_t m_Textures = 0;
};
}

#endif