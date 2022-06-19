#pragma once
#include <gl/Buffer.hpp>
#include <gl/VertexArray.hpp>

namespace wc {

	static const uint32_t MaxLineVertexCount = 100 * 2;

	struct LineVertex {
		glm::vec3 pos;
		glm::vec4 color;
	};

	class LineBatcher {
		uint32_t IndexCount = 0;
		uint32_t byteOffset = 0;

		gl::Buffer lineBuffer;
		gl::VertexArray lineArray;
		gl::Shader shader;
	public:
		void Create() {
			shader.Create("resourcepacks/default/shaders/Line3D.vert", "resourcepacks/default/shaders/Line3D.frag");

			lineArray.Create();
			lineArray.VertexAttribPointer(0, 3, offsetof(LineVertex, pos));
			lineArray.VertexAttribPointer(1, 4, offsetof(LineVertex, color));
			lineBuffer.Create(MaxLineVertexCount * sizeof(LineVertex), GL_DYNAMIC_STORAGE_BIT);
			lineArray.SetVertexBuffer(lineBuffer, sizeof(LineVertex));
		}

		void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f)) {
			if (IndexCount >= MaxLineVertexCount) Flush();

			float vertices[] = {
				// positions
				start.x, start.y, start.z, color.r, color.g, color.b, color.a,
				end.x,   end.y,   end.z  , color.r, color.g, color.b, color.a
			};

			lineBuffer.SetData(sizeof(vertices), vertices, byteOffset);
			byteOffset += sizeof(vertices);
			IndexCount += 2;
		}

		void DrawOutlineCube(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color) {
			DrawLine(pos, pos + glm::vec3(0.f, size.y, 0.f), color);
			DrawLine(pos, pos + glm::vec3(size.x, 0.f, 0.f), color);
			DrawLine(pos + glm::vec3(size.x, 0.f, 0.f), pos + glm::vec3(size.x, size.y, 0.f), color);
			DrawLine(pos + glm::vec3(size.x, size.y, 0.f), pos + glm::vec3(0.f, size.y, 0.f), color);

			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos + glm::vec3(0.f, size.y, size.z), color);
			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos + glm::vec3(size.x, 0.f, size.z), color);
			DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, size.y, size.z), color);
			DrawLine(pos + size, pos + glm::vec3(0.f, size.y, size.z), color);

			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos, color);
			DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, 0.f, 0.f), color);

			DrawLine(pos + glm::vec3(0.f, size.y, size.z), pos + glm::vec3(0.f, size.y, 0.f), color);
			DrawLine(pos + size, pos + glm::vec3(size.x, size.y, 0.f), color);
		}

		void DrawOutlineCube(const AABB& aabb, const glm::vec4& color) {
			DrawOutlineCube(aabb.position, aabb.size, color);
		}

		void Flush() {
			if (!IndexCount) return;
			shader.use();

			lineArray.Bind();
			glDrawArrays(GL_LINES, 0, IndexCount);
			IndexCount = 0;
			byteOffset = 0;
		}

		LineBatcher() {	}
	};
}