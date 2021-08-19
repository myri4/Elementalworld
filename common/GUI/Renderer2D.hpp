#ifndef RENDERER2D_HPP
#define RENDERER2D_HPP
#include <string>

#include <Renderer/Renderer.hpp>
#include <gl/Shaders.hpp>
#include <ft2build.h>
#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>
#include FT_FREETYPE_H

namespace wc {

	static const uint32_t MaxQuadCount = 1000;
	static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
	static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;

	static const uint32_t MaxLineCount = 100;
	static const uint32_t MaxLineVertexCount = MaxQuadCount * 2;

	static const uint8_t MaxTextures = 32;

	struct Vertex2D {
		glm::vec2 Position;
		glm::vec3 TexCoords;
		uint32_t Color;
		float Type;

		Vertex2D() {};
		Vertex2D(const glm::vec2& pos, const glm::vec3& texCoords, const uint32_t& color, const float& type) : Position(pos), TexCoords(texCoords), Color(color), Type(type) {}
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

		struct Data {
			//Quad Rendering
			uint32_t IndexCount = 0;
			uint32_t TextureSlots[MaxTextures] = { 0 };
			uint32_t byteOffset = 0;
			uint8_t TextureSlotIndex = 1;
			gl::Texture whiteTexture;
			gl::VertexBuffer m_VBO;
			gl::VertexArray m_VAO;
			gl::IndexBuffer m_EBO;

			gl::VertexBuffer m_LineVBO;
			gl::VertexArray m_LineVAO;
			uint32_t lineByteOffset = 0;
			uint32_t LineIndexCount = 0;

			gl::Shader m_Shader;
		} m_Data;

		void Init(const bool& lines = false) {
			// Quad Rendering
			uint32_t indices[MaxQuadIndexCount];
			uint32_t offset = 0;

			for (uint32_t i = 0; i < MaxQuadIndexCount; i += 6) {
				indices[i + 0] = offset;
				indices[i + 1] = 1 + offset;
				indices[i + 2] = 2 + offset;

				indices[i + 3] = 2 + offset;
				indices[i + 4] = 3 + offset;
				indices[i + 5] = offset;

				offset += 4;
			}

			m_Data.m_EBO.Create(indices, sizeof(indices));
			m_Data.m_VAO.Create();
			m_Data.m_VBO.Create(nullptr, MaxQuadVertexCount * sizeof(Vertex2D), GL_DYNAMIC_DRAW);
			Renderer::VertexAttribPointer(0, 2, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Position));
			Renderer::VertexAttribPointer(1, 3, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, TexCoords));
			Renderer::VertexAttribPointer(2, 1, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Color));
			Renderer::VertexAttribPointer(3, 1, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Type));


			m_Data.m_LineVAO.Create();
			m_Data.m_LineVBO.Create(nullptr, MaxLineVertexCount * sizeof(Vertex2D), GL_DYNAMIC_DRAW);
			Renderer::VertexAttribPointer(0, 2, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Position));
			Renderer::VertexAttribPointer(1, 3, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, TexCoords));
			Renderer::VertexAttribPointer(2, 1, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Color));
			Renderer::VertexAttribPointer(3, 1, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, Type));

			m_Data.m_Shader.Create("shaderpacks/default/Renderer2D.glsl");

			int32_t samplers[MaxTextures];
			for (uint8_t i = 0; i < MaxTextures; i++) samplers[i] = i;
			m_Data.m_Shader.SetArray(0, MaxTextures, samplers);

			float color[] = { 1.f, 1.f, 1.f };
			m_Data.whiteTexture.Create(color, 1, 1);
			m_Data.TextureSlots[0] = m_Data.whiteTexture;

			for (uint8_t i = 1; i < MaxTextures; i++) m_Data.TextureSlots[i] = 0;
		}

		void SetProjection(const glm::mat4& proj) {
			m_Data.m_Shader.setMat4(0, proj);
		}

		void FlushLines() {
			if (!m_Data.LineIndexCount) return;
			m_Data.m_Shader.use();

			m_Data.m_LineVAO.Bind();
			Renderer::DrawArrays(m_Data.LineIndexCount, 0, GL_LINES);
			m_Data.LineIndexCount = 0;
			m_Data.lineByteOffset = 0;
		}

		void Flush() {
			if (!m_Data.IndexCount) return;
			m_Data.m_Shader.use();

			for (uint8_t i = 0; i < m_Data.TextureSlotIndex; i++) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, m_Data.TextureSlots[i]);
			}
			m_Data.m_VAO.Bind();
			m_Data.m_EBO.Bind();
			Renderer::DrawIndexed(m_Data.IndexCount);
			m_Data.IndexCount = 0;
			m_Data.byteOffset = 0;
			m_Data.TextureSlotIndex = 1;
			glActiveTexture(GL_TEXTURE0);
			m_Data.m_EBO.Unbind();
			FlushLines();
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount) Flush();

			//float textureIndex = 0.f;

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);

			Vertex2D vertices[4];
			vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { 1.f, 1.f, 0.f }, Color, 0.f);
			vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { 0.f, 1.f, 0.f }, Color, 0.f);
			vertices[2] = Vertex2D({ pos.x,			 pos.y, }, { 0.f, 0.f, 0.f }, Color, 0.f);
			vertices[3] = Vertex2D({ pos.x + size.x, pos.y, }, { 1.f, 0.f, 0.f }, Color, 0.f);

			m_Data.m_VBO.SetData(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const gl::Texture& tex, const glm::vec2& textureStart, const glm::vec2& textureEnd, const glm::vec4& color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

			float textureIndex = 0.f;
			uint32_t texID = tex;
			for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.f) {
				textureIndex = (float)m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}
			glm::vec2 texSize = tex.GetSize();
			float tsx = 1.f / texSize.x;
			float tsy = 1.f / texSize.y;

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
			Vertex2D vertices[4];
			vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { textureEnd.x   * tsx, textureEnd.y   * tsy, textureIndex }, Color, 0);
			vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { textureStart.x * tsx, textureEnd.y   * tsy, textureIndex }, Color, 0);
			vertices[2] = Vertex2D({ pos.x,			 pos.y, },         { textureStart.x * tsx, textureStart.y * tsy, textureIndex }, Color, 0);
			vertices[3] = Vertex2D({ pos.x + size.x, pos.y, },         { textureEnd.x   * tsx, textureStart.y * tsy, textureIndex }, Color, 0);

			m_Data.m_VBO.SetData(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const uint32_t& texID, const glm::vec4& color = glm::vec4(1.f), const float& Type = 0) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

			float textureIndex = 0.f;
			for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.f) {
				textureIndex = (float)m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
			Vertex2D vertices[4];
			vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { 1.f, 1.f, textureIndex }, Color, Type);
			vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { 0.f, 1.f, textureIndex }, Color, Type);
			vertices[2] = Vertex2D({ pos.x,			 pos.y, }, { 0.f, 0.f, textureIndex }, Color, Type);
			vertices[3] = Vertex2D({ pos.x + size.x, pos.y, }, { 1.f, 0.f, textureIndex }, Color, Type);

			m_Data.m_VBO.SetData(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}

		void DrawQuadIndexedSprite(const glm::vec2& pos, const glm::vec2& size, const gl::Texture& tex, const glm::vec2& coords, const glm::vec2& sprSize, const glm::vec4& color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex > MaxTextures - 1) Flush();

			float textureIndex = 0.f;
			uint32_t texID = tex;
			for (uint8_t i = 1; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.f) {
				textureIndex = (float)m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}

			float tsx = 1.f / tex.GetSize().x;
			float tsy = 1.f / tex.GetSize().y;

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
			Vertex2D vertices[4];
			vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { (coords.x * sprSize.x) * tsx, ((coords.y + 1) * sprSize.y) * tsy, textureIndex }, Color, 0);
			vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { ((coords.x + 1) * sprSize.x) * tsx, ((coords.y + 1) * sprSize.y) * tsy, textureIndex }, Color, 0);
			vertices[2] = Vertex2D({ pos.x,			 pos.y, }, { ((coords.x + 1) * sprSize.x) * tsx, (coords.y * sprSize.y) * tsy, textureIndex }, Color, 0);
			vertices[3] = Vertex2D({ pos.x + size.x, pos.y, }, { (coords.x * sprSize.x) * tsx, (coords.y * sprSize.y) * tsy, textureIndex }, Color, 0);

			/*float vertices[] = {
				// positions                                                              texture coords
				pos.x + size.x, pos.y + size.y, (coords.x * sprSize.x)		  * tsx, ((coords.y + 1) * sprSize.y) * tsy, textureIndex, color.r, color.g, color.b, color.a, 0.0f, // top right
				pos.x,			pos.y + size.y, ((coords.x + 1) * sprSize.x)  * tsx, ((coords.y + 1) * sprSize.y) * tsy, textureIndex, color.r, color.g, color.b, color.a, 0.0f, // top left
				pos.x,			pos.y,			((coords.x + 1) * sprSize.x)  * tsx, (coords.y * sprSize.y)		  * tsy, textureIndex, color.r, color.g, color.b, color.a, 0.0f, // bottom left
				pos.x + size.x, pos.y,			(coords.x * sprSize.x)		  * tsx, (coords.y * sprSize.y)		  * tsy, textureIndex, color.r, color.g, color.b, color.a, 0.0f  // bottom right
			};*/

			m_Data.m_VBO.SetData(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.IndexCount += 6;
		}
#undef DrawText
		void DrawText(const std::string& text, const Font& font, glm::vec2 pos = glm::vec2(0.f), const float& scale = 0.4f, const glm::vec4& color = glm::vec4(1.f)) {

			for (auto& c : text) {
				Character ch = font.Characters[c];

				float xpos = pos.x + ch.Bearing.x * scale;
				float ypos = pos.y - ch.Bearing.y * scale;

				float w = ch.Size.x * scale;
				float h = ch.Size.y * scale;
				// update VBO for each character
				Renderer2D::DrawQuad({ xpos, ypos }, { w,h }, ch.TextureID, color, 1.f);

				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				pos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
			}
		}

		void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color = glm::vec4(1.f)) {
			if (m_Data.LineIndexCount >= MaxLineVertexCount) FlushLines();
			/*
			float vertices[] = {
				// positions
				start.x, start.y, 0.f, 0.f, 0.f, color.r, color.g, color.b, color.a, 2.f,
				end.x,   end.y,   0.f, 0.f, 0.f, color.r, color.g, color.b, color.a, 2.f
			};
			*/

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
			Vertex2D vertices[2];
			vertices[0] = Vertex2D(start, glm::vec3(0.f), Color, 2.f);
			vertices[1] = Vertex2D(end, glm::vec3(0.f), Color, 2.f);

			m_Data.m_LineVBO.SetData(m_Data.lineByteOffset, sizeof(vertices), vertices);
			m_Data.lineByteOffset += sizeof(vertices);
			m_Data.LineIndexCount += 2;
		}

		void DrawLineDC(const glm::vec2& start, const glm::vec3& end, const glm::vec4& startColor = glm::vec4(1.f), const glm::vec4& endColor = glm::vec4(1.f)) {
			if (m_Data.LineIndexCount >= MaxLineVertexCount) FlushLines();
			/*
			float vertices[] = {
				// positions
				start.x, start.y, 0.f, 0.f, 0.f, 0.f, startColor.r, startColor.g, startColor.b, startColor.a, 2.f,
				end.x,   end.y,   0.f, 0.f, 0.f, 0.f, endColor.r,   endColor.g,   endColor.b,   endColor.a, 2.f
			};
			*/

			uint32_t ColorStart, colorEnd;
			Vertex2D vertices[2];
			vertices[0] = Vertex2D(start, glm::vec3(0.f), ColorStart, 2.f);
			vertices[1] = Vertex2D(end, glm::vec3(0.f), colorEnd, 2.f);

			m_Data.m_LineVBO.SetData(m_Data.lineByteOffset, sizeof(vertices), vertices);
			m_Data.lineByteOffset += sizeof(vertices);
			m_Data.LineIndexCount += 2;
		}

		void SetLineWidth(const float& width) { glLineWidth(width); }

		glm::mat4 Get2DProj(const glm::vec2& windowSize) { return glm::ortho(0.f, windowSize.x, windowSize.y, 0.f); }
	}
}
#endif