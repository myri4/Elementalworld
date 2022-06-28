#pragma once

#include <glad/glad.h>
#include <stb_image/stb_image.h>

namespace gl {
	class TextureArray {
		uint32_t width = 32, height = 32;

		uint32_t MaxTextureSize = 1;
		uint32_t m_RendererID = 0;
		uint32_t m_Textures = 0;
	public:
		TextureArray() {}

		void Create(const uint32_t& arraySize, const uint32_t& Width, const uint32_t& Height) {
			width = Width;
			height = Height;
			MaxTextureSize = arraySize;
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
			glTextureStorage3D(m_RendererID, 1, GL_RGBA8, width, height, MaxTextureSize);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_NEAREST_MIPMAP_LINEAR
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			float amount = 0.f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &amount);
			amount = glm::min(amount, 4.f);
			glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, amount);
		}

		~TextureArray() { glDeleteTextures(1, &m_RendererID); }

		void AddTexture(const void* data) {
			glTextureSubImage3D(m_RendererID, 0, 0, 0, m_Textures, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			m_Textures++;			
		}

		void GenerateMipmap() {
			glGenerateTextureMipmap(m_RendererID);
		}

		void Bind(const uint32_t& unit = 0) { glBindTextureUnit(unit, m_RendererID); }

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }

		uint32_t GetGeneretedTextures() { return m_Textures; }
	};
}