#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>

namespace gl {

	struct TextureProps {
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint8_t mipMapLevel = 4;
		int internalFormat = 0;
		uint32_t format = 0;
		uint32_t type = 0;
		const void* data = nullptr;
		int min_filter = 0;
		int mag_filter = 0;
		int wrap_s = 0;
		int wrap_t = 0;

		void SetSize(const glm::ivec2& size) { Width = size.x; Height = size.y; }
	};	

	class Texture {		
	public:
		Texture() {}
		~Texture() { Destroy(); }		

		void Create(const unsigned char* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_UNSIGNED_BYTE); }
		void Create(const char* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_BYTE); }
		void Create(const float* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_FLOAT); }
		void Create(const int* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_INT); }
		void Create(const unsigned int* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_UNSIGNED_INT); }
		void Create(const short* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_SHORT); }
		void Create(const unsigned short* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents = 3, const uint8_t& mipMapLevel = 4) { CreateMode(data, Width, Height, nrComponents, mipMapLevel, GL_UNSIGNED_SHORT); }

		void Create(const TextureProps& props) {
			if (!m_RendererID) {
				glGenTextures(1, &m_RendererID);
				glBindTexture(GL_TEXTURE_2D, m_RendererID);
				glTexImage2D(GL_TEXTURE_2D, 0, props.internalFormat, props.Width, props.Height, 0, props.format, props.type, props.data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, props.min_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, props.mag_filter);
				if (props.wrap_s) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, props.wrap_s);
				if (props.wrap_t) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, props.wrap_t);
				if (props.mipMapLevel) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, props.mipMapLevel);
			}
		}


		void SetData(const unsigned char* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_BYTE); }
		void SetData(const char* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_BYTE); }
		void SetData(const float* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_FLOAT); }
		void SetData(const int* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_INT); }
		void SetData(const unsigned int* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_INT); }
		void SetData(const short* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_SHORT); }
		void SetData(const unsigned short* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset = 0, const uint32_t& yoffset = 0) const { SetDataMode(data, width, height, xoffset, yoffset, GL_UNSIGNED_SHORT); }

		void Destroy() const { glDeleteTextures(1, &m_RendererID); }

		void Bind(const uint8_t& ActiveTexture = 0) const {
			glActiveTexture(GL_TEXTURE0 + ActiveTexture);
			glBindTexture(GL_TEXTURE_2D, m_RendererID);
		}

		void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
		uint32_t GetRendererID() const { return m_RendererID; }
		glm::vec2 GetSize() const { 
			int w, h;
			Bind();
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
			return glm::vec2(w, h); 
		}

	private:
		uint32_t m_RendererID = 0;
		
		uint32_t GetFormat() const {
			int32_t format = 0;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
			return format;
		}

		void CreateMode(const void* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& nrComponents, const uint8_t& mipMapLevel, const uint32_t& type) {
			if (!m_RendererID) {
				int32_t format = 0;
				if (nrComponents == 1) format = GL_RED;
				else if (nrComponents == 3)	format = GL_RGB;
				else if (nrComponents == 4) format = GL_RGBA;

				glGenTextures(1, &m_RendererID);
				glBindTexture(GL_TEXTURE_2D, m_RendererID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, format, type, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapLevel);
			}
		}

		void SetDataMode(const void* data, const uint32_t& width, const uint32_t& height, const uint32_t& xoffset, const uint32_t& yoffset, const uint32_t& type) const {
			if (m_RendererID) {
				Bind();
				glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GetFormat(), type, data);

				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
			}
		}
	};

	void load(const char* path, Texture& tex) {
		int fwidth, fheight, fnrComponents;
		unsigned char* data = stbi_load(path, &fwidth, &fheight, &fnrComponents, 0);

		if (data) tex.Create(data, fwidth, fheight, fnrComponents);
		else WC_ERROR("Could not open file location at path {0}!", path);

		stbi_image_free(data);
	}
}
#endif