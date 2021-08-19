#ifndef TEXTURE_ARRAY_HPP
#define TEXTURE_ARRAY_HPP

#include <glad/glad.h>
#include <stb_image/stb_image.h>

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
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);

			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GetFormat(), width, height, MaxTextureSize, 0, GetFormat(), GL_UNSIGNED_BYTE, nullptr);
			//glTextureStorage3D(m_RendererID, 1, GetFormat(), width, height, MaxTextureSize);
		}
	}

	~TextureArray() { glDeleteTextures(1, &m_RendererID); }

	void AddTexture(const void* data) {
		if (m_Textures <= MaxTextureSize) {
			glTextureSubImage3D(m_RendererID, 0, 0, 0, m_Textures, width, height, 1, GetFormat(), GL_UNSIGNED_BYTE, data);

			glGenerateTextureMipmap(m_RendererID);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_NEAREST_MIPMAP_LINEAR
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			m_Textures++;
		}
	}

	void Bind() { glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID); }
	static void unbind() { glBindTexture(GL_TEXTURE_2D_ARRAY, 0); }

	inline operator uint32_t& () { return m_RendererID; }
	inline operator const uint32_t& () const { return m_RendererID; }

	uint32_t GetGeneretedTextures() { return m_Textures; }
private:
	uint32_t width = 32, height = 32;
	uint8_t nrComponents = 4;
	uint32_t GetFormat() {
		if (nrComponents == 1) return GL_RED;
		else if (nrComponents == 3)	return GL_RGB;
		else if (nrComponents == 4) return GL_RGBA;
		return 0;
	}

	uint32_t MaxTextureSize = 1;
	uint32_t m_RendererID = 0;
	uint32_t m_Textures = 0;
};
}

#endif