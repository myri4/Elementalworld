#pragma once
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <Utils/Log.hpp>

namespace gl{
	
enum class TextureStatus { OK, COULD_NOT_OPEN_TEXTURE_FILE };
class Texture{
public:
	Texture() {}
	Texture(const char* path) { load(path); }

	TextureStatus load(const char* path) {
		if (!m_RendererID) {
			int nrComponents;
			glGenTextures(1, &m_RendererID);

			auto* data = stbi_load(path, &width, &height, &nrComponents, 0);

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
		else return TextureStatus::OK;
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

	std::array<glm::vec2, 4> GetSpriteIndexCoords(const glm::vec2& coords, const glm::vec2& sprSize) {
		return {
			glm::vec2((coords.x * sprSize.x) / width, (coords.y * sprSize.y) / height),
			glm::vec2((coords.x * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, (coords.y * sprSize.y) / height)
		};
	}
	//std::array<glm::vec2, 4> GetSpriteIndexCoords(const glm::vec2& coords, const glm::vec2& sprSize) {
	//	return {
	//		glm::vec2((coords.x * sprSize.x) / width, (coords.y * sprSize.y) / height),
	//		glm::vec2((coords.x * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
	//		glm::vec2(((coords.x + 1) * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
	//		glm::vec2(((coords.x + 1) * sprSize.x) / width, (coords.y * sprSize.y) / height)
	//	};
	//}
		int width = 0, height = 0;
private:
	uint32_t m_RendererID = 0;	
};

class Cubemap {
public:
	Cubemap() {}
	Cubemap(const std::array<const char*, 6>& faces) { Create(faces); }
	~Cubemap() { glDeleteTextures(1, &m_RendererID); }
	void Create(const std::array<const char*, 6>& faces) {
		if (!m_RendererID) {
			int32_t width, height, nrChannels;
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

			for (uint32_t i = 0; i < faces.size(); i++) {
				auto* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
				if (data)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

				else
					WC_ERROR("Cubemap texture failed to load at path: {0}", faces[i]);

				stbi_image_free(data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	uint32_t m_RendererID = 0;
};

class TextureArray {
public:
	TextureArray() {}

	void Create() {
		if (!m_RendererID) {
			//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);

			glGenTextures(1, &m_RendererID);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);

			glTexStorage3D(GL_TEXTURE_2D_ARRAY,
				5,                    //5 mipmaps
				GL_RGBA,               //Internal format
				width, height,           //width,height
				256                   //Number of layers
			);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}
	}

	~TextureArray() { glDeleteTextures(1, &m_RendererID); }

	TextureStatus AddTexture(const char* filePath) {
		int nrComponents = 0;
		auto* data = stbi_load(filePath, &width, &height, &nrComponents, 0);
		if (data) {
			uint32_t format = 0;
			if (nrComponents == 1) format = GL_RED;
			else if (nrComponents == 3)	format = GL_RGB;
			else if (nrComponents == 4) format = GL_RGBA;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				0,                      //Mipmap number
				0, 0, m_Textures, //xoffset, yoffset, zoffset
				width, height, 1,          //width, height, depth
				GL_RGBA,                 //format
				GL_UNSIGNED_BYTE,       //type
				data); //pointer to data
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -1);
			m_Textures++;
			return TextureStatus::OK;
		}
	}

	void Bind() {
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
	}

	std::array<glm::vec2, 4> GetSpriteIndexCoords(const glm::vec2& coords, const glm::vec2& sprSize) {
		return {
			glm::vec2((coords.x * sprSize.x) / width, (coords.y * sprSize.y) / height),
			glm::vec2((coords.x * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
			glm::vec2(((coords.x + 1) * sprSize.x) / width, (coords.y * sprSize.y) / height)
		};
	}

	void unbind() { glBindTexture(GL_TEXTURE_2D_ARRAY, 0); }
	uint32_t GetRendererID() { return m_RendererID; }
	int width = 32, height = 32;

private:
	uint32_t m_RendererID = 0;
	uint32_t m_Textures = 0;
};
}