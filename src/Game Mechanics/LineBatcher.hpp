#ifndef LINEBATCHER_HPP
#define LINEBATCHER_HPP

#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>
#include <GUI/Renderer2D.hpp>

namespace wc {

	struct LineVertex {
		glm::vec3 pos;
		glm::vec4 color; // @Temp
	};

	class LineBatcher {
	public: // Variables
		uint32_t IndexCount = 0;
		uint32_t byteOffset = 0;

		gl::VertexBuffer lineBuffer;
		gl::VertexArray lineArray;
		gl::Shader shader;

	public:
		void Create() {
			shader.Create("shaderpacks/default/Line3D.glsl");

			lineArray.Create();
			lineBuffer.Create(nullptr, MaxLineVertexCount * sizeof(LineVertex), GL_DYNAMIC_DRAW);
			Renderer::VertexAttribPointer(0, 3, sizeof(LineVertex), (const void*)offsetof(LineVertex, pos));
			Renderer::VertexAttribPointer(1, 4, sizeof(LineVertex), (const void*)offsetof(LineVertex, color));
		}

		void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f)) {
			if (IndexCount >= MaxLineVertexCount) Flush();

			float vertices[] = {
				// positions
				start.x, start.y, start.z, color.r, color.g, color.b, color.a,
				end.x,   end.y,   end.z  , color.r, color.g, color.b, color.a
			};

			lineBuffer.SetData(byteOffset, sizeof(vertices), vertices);
			byteOffset += sizeof(vertices);
			IndexCount += 2;
		}

		void Flush() {
			if (!IndexCount) return;
			shader.use();

			lineArray.Bind();
			Renderer::DrawArrays(IndexCount, 0, GL_LINES);
			IndexCount = 0;
			byteOffset = 0;
		}

		LineBatcher() {	}
	};

	struct QuadVertex {
		glm::vec3 pos;
		glm::vec4 color; // @Temp
	};

	class QuadBatcher {
	public: // Variables
		uint32_t IndexCount = 0;
		uint32_t byteOffset = 0;

		gl::VertexBuffer quadBuffer;
		gl::VertexArray quadArray;
		gl::Shader shader;

	public:
		void Create() {
			shader.Create("shaderpacks/default/Line3D.glsl");

			quadArray.Create();
			quadBuffer.Create(nullptr, MaxLineVertexCount * sizeof(LineVertex), GL_DYNAMIC_DRAW);
			Renderer::VertexAttribPointer(0, 3, sizeof(LineVertex), (const void*)offsetof(LineVertex, pos));
			Renderer::VertexAttribPointer(1, 4, sizeof(LineVertex), (const void*)offsetof(LineVertex, color));
		}

		void DrawQuad(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f)) {
			if (IndexCount >= MaxLineVertexCount) Flush();

			float vertices[] = {
				// positions
				start.x, start.y, start.z, color.r, color.g, color.b, color.a,
				end.x,   end.y,   end.z  , color.r, color.g, color.b, color.a
			};

			quadBuffer.SetData(byteOffset, sizeof(vertices), vertices);
			byteOffset += sizeof(vertices);
			IndexCount += 2;
		}

		void Flush() {
			if (!IndexCount) return;
			shader.use();

			quadArray.Bind();
			Renderer::DrawArrays(IndexCount, 0, GL_LINES);
			IndexCount = 0;
			byteOffset = 0;
		}

		QuadBatcher() {	}
	};
}
#endif