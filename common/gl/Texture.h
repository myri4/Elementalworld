#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include "glErrors.h"

namespace gl{
	enum class TextureStatus {
		OK, COULD_NOT_OPEN_TEXTURE_FILE
};
class Texture{
public:
	Texture(const char* path, bool flipped = false) { load(path, flipped); }
	Texture() { }
	TextureStatus load(const char* path, bool flipped = false) {
		int nrComponents;
		glCall(glGenTextures(1, &m_RendererID));

		auto data = stbi_load(path, &width, &height, &nrComponents, 0);
		stbi_set_flip_vertically_on_load(flipped);
		
		if (data)
		{
			GLenum format;
			if (nrComponents == 1) format = GL_RED;
			else if (nrComponents == 3)	format = GL_RGB;
			else if (nrComponents == 4) format = GL_RGBA;

			glCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
			glCall(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
			glCall(glGenerateMipmap(GL_TEXTURE_2D));

			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCall(glBindTexture(GL_TEXTURE_2D, 0));
			return TextureStatus::OK;
		}
		else
		{
			uint32_t white = 0xffffffff;
			glCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
			glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white));
			glCall(glGenerateMipmap(GL_TEXTURE_2D));

			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCall(glBindTexture(GL_TEXTURE_2D, 0));
			return TextureStatus::COULD_NOT_OPEN_TEXTURE_FILE;
		}
		
		stbi_image_free(data);
	}
	~Texture() {glCall(glDeleteTextures(1, &m_RendererID));}
	void Bind(uint32_t ActiveTexture = 0) {
		glCall(glActiveTexture(GL_TEXTURE0 + ActiveTexture));
		glCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
	}
	void unbind() { glCall(glBindTexture(GL_TEXTURE_2D, 0)); }
	int width, height;
	uint32_t GetRendererID() { return m_RendererID; }
private:
	uint32_t m_RendererID;	
};
}