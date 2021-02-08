#ifndef RENDERER2D_HPP
#define RENDERER2D_HPP
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <gl/Shaders.hpp>
#include <ft2build.h>
#include <gl/IndexBuffer.hpp>
#include <gl/Vertex.hpp>
#include FT_FREETYPE_H

namespace wc {

	static const uint32_t MaxQuadCount = 1000;
	static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
	static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;
	static const uint8_t MaxTextures = 32;

	struct Vertex2D {
		glm::vec2 Position;
		glm::vec3 TexCoords;
		glm::vec4 Color;
	};

	class Character {
	public:
		Character() {}
		Character(const uint32_t& TextureID, const glm::ivec2& Size, const glm::ivec2& Bearing, const uint32_t& Advance) : TextureID(TextureID), Size(Size), Bearing(Bearing), Advance(Advance) {}
		uint32_t TextureID = 0;     // ID handle of the glyph texture
		glm::ivec2   Size = glm::ivec2(0);      // Size of glyph
		glm::ivec2   Bearing = glm::ivec2(0);   // Offset from baseline to left/top of glyph
		uint32_t Advance = 0;       // Horizontal offset to advance to next glyph
	};

	struct Font {
		Character Characters[150];
		void Load(const char* fontFileLoc, const int& glyphs) {
			// FreeType
			// --------
			FT_Library ft;
			// All functions return a value different than 0 whenever an error occurred
			if (FT_Init_FreeType(&ft)) WC_ERROR("Could not init freetype library!");

			// find path to font
			if (fontFileLoc == "") WC_ERROR("Could not find font file location!");

			// load font as face
			FT_Face face;
			if (FT_New_Face(ft, fontFileLoc, 0, &face)) { WC_ERROR("Failed to load font!"); }
			else {
				// set size to load glyphs as
				FT_Set_Pixel_Sizes(face, 0, 48);

				// disable byte-alignment restriction
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				// load first 128 characters of ASCII set
				for (uint8_t c = 0; c < glyphs; c++)
				{
					// Load character glyph 
					if (FT_Load_Char(face, c, FT_LOAD_RENDER)) WC_ERROR("Failed to load glyph!");

					// generate texture
					uint32_t texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
					// set texture options
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					// now store character for later use
					Character character = {
						texture,
						glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
						glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
						static_cast<uint32_t>(face->glyph->advance.x)
					};
					//Characters.insert(std::pair<char, Character>(c, character));
					Characters[c] = character;
				}
			}
			// destroy FreeType once we're finished
			FT_Done_Face(face);
			FT_Done_FreeType(ft);
		}
	};

	namespace Renderer2D {

	static struct RendererData {
		//Quad Rendering
		uint32_t IndexCount = 0;
		uint32_t TextureSlots[MaxTextures];
		uint32_t byteOffset = 0;
		uint8_t TextureSlotIndex = 1;
		gl::Texture whiteTexture;
		gl::VertexBuffer m_VBO;
		gl::VertexArray m_VAO;
		gl::IndexBuffer m_EBO;
		gl::Shader m_Shader;

		// Text Rendering
		gl::Shader m_TextShader;
		gl::VertexBuffer m_TextVBO;
		gl::VertexArray  m_TextVAO;
	};

	static RendererData m_Data;

	static void Init() {
			// Quad Rendering
			uint32_t indices[MaxQuadIndexCount];
			uint32_t offset = 0;

			for (uint32_t i = 0; i < MaxQuadIndexCount; i += 6) {
				indices[i + 0] = 0 + offset;
				indices[i + 1] = 1 + offset;
				indices[i + 2] = 2 + offset;

				indices[i + 3] = 2 + offset;
				indices[i + 4] = 3 + offset;
				indices[i + 5] = 0 + offset;

				offset += 4;
			}

			m_Data.m_EBO.Create(indices, sizeof(indices));
			m_Data.m_VAO.Create();
			m_Data.m_VBO.Create(nullptr, MaxQuadVertexCount * sizeof(Vertex2D), GL_DYNAMIC_DRAW);
			m_Data.m_VAO.VertexAttribPointer(0, 2, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Position));
			m_Data.m_VAO.VertexAttribPointer(1, 3, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, TexCoords));
			m_Data.m_VAO.VertexAttribPointer(2, 4, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Color));
			m_Data.m_Shader.Create("shaderpacks/default/Renderer2D.glsl");

			int32_t samplers[MaxTextures];
			for (uint8_t i = 0; i < MaxTextures; i++) samplers[i] = i;
			m_Data.m_Shader.use();
			m_Data.m_Shader.SetArray("u_Texture", MaxTextures, samplers);

			uint32_t color = 0xffffffff;
			m_Data.whiteTexture.Create(&color, 1, 1);
			m_Data.TextureSlots[0] = m_Data.whiteTexture.GetRendererID();

			for (uint8_t i = 1; i < MaxTextures; i++) m_Data.TextureSlots[i] = 0;

			// Text Rendering
			m_Data.m_TextShader.Create("shaderpacks/default/text.glsl");

			m_Data.m_TextVAO.Create();
			m_Data.m_TextVAO.Bind();
			m_Data.m_TextVBO.Create(nullptr, sizeof(float) * 6 * 4, GL_DYNAMIC_DRAW);
			m_Data.m_TextVAO.VertexAttribPointer(0, 4, 4 * sizeof(float), 0);
		}
	
	static void SetProjection(const glm::mat4& proj) {
			m_Data.m_Shader.use();
			m_Data.m_Shader.setMat4("proj", proj);
	}

	static void SetTextProjection(const glm::mat4& proj) {
			m_Data.m_TextShader.use();
			m_Data.m_TextShader.setMat4("projection", proj);
		}

	static void Flush() {
			m_Data.m_Shader.use();

			for (uint8_t i = 0; i < m_Data.TextureSlotIndex; i++) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, m_Data.TextureSlots[i]);
				//glBindTextureUnit(i, m_Data.TextureSlots[i]);
			}
			m_Data.m_VAO.Bind();
			m_Data.m_EBO.Bind();
			glDrawElements(GL_TRIANGLES, m_Data.IndexCount, GL_UNSIGNED_INT, nullptr);
			m_Data.IndexCount = 0;
			m_Data.byteOffset = 0;
			m_Data.TextureSlotIndex = 1;
			glActiveTexture(GL_TEXTURE0);
			m_Data.m_EBO.Unbind();
		}
	
	static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, glm::vec4 color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount) Flush();

			float textureIndex = 0.f;
			float vertices[] = {
				// positions                                                              texture coords
				pos.x + size.x, pos.y + size.y,  1.f, 0.f, textureIndex, color.r, color.g, color.b, color.a,  // top right
				pos.x + size.x, pos.y,			 1.f, 1.f, textureIndex, color.r, color.g, color.b, color.a, // bottom right
				pos.x,			pos.y,			 0.f, 1.f, textureIndex, color.r, color.g, color.b, color.a, // bottom left
				pos.x,			pos.y + size.y,  0.f, 0.f, textureIndex, color.r, color.g, color.b, color.a// top left 
			};

			m_Data.m_VBO.Update(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

	static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const gl::Texture& tex, const glm::vec2& textureStart, const glm::vec2& textureEnd, glm::vec4 color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

			float textureIndex = 0.f;
			uint32_t texID = tex.GetRendererID();
			for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = (float)texID;
					break;
				}
			}

			if (textureIndex == 0.f) {
				textureIndex = (float)m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}


			float vertices[] = {
					// positions                                                              texture coords
					pos.x + size.x, pos.y + size.y,  textureEnd.x   / tex.GetSize().x, textureStart.y / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a, // top right
					pos.x + size.x, pos.y,			 textureEnd.x   / tex.GetSize().x, textureEnd.y   / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a, // bottom right
					pos.x,			pos.y,			 textureStart.x / tex.GetSize().x, textureEnd.y   / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a, // bottom left
					pos.x,			pos.y + size.y,  textureStart.x / tex.GetSize().x, textureStart.y / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a  // top left 
			};

			m_Data.m_VBO.Update(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

	static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const gl::Texture& tex, glm::vec4 color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

			float textureIndex = 0.f;
			uint32_t texID = tex.GetRendererID();
			for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = (float)texID;
					break;
				}
			}

			if (textureIndex == 0.f) {
				textureIndex = (float)m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}


			float vertices[] = {
				// positions                                                              texture coords
				pos.x + size.x, pos.y + size.y,  1.f, 0.f, textureIndex, color.r, color.g, color.b, color.a,  // top right
				pos.x + size.x, pos.y,			 1.f, 1.f, textureIndex, color.r, color.g, color.b, color.a, // bottom right
				pos.x,			pos.y,			 0.f, 1.f, textureIndex, color.r, color.g, color.b, color.a, // bottom left
				pos.x,			pos.y + size.y,  0.f, 0.f, textureIndex, color.r, color.g, color.b, color.a// top left 
			};

			m_Data.m_VBO.Update(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

	static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const uint32_t& tex, glm::vec4 color = glm::vec4(1.f)) {
		if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

		float textureIndex = 0.f;
		uint32_t texID = tex;
		for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
			if (m_Data.TextureSlots[i] == texID) {
				textureIndex = (float)texID;
				break;
			}
		}

		if (textureIndex == 0.f) {
			textureIndex = (float)m_Data.TextureSlotIndex;
			m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
			m_Data.TextureSlotIndex++;
		}


		float vertices[] = {
			// positions                                                              texture coords
			pos.x + size.x, pos.y + size.y,  1.f, 0.f, textureIndex, color.r, color.g, color.b, color.a,  // top right
			pos.x + size.x, pos.y,			 1.f, 1.f, textureIndex, color.r, color.g, color.b, color.a, // bottom right
			pos.x,			pos.y,			 0.f, 1.f, textureIndex, color.r, color.g, color.b, color.a, // bottom left
			pos.x,			pos.y + size.y,  0.f, 0.f, textureIndex, color.r, color.g, color.b, color.a// top left 
		};

		m_Data.m_VBO.Update(m_Data.byteOffset, sizeof(vertices), vertices);
		m_Data.byteOffset += sizeof(vertices);
		m_Data.IndexCount += 6;
	}

	static void DrawQuadIndexedSprite(const glm::vec2& pos, const glm::vec2& size, const gl::Texture& tex, const glm::vec2& coords, const glm::vec2& sprSize, glm::vec4 color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

			float textureIndex = 0.f;
			uint32_t texID = tex.GetRendererID();
			for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = (float)texID;
					break;
				}
			}

			if (textureIndex == 0.f) {
				textureIndex = (float)m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}


			float vertices[] = {
				// positions                                                              texture coords
				pos.x + size.x, pos.y + size.y,  (coords.x * sprSize.x) / tex.GetSize().x,       (coords.y * sprSize.y) / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a, // top right
				pos.x + size.x, pos.y,			 (coords.x * sprSize.x) / tex.GetSize().x,       ((coords.y + 1) * sprSize.y) / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a, // bottom right
				pos.x,			pos.y,			 ((coords.x + 1) * sprSize.x) / tex.GetSize().x, ((coords.y + 1) * sprSize.y) / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a, // bottom left
				pos.x,			pos.y + size.y,  ((coords.x + 1) * sprSize.x) / tex.GetSize().x, (coords.y * sprSize.y) / tex.GetSize().y, textureIndex, color.r, color.g, color.b, color.a  // top left 
			};

			m_Data.m_VBO.Update(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

	static void DrawTexts(const std::string& text, const Font& font, glm::vec2 pos = glm::vec2(0.0f), const float& scale = 0.4f, const glm::vec4& color = glm::vec4(0.5, 0.8f, 0.2f, 1.f)) {
			// activate corresponding render state	
			m_Data.m_TextShader.use();
			m_Data.m_TextShader.setVec4("textColor", color);
			m_Data.m_TextVAO.Bind();

			// iterate through all characters
			for (auto& c : text) {
				Character ch = font.Characters[c];

				float xpos = pos.x + ch.Bearing.x * scale;
				float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale;

				float w = ch.Size.x * scale;
				float h = ch.Size.y * scale;
				// update VBO for each character
				float vertices[] = {
					xpos,     ypos + h,   0.0f, 0.0f,
					xpos + w, ypos,       1.0f, 1.0f,
					xpos,     ypos,       0.0f, 1.0f,

					xpos,     ypos + h,   0.0f, 0.0f,
					xpos + w, ypos + h,   1.0f, 0.0f,
					xpos + w, ypos,       1.0f, 1.0f,
				};
				// render glyph texture over quad
				glBindTexture(GL_TEXTURE_2D, ch.TextureID);
				// update content of VBO memory
				m_Data.m_TextVBO.Update(0, sizeof(vertices), vertices);

				// render quad
				glDrawArrays(GL_TRIANGLES, 0, 6);
				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				pos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
			}
		}
	}
}
#endif