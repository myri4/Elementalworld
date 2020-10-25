#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <wclibs/Core.hpp>
#include <glm/glm.hpp>
#include <array>

namespace gl{
	
class Texture{
public:
	Texture() {}
	enum class TextureStatus { OK, COULD_NOT_OPEN_TEXTURE_FILE };
	Texture(const char* path) { load(path); }

	TextureStatus load(const char* path) {
		int nrComponents;
		glGenTextures(1, &m_RendererID);

		auto data = stbi_load(path, &width, &height, &nrComponents, 0);

		if (data)
		{
			uint32_t format = 0;
			if (nrComponents == 1) format = GL_RED;
			else if (nrComponents == 3)	format = GL_RGB;
			else if (nrComponents == 4) format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, m_RendererID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
			return TextureStatus::OK;
		}
		else
		{
			uint8_t white = 0xffffffff;
			glBindTexture(GL_TEXTURE_2D, m_RendererID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
			return TextureStatus::COULD_NOT_OPEN_TEXTURE_FILE;
		}

		stbi_image_free(data);
	}
	
	~Texture() {glDeleteTextures(1, &m_RendererID);}

	void SetData(const char* data, const int& x, const int& y) {
		
		if (data) {
			Bind();
			glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}

	void Bind(const uint32_t& ActiveTexture = 0) {
		glActiveTexture(GL_TEXTURE0 + ActiveTexture);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
	uint32_t GetRendererID() { return m_RendererID; }

	std::array<glm::vec2, 4> GetSpriteCoords(const glm::vec2& coords, const glm::vec2& sprSize) {
		return {
			glm::vec2((coords.x * sprSize.x) / width, (coords.y * sprSize.y) / height),
			glm::vec2((coords.x * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, (coords.y * sprSize.y) / height)
		};
	}
		int width = 0, height = 0;
private:
	uint32_t m_RendererID = 0;	
};
}