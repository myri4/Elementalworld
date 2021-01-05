#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>

namespace wc {
	class Quad {
	private:
		glm::vec2 size;
		glm::vec2 pos;
		glm::vec2 startPos;
		glm::vec2 endPos;
	public:
		void SetSize(const glm::vec2& size) { this->size = size; }
		void SetPos(const glm::vec2& pos) { this->pos = pos; }
		void SetSpriteRect(const glm::vec2& startPos, const glm::vec2& endPos) { this->startPos = startPos; this->endPos = endPos; }
		glm::vec2 GetSize() { return size; }
		glm::vec2 GetPos() { return pos; }
		glm::vec2 GetSpriteStart() { return startPos; }
		glm::vec2 GetSpriteEnd() { return endPos; }
	};

	class Renderer2D {
	private:
		gl::VertexBuffer rVBO;
		gl::VertexArray rVAO;
		gl::IndexBuffer rEBO;
		gl::Shader rShader;
		uint32_t indexCount;
	public:
		void Create() {
			uint32_t indicies[] = { 0, 1, 2, 2, 3, 0 };
			rEBO.Create(indicies, sizeof(indicies));
			rVAO.Create();
			int maxSize = 1000; // bytes
			rVBO.Create(nullptr, maxSize);
			rVAO.VertexAttribPointer(0, 2, 5 * sizeof(float), (void*)0);
			rVAO.VertexAttribPointer(1, 3, 5 * sizeof(float), (void*)(2 * sizeof(float)));
			rShader.Create("shaderpacks/default/2DRendererShader.glsl");
		}

		void DrawQuad(Quad& quad, gl::Texture& tex, float index = 1) {
			float vertices[] = {
				// positions  // texture coords
				quad.GetPos().x + quad.GetSize().x, quad.GetPos().y + quad.GetSize().y,  quad.GetSpriteEnd().x   / tex.GetSize().x, quad.GetSpriteStart().y / tex.GetSize().y, index,  // top right
				quad.GetPos().x + quad.GetSize().x, quad.GetPos().y,					 quad.GetSpriteEnd().x   / tex.GetSize().x, quad.GetSpriteEnd().y   / tex.GetSize().y, index,  // bottom right
				quad.GetPos().x,					quad.GetPos().y,					 quad.GetSpriteStart().x / tex.GetSize().x, quad.GetSpriteEnd().y   / tex.GetSize().y, index,  // bottom left
				quad.GetPos().x,					quad.GetPos().y + quad.GetSize().y,  quad.GetSpriteStart().x / tex.GetSize().x, quad.GetSpriteStart().y / tex.GetSize().y, index  // top left 
			};
			rVBO.Update(0, sizeof(vertices), vertices);
		}

		void Draw(const glm::mat4& proj) {
			rShader.use();
			rShader.setMat4("proj", proj);
			rVAO.Bind();
			rEBO.Bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		}
	};
}

