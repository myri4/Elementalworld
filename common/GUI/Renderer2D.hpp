#pragma once

#include <Renderer/Renderer.hpp>
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>

namespace wc {

	static const uint32_t MaxQuadCount = 100;
	static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
	static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;

	static const uint8_t MaxTextures = 32;

	struct Vertex2D {
		glm::vec2 Position = glm::vec2(0.f);
		glm::vec3 TexCoords = glm::vec3(0.f);

		Vertex2D() {};
		Vertex2D(const glm::vec2& pos, const glm::vec3& texCoords) : Position(pos), TexCoords(texCoords) {}
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
			gl::Buffer m_VBO;
			gl::VertexArray m_VAO;
			gl::Buffer m_EBO;

			gl::Shader m_Shader;
			glm::vec2 windowSize = glm::vec2();
		} m_Data;

		void Init() {
			// Quad Rendering
			m_Data.m_EBO.Create(nullptr, sizeof(uint32_t) * MaxQuadIndexCount, GL_DYNAMIC_STORAGE_BIT);
			m_Data.m_VAO.Create();
			m_Data.m_VBO.Create(nullptr, MaxQuadVertexCount * sizeof(Vertex2D), GL_DYNAMIC_STORAGE_BIT);
			m_Data.m_VAO.VertexAttribPointer(0, 2, offsetof(Vertex2D, Position));
			m_Data.m_VAO.VertexAttribPointer(1, 3, offsetof(Vertex2D, TexCoords));
			m_Data.m_VAO.AddVertexBuffer(m_Data.m_VBO, sizeof(Vertex2D));
			m_Data.m_VAO.AddIndexBuffer(m_Data.m_EBO);
			m_Data.m_Shader.Create("resourcepacks/default/shaders/Renderer2D.glsl");
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

		uint32_t getTexture(const uint32_t& texID) {
			uint32_t textureIndex = 0;

			for (uint32_t i = 0; i < m_Data.TextureSlotIndex; i++) {
				if (m_Data.TextureSlots[i] == texID) {
					textureIndex = i;
					break;
				}
			}

			if (textureIndex == 0) {
				textureIndex = m_Data.TextureSlotIndex;
				m_Data.TextureSlots[m_Data.TextureSlotIndex] = texID;
				m_Data.TextureSlotIndex++;
			}
			return textureIndex;
		}		

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const uint32_t& texID) {
			if (m_Data.IndexCount >= MaxQuadIndexCount || m_Data.TextureSlotIndex >= MaxTextures) Flush();

			float textureIndex = getTexture(texID);

			Vertex2D vertices[4];

			vertices[0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { 1.f, 0.f, textureIndex });
			vertices[1] = Vertex2D({ pos.x,			 pos.y + size.y }, { 0.f, 0.f, textureIndex });
			vertices[2] = Vertex2D({ pos.x,			 pos.y, }, { 0.f, 1.f, textureIndex });
			vertices[3] = Vertex2D({ pos.x + size.x, pos.y, }, { 1.f, 1.f, textureIndex });

			for (uint8_t i = 0; i < 4; i++) {
				glm::vec2& Pos = vertices[i].Position;
				Pos = ((vertices[i].Position / m_Data.windowSize) * 2.f - 1.f);
				Pos.y = -Pos.y;
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
	}
}