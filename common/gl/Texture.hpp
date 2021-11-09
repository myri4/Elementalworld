#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>

namespace gl {

	struct TextureProps {
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint8_t mipMapLevel = 4;
		int internalFormat = GL_RGB8;
		uint32_t format = GL_RGB;
		uint32_t type = GL_UNSIGNED_BYTE;
		const void* data = nullptr;
		int min_filter = GL_LINEAR;
		int mag_filter = GL_LINEAR;
		int wrap_s = GL_REPEAT;
		int wrap_t = GL_REPEAT;
		uint32_t samples = 0;

		void SetSize(const glm::ivec2& size) { Width = size.x; Height = size.y; }
	};

	class Texture {
	public:
		Texture() {}
		//~Texture() { Destroy(); }		

		inline void Create(const unsigned char* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_UNSIGNED_BYTE); }
		inline void Create(const char* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_BYTE); }
		inline void Create(const float* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_FLOAT); }
		inline void Create(const int* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_INT); }
		inline void Create(const unsigned int* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_UNSIGNED_INT); }
		inline void Create(const short* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_SHORT); }
		inline void Create(const unsigned short* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, GL_UNSIGNED_SHORT); }

		void Create(const TextureProps& props) {
			if (props.samples)
				glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_RendererID);
			else {
				glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

				glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, props.min_filter);
				glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, props.mag_filter);
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, props.wrap_s);
				glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, props.wrap_t);
				glTextureParameteri(m_RendererID, GL_TEXTURE_MAX_LEVEL, props.mipMapLevel);
			}

			if (props.samples)
				glTextureStorage2DMultisample(m_RendererID, 1, props.internalFormat, props.Width, props.Height, true);
			else
				glTextureStorage2D(m_RendererID, 1, props.internalFormat, props.Width, props.Height);

			if (props.data) glTextureSubImage2D(m_RendererID, 0, 0, 0, props.Width, props.Height, props.format, props.type, props.data);
		}


		void SetData(const unsigned char* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_BYTE); }
		void SetData(const char* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_BYTE); }
		void SetData(const float* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_FLOAT); }
		void SetData(const int* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_INT); }
		void SetData(const unsigned int* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_INT); }
		void SetData(const short* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_SHORT); }
		void SetData(const unsigned short* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_SHORT); }

		void Destroy() const { glDeleteTextures(1, &m_RendererID); }

		void Bind(const uint8_t& textureUnit = 0) const {
			glBindTextureUnit(textureUnit, m_RendererID);
		}

		static void unbind() { glBindTextureUnit(0, 0); }

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }

		glm::ivec2 GetSize() const {
			int w, h;
			glGetTextureLevelParameteriv(m_RendererID, 0, GL_TEXTURE_WIDTH, &w);
			glGetTextureLevelParameteriv(m_RendererID, 0, GL_TEXTURE_HEIGHT, &h);
			return glm::ivec2(w, h);
		}

		uint32_t GetFormat() const {
			int format = 0;
			glGetTextureLevelParameteriv(m_RendererID, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
			return format;
		}

	private:
		uint32_t m_RendererID = 0;

		void CreateMode(const void* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents, const uint32_t& type) {
			int format = GL_RGB;
			int internalFormat = GL_RGB8;
			if (nrComponents == 1) { format = GL_RED; internalFormat = GL_R8; }
			else if (nrComponents == 4) { format = GL_RGBA; internalFormat = GL_RGBA8; }

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glGenerateTextureMipmap(m_RendererID);

			glTextureStorage2D(m_RendererID, 1, internalFormat, Width, Height);
			glTextureSubImage2D(m_RendererID, 0, 0, 0, Width, Height, format, type, data);
		}

		void SetDataMode(const void* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset, const uint32_t& yoffset, const uint32_t& type) const {
			glTextureSubImage2D(m_RendererID, 0, xoffset, yoffset, width, height, GetFormat(), type, data);

			glGenerateTextureMipmap(m_RendererID);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
	};

	void load(const char* path, Texture& tex) {
		int fwidth, fheight, fnrComponents;
		auto data = stbi_load(path, &fwidth, &fheight, &fnrComponents, 0);

		if (data) tex.Create(data, fwidth, fheight, fnrComponents);
		else WC_ERROR("Could not open file location at path {0}!", path);

		delete data; // stbi free
	}
}