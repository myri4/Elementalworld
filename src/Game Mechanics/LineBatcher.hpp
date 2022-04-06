#pragma once
#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>
#include <GUI/Renderer2D.hpp>

namespace wc {

	//static const uint32_t MaxLineCount = 100;
	static const uint32_t MaxLineVertexCount = 100 * 2;

	struct LineVertex {
		glm::vec3 pos;
		glm::vec4 color; // @Temp
	};

	class LineBatcher {
	public: // Variables
		uint32_t IndexCount = 0;
		uint32_t byteOffset = 0;

		gl::Buffer lineBuffer;
		gl::VertexArray lineArray;
		gl::Shader shader;

	public:
		void Create() {
			shader.Create("resourcepacks/default/shaders/Line3D.glsl");

			lineArray.Create();
			lineBuffer.Create(nullptr, MaxLineVertexCount * sizeof(LineVertex), GL_DYNAMIC_STORAGE_BIT);
			lineArray.AddVertexBuffer(lineBuffer, sizeof(LineVertex));
			lineArray.VertexAttribPointer(0, 3, offsetof(LineVertex, pos));
			lineArray.VertexAttribPointer(1, 4, offsetof(LineVertex, color));
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
}