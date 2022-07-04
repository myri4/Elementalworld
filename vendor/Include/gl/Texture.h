#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <Utils/Log.h>
#undef min

namespace gl {

	struct TextureProps {
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint8_t mips = 1;
		int internalFormat = GL_RGB8;
		int min_filter = GL_LINEAR;
		int mag_filter = GL_LINEAR;
		int wrap_s = GL_REPEAT;
		int wrap_t = GL_REPEAT;

		void SetSize(const glm::ivec2& size) { Width = size.x; Height = size.y; }
	};

	class Texture {
	public:
		Texture() {}

		inline void Create(const float* data, const uint32_t& Width, const uint32_t& Height) {
			TextureProps props;
			props.Width = Width;
			props.Height = Height;
			props.mag_filter = GL_NEAREST;
			props.min_filter = GL_NEAREST;
			props.wrap_s = GL_CLAMP_TO_EDGE;
			props.wrap_t = GL_CLAMP_TO_EDGE;

			Create(props);
			glTextureSubImage2D(m_RendererID, 0, 0, 0, Width, Height, GL_RGB, GL_FLOAT, data);
		}

		void Create(const TextureProps& props) {
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

			glTextureStorage2D(m_RendererID, props.mips, props.internalFormat, props.Width, props.Height);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, props.min_filter);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, props.mag_filter);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, props.wrap_s);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, props.wrap_t);
		}

		void Destroy() {
			glDeleteTextures(1, &m_RendererID);
			m_RendererID = 0;
		}

		void Bind(const uint8_t& textureUnit = 0) const { glBindTextureUnit(textureUnit, m_RendererID); }

		void BindTextureImage(const uint8_t& textureUnit = 0, const GLenum& access = GL_READ_ONLY, const uint32_t& level = 0) { glBindImageTexture(textureUnit, m_RendererID, level, false, 0, access, GetInternalFormat()); }

		glm::ivec2 GetMipSize(int level)
		{
			glm::ivec2 size = GetSize();
			while (level != 0)
			{
				size.x /= 2;
				size.y /= 2;
				level--;
			}

			return size;
		}

		int GetMipLevelCount()
		{
			glm::vec2 textureSize = GetSize();
			return (int)glm::floor(glm::log2(glm::min(textureSize.x, textureSize.y)));
		}

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }

		glm::ivec2 GetSize() const {
			int w, h;
			glGetTextureLevelParameteriv(m_RendererID, 0, GL_TEXTURE_WIDTH, &w);
			glGetTextureLevelParameteriv(m_RendererID, 0, GL_TEXTURE_HEIGHT, &h);
			return glm::ivec2(w, h);
		}

		uint32_t GetInternalFormat() const {
			int format = 0;
			glGetTextureLevelParameteriv(m_RendererID, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
			return format;
		}

		void GenerateMipMap() { glGenerateTextureMipmap(m_RendererID); }

	private:
		uint32_t m_RendererID = 0;
	};

	void load(const char* path, Texture& tex) {
		int fnrComponents;
		TextureProps props;
		props.min_filter = GL_NEAREST_MIPMAP_LINEAR;
		props.min_filter = GL_NEAREST;
		props.wrap_s = GL_CLAMP_TO_EDGE;
		props.wrap_t = GL_CLAMP_TO_EDGE;

		auto data = stbi_load(path, (int32_t*)&props.Width, (int32_t*)&props.Height, &fnrComponents, 0);

		if (data) { 
			int format = GL_RGB;
			if (fnrComponents == 4) { props.internalFormat = GL_RGBA8; format = GL_RGBA; }

			tex.Create(props);

			glTextureSubImage2D(tex, 0, 0, 0, props.Width, props.Height, format, GL_UNSIGNED_BYTE, data);
		}
		else WC_ERROR("Could not open file location at path {0}!", path);

		delete data; // stbi free
	}
}