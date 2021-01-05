#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <Utils/Log.hpp>

namespace gl{

class Texture{
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
			glBindTexture(GL_TEXTURE_2D, 0);			
		}
	}
	

	void SetData(const void *data, const int& width, const int& height, const int& xoffset = 0, const int& yoffset = 0) {
		if (m_RendererID){
			Bind();
			glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GetFormat(), GL_UNSIGNED_BYTE, data);
		}
	}

	void Destroy() {
		glDeleteTextures(1, &m_RendererID);
	}

	void Bind(const uint32_t& ActiveTexture = 0) {
		glActiveTexture(GL_TEXTURE0 + ActiveTexture);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
	uint32_t GetRendererID() { return m_RendererID; }
	const glm::vec2& GetSize() { return glm::vec2(width, height); }

	std::array<glm::vec2, 4> GetSpriteIndexCoords(const glm::vec2& coords, const glm::vec2& sprSize) {
		return {
			glm::vec2((coords.x * sprSize.x) / width, (coords.y * sprSize.y) / height),
			glm::vec2((coords.x * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, (coords.y * sprSize.y) / height)
		};
	}
private:

	uint32_t GetFormat() {
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
class Cubemap {
public:
	Cubemap() {}
	Cubemap(const std::array<const char*, 6>& faces) { Create(faces); }
	~Cubemap() { glDeleteTextures(1, &m_RendererID); }
	void Create(const std::array<const char*, 6>& faces) {
		if (!m_RendererID) {
			int32_t width, height;
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

			for (uint32_t i = 0; i < faces.size(); i++) {
				auto* data = stbi_load(faces[i], &width, &height, &nrComponents, 0);
				if (data) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetFormat(), width, height, 0, GetFormat(), GL_UNSIGNED_BYTE, data);
				else WC_ERROR("Cubemap texture failed to load at path: {0}", faces[i]);

				stbi_image_free(data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
	}
	void Bind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
	}
	void Unbind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	uint32_t GetRendererID() { return this->m_RendererID; }
private:
	uint32_t GetFormat() {
		uint32_t format = 0;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3)	format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;
		return format;
	}
	uint32_t m_RendererID = 0;
	int32_t nrComponents = 1;
};
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