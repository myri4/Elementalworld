#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>
#undef min

namespace gl {

	struct TextureProps {
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint8_t mipMapLevel = 1;
		uint8_t mips = 1;
		int internalFormat = GL_RGB8;
		uint32_t format = GL_RGB;
		uint32_t type = GL_UNSIGNED_BYTE;
		const void* data = nullptr;
		int min_filter = GL_LINEAR;
		int mag_filter = GL_LINEAR;
		int wrap_s = GL_REPEAT;
		int wrap_t = GL_REPEAT;

		void SetSize(const glm::ivec2& size) { Width = size.x; Height = size.y; }
	};

	class Texture {
	public:
		Texture() {}
		//~Texture() { Destroy(); }		

		inline void Create(const unsigned char* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_UNSIGNED_BYTE); }
		inline void Create(const char* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_BYTE); }
		inline void Create(const float* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_FLOAT); }
		inline void Create(const int* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_INT); }
		inline void Create(const unsigned int* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_UNSIGNED_INT); }
		inline void Create(const short* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_SHORT); }
		inline void Create(const unsigned short* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3) { CreateMode(data, Width, Height, nrComponents, GL_UNSIGNED_SHORT); }

		void Create(const TextureProps& props) {
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

			glTextureStorage2D(m_RendererID, props.mips, props.internalFormat, props.Width, props.Height);
			Parameteri(GL_TEXTURE_MIN_FILTER, props.min_filter);
			Parameteri(GL_TEXTURE_MAG_FILTER, props.mag_filter);
			Parameteri(GL_TEXTURE_WRAP_S, props.wrap_s);
			Parameteri(GL_TEXTURE_WRAP_T, props.wrap_t);

			if (props.data) glTextureSubImage2D(m_RendererID, 0, 0, 0, props.Width, props.Height, props.format, props.type, props.data);
		}

		// @TODO: Remove
		void CreateRml(const glm::ivec2& size, const void* data) {
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			GLint filter = GL_LINEAR;
			if (size.x < 100 || size.y < 100) filter = GL_NEAREST;
			glTextureStorage2D(m_RendererID, 1, GL_RGBA8, size.x, size.y);
			Parameteri(GL_TEXTURE_MIN_FILTER, filter);
			Parameteri(GL_TEXTURE_MAG_FILTER, filter);
			Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		void Parameterf(const GLenum& pname, const GLfloat& param)  const { glTextureParameterf(m_RendererID, pname, param); }
		void Parameterfv(const GLenum& pname, const GLfloat* params) const { glTextureParameterfv(m_RendererID, pname, params); }
		void Parameteri(const GLenum& pname, const GLint& param)    const { glTextureParameteri(m_RendererID, pname, param); }
		void ParameterIiv(const GLenum& pname, const GLint* params)   const { glTextureParameterIiv(m_RendererID, pname, params); }
		void ParameterIuiv(const GLenum& pname, const GLuint* params)  const { glTextureParameterIuiv(m_RendererID, pname, params); }
		void Parameteriv(const GLenum& pname, const GLint* param)    const { glTextureParameteriv(m_RendererID, pname, param); }

		void SetData(const unsigned char* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_BYTE); }
		void SetData(const char* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_BYTE); }
		void SetData(const float* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_FLOAT); }
		void SetData(const int* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_INT); }
		void SetData(const unsigned int* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_INT); }
		void SetData(const short* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_SHORT); }
		void SetData(const unsigned short* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { if (m_RendererID) SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_SHORT); }

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

		void CreateMode(const void* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents, const uint32_t& type) {
			int format = GL_RGB;
			int internalFormat = GL_RGB8;
			if (nrComponents == 1) { format = GL_RED; internalFormat = GL_R8; }
			else if (nrComponents == 4) { format = GL_RGBA; internalFormat = GL_RGBA8; }

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTextureStorage2D(m_RendererID, 1, internalFormat, Width, Height);
			glTextureSubImage2D(m_RendererID, 0, 0, 0, Width, Height, format, type, data);

			glGenerateTextureMipmap(m_RendererID);
		}

		void SetDataMode(const void* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset, const uint32_t& yoffset, const uint32_t& type) const {
			glTextureSubImage2D(m_RendererID, 0, xoffset, yoffset, width, height, GetInternalFormat(), type, data); // Fix GetInternalFormat
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