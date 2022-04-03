#pragma once
#include <string>

#include <ft2build.h>
#include <Renderer/Renderer.hpp>
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>
#include <Utils/Window.hpp>
#include FT_FREETYPE_H

namespace wc {

	static const uint32_t MaxQuadCount = 1000;
	static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
	static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;

	static const uint32_t MaxLineCount = 100;
	static const uint32_t MaxLineVertexCount = MaxLineCount * 2;

	static const uint8_t MaxTextures = 32;

	struct Vertex2D {
		glm::vec2 Position = glm::vec2(0.f);
		glm::vec3 TexCoords = glm::vec3(0.f);
		uint32_t Color = 0xff;
		float Type = 0.f;

		Vertex2D() {};
		Vertex2D(const glm::vec2& pos, const glm::vec3& texCoords, const uint32_t& color, const float& type) : Position(pos), TexCoords(texCoords), Color(color), Type(type) {}
	};

	struct Character {
		Character() {}
		Character(const glm::ivec2& Size, const glm::ivec2& Bearing, const uint32_t& Advance) : Size(Size), Bearing(Bearing), Advance(Advance) {}
		gl::Texture texture;     // ID handle of the glyph texture
		glm::ivec2   Size = glm::ivec2(0);      // Size of glyph
		glm::ivec2   Bearing = glm::ivec2(0);   // Offset from baseline to left/top of glyph
		uint32_t Advance = 0;       // Horizontal offset to advance to next glyph
	};

	struct Font {
		Character Characters[128];
		void Load(const char* fontFileLoc) {
			// FreeType
			// --------
			// disable byte-alignment restriction
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			FT_Library ft;
			// All functions return a value different than 0 whenever an error occurred
			if (FT_Init_FreeType(&ft)) WC_ERROR("Could not init freetype library!");

			// load font as face
			FT_Face face;
			if (FT_New_Face(ft, fontFileLoc, 0, &face)) WC_ERROR("Failed to load font!");
			else {
				// set size to load glyphs as
				FT_Set_Pixel_Sizes(face, 0, 48);

				// load first 128 characters of ASCII set
				for (uint8_t c = 0; c < ARRAYSIZE(Characters); c++)
				{
					// Load character glyph 
					if (FT_Load_Char(face, c, FT_LOAD_RENDER)) WC_ERROR("Failed to load glyph!");

					// generate texture
					Characters[c].Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
					Characters[c].Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
					Characters[c].Advance = static_cast<uint32_t>(face->glyph->advance.x);

					gl::TextureProps props;
					props.data = face->glyph->bitmap.buffer;
					props.format = GL_RED;
					props.SetSize(Characters[c].Size);
					props.internalFormat = GL_R8;
					props.mag_filter = GL_LINEAR;
					props.min_filter = GL_LINEAR;
					props.wrap_s = GL_CLAMP_TO_EDGE;
					props.wrap_t = GL_CLAMP_TO_EDGE;
					if (props.Width > 0 && props.Height > 0)
					Characters[c].texture.Create(props);

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
			uint32_t indByteOffset = 0;
			uint32_t iOffset = 0;
			uint8_t TextureSlotIndex = 0;
			gl::Texture whiteTexture;
			gl::Buffer m_VBO;
			gl::VertexArray m_VAO;
			gl::Buffer m_EBO;

			gl::Shader m_Shader;
			glm::vec2 windowSize = glm::vec2();
		} m_Data;

		void Init(const bool& lines = false) {
			// Quad Rendering
			m_Data.m_EBO.Create(nullptr, sizeof(uint32_t) * MaxQuadIndexCount, GL_DYNAMIC_STORAGE_BIT);
			m_Data.m_VAO.Create();
			m_Data.m_VBO.Create(nullptr, MaxQuadVertexCount * sizeof(Vertex2D), GL_DYNAMIC_STORAGE_BIT);
			m_Data.m_VAO.VertexAttribPointer(0, 2, offsetof(Vertex2D, Position));
			m_Data.m_VAO.VertexAttribPointer(1, 3, offsetof(Vertex2D, TexCoords));
			m_Data.m_VAO.VertexAttribPointer(2, 1, offsetof(Vertex2D, Color));
			m_Data.m_VAO.VertexAttribPointer(3, 1, offsetof(Vertex2D, Type));
			m_Data.m_VAO.AddVertexBuffer(m_Data.m_VBO, sizeof(Vertex2D));
			m_Data.m_VAO.AddIndexBuffer(m_Data.m_EBO);
			m_Data.m_Shader.Create("shaderpacks/default/Renderer2D.glsl");

			float color[] = { 1.f, 1.f, 1.f };
			m_Data.whiteTexture.Create(color, 1, 1);

			for (uint8_t i = 0; i < MaxTextures; i++) m_Data.TextureSlots[i] = 0;
		}

		void Flush() {
			if (!m_Data.IndexCount) return;
			m_Data.m_Shader.use();

			for (uint8_t i = 0; i < m_Data.TextureSlotIndex; i++)
				glBindTextureUnit(i, m_Data.TextureSlots[i]);
			
			m_Data.m_VAO.Bind();
			Renderer::DrawIndexed(m_Data.IndexCount);
			m_Data.IndexCount = 0;
			m_Data.byteOffset = 0;
			m_Data.indByteOffset = 0;
			m_Data.iOffset = 0;
			m_Data.TextureSlotIndex = 0;
		}

		float getTexture(const uint32_t& texID) {
			float textureIndex = 0.f;
			for (uint8_t i = 0; i < m_Data.TextureSlotIndex; i++) {
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
			return textureIndex;
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const gl::Texture& tex, const glm::vec2& textureStart, const glm::vec2& textureEnd, const glm::vec4& color = glm::vec4(1.f)) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex >= MaxTextures) Flush();

			float textureIndex = getTexture(tex);
			glm::vec2 texSize = tex.GetSize();
			float tsx = 1.f / texSize.x;
			float tsy = 1.f / texSize.y;

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
			Vertex2D vertices[4];
			vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { textureEnd.x   * tsx, textureEnd.y   * tsy, textureIndex }, Color, 0);
			vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { textureStart.x * tsx, textureEnd.y   * tsy, textureIndex }, Color, 0);
			vertices[2] = Vertex2D({ pos.x,			 pos.y, },         { textureStart.x * tsx, textureStart.y * tsy, textureIndex }, Color, 0);
			vertices[3] = Vertex2D({ pos.x + size.x, pos.y, },         { textureEnd.x   * tsx, textureStart.y * tsy, textureIndex }, Color, 0);

			for (uint8_t i = 0; i < 4; i++) {
				glm::vec2 Pos = ((vertices[i].Position / m_Data.windowSize) * 2.f - 1.f);
				Pos.y = -Pos.y;
				vertices[i].Position = Pos;
			}

			uint32_t indices[6];

			indices[0] = m_Data.iOffset;
			indices[1] = 1 + m_Data.iOffset;
			indices[2] = 2 + m_Data.iOffset;

			indices[3] = 2 + m_Data.iOffset;
			indices[4] = 3 + m_Data.iOffset;
			indices[5] = m_Data.iOffset;

			m_Data.m_VBO.SetData(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.m_EBO.SetData(m_Data.indByteOffset, sizeof(indices), indices);
			m_Data.indByteOffset += sizeof(indices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.iOffset += ARRAYSIZE(vertices);
			m_Data.IndexCount += ARRAYSIZE(indices);
		}
		
		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color = glm::vec4(1.f)) {
			DrawQuad(pos, size, m_Data.whiteTexture, { 0,0 }, {1, 1}, color);
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const uint32_t& texID, const glm::vec4& color = glm::vec4(1.f), const float& Type = 0, const bool& flipped = false) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex >= MaxTextures) Flush();

			float textureIndex = getTexture(texID);

			uint32_t Color = (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
			Vertex2D vertices[4];
			if (flipped) {
				vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { 1.f, 0.f, textureIndex }, Color, Type);
				vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { 0.f, 0.f, textureIndex }, Color, Type);
				vertices[2] = Vertex2D({ pos.x,			 pos.y, }, { 0.f, 1.f, textureIndex }, Color, Type);
				vertices[3] = Vertex2D({ pos.x + size.x, pos.y, }, { 1.f, 1.f, textureIndex }, Color, Type);
			}
			else {
				vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { 1.f, 1.f, textureIndex }, Color, Type);
				vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { 0.f, 1.f, textureIndex }, Color, Type);
				vertices[2] = Vertex2D({ pos.x,			 pos.y, }, { 0.f, 0.f, textureIndex }, Color, Type);
				vertices[3] = Vertex2D({ pos.x + size.x, pos.y, }, { 1.f, 0.f, textureIndex }, Color, Type);
			}
			for (uint8_t i = 0; i < 4; i++) {
				glm::vec2 Pos = ((vertices[i].Position / m_Data.windowSize) * 2.f - 1.f);
				Pos.y = -Pos.y;
				vertices[i].Position = Pos;
			}

			uint32_t indices[6];

			indices[0] = m_Data.iOffset;
			indices[1] = 1 + m_Data.iOffset;
			indices[2] = 2 + m_Data.iOffset;

			indices[3] = 2 + m_Data.iOffset;
			indices[4] = 3 + m_Data.iOffset;
			indices[5] = m_Data.iOffset;

			m_Data.m_VBO.SetData(m_Data.byteOffset, sizeof(vertices), vertices);
			m_Data.m_EBO.SetData(m_Data.indByteOffset, sizeof(indices), indices);
			m_Data.indByteOffset += sizeof(indices);
			m_Data.byteOffset += sizeof(vertices);
			m_Data.iOffset += ARRAYSIZE(vertices);
			m_Data.IndexCount += ARRAYSIZE(indices);
		}
#undef DrawText
		void DrawText(const std::string& text, const Font& font, glm::vec2 pos = glm::vec2(0.f), const float& scale = 0.4f, const glm::vec4& color = glm::vec4(1.f)) {
			for (auto& c : text) {
				const Character& ch = font.Characters[c];

				float xpos = pos.x + ch.Bearing.x * scale;
				float ypos = pos.y - ch.Bearing.y * scale;

				DrawQuad({ xpos, ypos }, glm::vec2(ch.Size) * scale, ch.texture, color, 1.f);

				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				pos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
			}
		}
	}
}