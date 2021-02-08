#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <Utils/Log.hpp>

namespace gl {

	struct TextureProps {
		uint32_t Width;
		uint32_t Height;
		uint8_t mipMapLevel = 4;
		int internalFormat;
		uint32_t format;
		uint32_t type;
		const void* data;
		int min_filter;
		int mag_filter;
		int wrap_s;
		int wrap_t;
	};

	class Texture {
	public:
		Texture() {}
		~Texture() { Destroy(); }

		void load(const char* path) {
			int fwidth, fheight, fnrComponents;
			auto* data = stbi_load(path, &fwidth, &fheight, &fnrComponents, 0);

			if (data) Create(data, fwidth, fheight, fnrComponents);
			else WC_ERROR("Could not open file location at path {0}!", path);

			stbi_image_free(data);
		}

		void Create(const void* data, const uint32_t& Width, const uint32_t& Height, const uint8_t& NrComponents = 3, uint8_t mipMapLevel = 4) {
			if (!m_RendererID) {
				width = Width;
				height = Height;
				nrComponents = NrComponents;
				glGenTextures(1, &m_RendererID);
				glBindTexture(GL_TEXTURE_2D, m_RendererID);
				glTexImage2D(GL_TEXTURE_2D, 0, GetFormat(), width, height, 0, GetFormat(), GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapLevel);
			}
		}

		void Create(const TextureProps& props) {
			if (!m_RendererID) {
				width = props.Width;
				height = props.Height;
				glGenTextures(1, &m_RendererID);
				glBindTexture(GL_TEXTURE_2D, m_RendererID);
				glTexImage2D(GL_TEXTURE_2D, 0, props.internalFormat, width, height, 0, props.format, props.type, props.data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, props.min_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, props.mag_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, props.wrap_s);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, props.wrap_t);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, props.mipMapLevel);
			}
		}


		void SetData(const void* data, const int& width, const int& height, const int& xoffset = 0, const int& yoffset = 0) const {
			if (m_RendererID) {
				Bind();
				glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GetFormat(), GL_UNSIGNED_BYTE, data);

				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
			}
		}

		void Destroy() {
			glDeleteTextures(1, &m_RendererID);
		}

		void Bind(const uint32_t& ActiveTexture = 0) const {
			glActiveTexture(GL_TEXTURE0 + ActiveTexture);
			glBindTexture(GL_TEXTURE_2D, m_RendererID);
		}

		void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
		uint32_t GetRendererID() const { return m_RendererID; }
		glm::vec2 GetSize() const { return glm::vec2(width, height); }

	private:

		uint32_t GetFormat() const {
			uint32_t format = 0;
			if (nrComponents == 1) format = GL_RED;
			else if (nrComponents == 3)	format = GL_RGB;
			else if (nrComponents == 4) format = GL_RGBA;
			return format;
		}
		uint8_t nrComponents = 3;
		uint32_t width = 0, height = 0;
		uint32_t m_RendererID = 0;
	};
}
#endif