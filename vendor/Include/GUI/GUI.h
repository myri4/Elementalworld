#pragma once

#include <glm/glm.hpp>
#include <RmlUi/Lua.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include <gl/Buffer.h>
#include <gl/Shaders.h>
#include <gl/Texture.h>

namespace wc {	

	class RenderInterface : public Rml::RenderInterface
	{
		static const uint32_t MaxQuadCount = 10000;
		static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
		static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;

		static const uint8_t MaxTextures = 32;

		bool transformEnabled = false;

		gl::Buffer VBO;
		gl::Buffer EBO;
		gl::Shader shader;

		struct RmlVertex {
			glm::vec2 position = glm::vec2(0.f);
			uint32_t color = 0xFFFFFFFF;
			glm::vec3 tex_coord = glm::vec3(0.f);

			RmlVertex() = default;
			RmlVertex(const glm::vec2& pos, const glm::vec3& texCoord) : position(pos), tex_coord(texCoord) { }
		};

		uint32_t IndexCount = 0;
		uint32_t TextureSlots[MaxTextures] = { 0 };
		uint32_t VertexCount = 0;
		uint8_t TextureSlotIndex = 0;

		uint32_t getTexture(const uint32_t& texID) {
			uint32_t textureIndex = 0;

			for (uint32_t i = 0; i < TextureSlotIndex; i++) {
				if (TextureSlots[i] == texID) {
					textureIndex = i;
					break;
				}
			}

			if (textureIndex == 0) {
				textureIndex = TextureSlotIndex;
				TextureSlots[TextureSlotIndex] = texID;
				TextureSlotIndex++;
			}
			return textureIndex;
		}

		uint32_t* indices = nullptr;
		RmlVertex* vertices = nullptr;
	public:
		glm::vec2 windowSize = glm::vec2(0.f);
		gl::Texture whiteTexture;
		void Create() {
			shader.Create("resourcepacks/default/shaders/RmlRenderer.vert", "resourcepacks/default/shaders/RmlRenderer.frag");
			uint32_t bits = GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT;
			EBO.Create(sizeof(uint32_t) * MaxQuadIndexCount, bits);
			VBO.Create(sizeof(RmlVertex) * MaxQuadVertexCount, bits);
			shader.VertexAttribPointer(0, 2, offsetof(RmlVertex, position));
			shader.VertexAttribPointer(1, 1, offsetof(RmlVertex, color));
			shader.VertexAttribPointer(2, 3, offsetof(RmlVertex, tex_coord));
			shader.SetVertexBuffer(VBO, sizeof(RmlVertex));
			shader.SetIndexBuffer(EBO);
			indices = (uint32_t*)EBO.Map(bits, sizeof(uint32_t) * MaxQuadIndexCount);
			vertices = (RmlVertex*)VBO.Map(bits, sizeof(RmlVertex) * MaxQuadVertexCount);

			float color[] = { 1.f, 1.f, 1.f };
			whiteTexture.Create(color, 1, 1);
		}

		void RenderGeometry(Rml::Vertex* p_vertices, int num_vertices, int* p_indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override {
			if (IndexCount + num_indices >= MaxQuadIndexCount || TextureSlotIndex >= MaxTextures) Flush();

			uint32_t texID = texture;
			if (texID == 0) texID = whiteTexture;
			float textureIndex = (float)getTexture(texID);

			for (int32_t i = 0; i < num_indices; i++) {
				indices[IndexCount] = p_indices[i] + VertexCount;
				IndexCount++;
			}

			glm::mat4 trans = glm::mat4(1.f);

			if (transformEnabled) {
				transformEnabled = false;
				trans = transform;
			}

			for (int32_t i = 0; i < num_vertices; i++) {
				RmlVertex& vertex = vertices[VertexCount];
				vertex.position = (glm::vec2(p_vertices[i].position.x + translation.x, p_vertices[i].position.y + translation.y) / windowSize) * 2.f - 1.f;
				vertex.position.y = -vertex.position.y;

				glm::vec4 Pos = glm::vec4(vertex.position.x, vertex.position.y, 0.f, 0.f) * trans;
				vertex.position = glm::vec2(Pos.x, Pos.y);

				glm::vec4 color = glm::vec4(p_vertices[i].colour.red, p_vertices[i].colour.green, p_vertices[i].colour.blue, p_vertices[i].colour.alpha);
				vertex.color = (uint32_t)(color.r) << 24 | (uint32_t)(color.g) << 16 | (uint32_t)(color.b) << 8 | (uint32_t)(color.a);

				vertex.tex_coord = glm::vec3(p_vertices[i].tex_coord.x, p_vertices[i].tex_coord.y, textureIndex);
				VertexCount++;
			}
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const uint32_t& texID) {
			if (IndexCount >= MaxQuadIndexCount || TextureSlotIndex >= MaxTextures) Flush();

			float textureIndex = (float)getTexture(texID);

			vertices[VertexCount + 0] = RmlVertex({ pos.x + size.x, pos.y + size.y }, { 1.f, 0.f, textureIndex });
			vertices[VertexCount + 1] = RmlVertex({ pos.x,		  pos.y + size.y }, { 0.f, 0.f, textureIndex });
			vertices[VertexCount + 2] = RmlVertex({ pos.x,		  pos.y, }, { 0.f, 1.f, textureIndex });
			vertices[VertexCount + 3] = RmlVertex({ pos.x + size.x, pos.y, }, { 1.f, 1.f, textureIndex });			

			for (uint8_t i = 0; i < 4; i++) {
				glm::vec2& Pos = vertices[i + VertexCount].position;
				Pos = ((vertices[i + VertexCount].position / windowSize) * 2.f - 1.f);
				Pos.y = -Pos.y;
			}

			indices[IndexCount + 0] = VertexCount;
			indices[IndexCount + 1] = 1 + VertexCount;
			indices[IndexCount + 2] = 2 + VertexCount;

			indices[IndexCount + 3] = 2 + VertexCount;
			indices[IndexCount + 4] = 3 + VertexCount;
			indices[IndexCount + 5] = VertexCount;

			VertexCount += 4;
			IndexCount += 6;
		}

		void Flush() {
			if (!IndexCount) return;

			shader.use();

			for (uint8_t i = 0; i < TextureSlotIndex; i++)
				glBindTextureUnit(i, TextureSlots[i]);

			glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);
			TextureSlotIndex = 0;
			IndexCount = 0;
			VertexCount = 0;
		}

		void EnableScissorRegion(bool enable) override {
			if (enable) {
				if (!transformEnabled) {
					glEnable(GL_SCISSOR_TEST);
					glDisable(GL_STENCIL_TEST);
				}
				else {
					glDisable(GL_SCISSOR_TEST);
					glEnable(GL_STENCIL_TEST);
				}
			}
			else {
				glDisable(GL_SCISSOR_TEST);
				glDisable(GL_STENCIL_TEST);
			}
		}

		void SetScissorRegion(int x, int y, int width, int height) override {
			if (!transformEnabled)
				glScissor(x, /*window.GetSize()*/windowSize.y - (y + height), width, height);
			else {
				// clear the stencil buffer
				glStencilMask(GLuint(-1));
				glClear(GL_STENCIL_BUFFER_BIT);
			
				// fill the stencil buffer
				glColorMask(false, false, false, false);
				glDepthMask(false);
				glStencilFunc(GL_NEVER, 1, GLuint(-1));
				glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
			
				float fx = (float)x;
				float fy = (float)y;
				float fwidth = (float)width;
				float fheight = (float)height;
			
				// draw transformed quad
				RmlVertex vertices2[4];
				vertices2[0].position = glm::vec2(fx, fy);
				vertices2[1].position = glm::vec2(fx, fy + fheight);
				vertices2[2].position = glm::vec2(fx + fwidth, fy + fheight);
				vertices2[3].position = glm::vec2(fx + fwidth, fy);
				
				uint32_t indices2[] = { 0,1,2,2,3,0 };
				memcpy(indices, indices2, sizeof(indices2));
				memcpy(vertices, vertices2, sizeof(vertices2));
				IndexCount = 0;
				VertexCount = 0;
				TextureSlotIndex = 0;
				shader.use();
			
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			
				// prepare for drawing the real thing
				glColorMask(true, true, true, true);
				glDepthMask(true);
				glStencilMask(0);
				glStencilFunc(GL_EQUAL, 1, GLuint(-1));
			}
		}

		bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
			std::string src = (std::string)source.c_str();

			gl::Texture texture;

			int fnrComponents;
			auto data = stbi_load(source.c_str(), &texture_dimensions.x, &texture_dimensions.y, &fnrComponents, 0);

			if (data) { 
				gl::TextureProps props;
				props.SetSize(glm::ivec2(texture_dimensions.x, texture_dimensions.y));
				GLint filter = GL_LINEAR;
				if (texture_dimensions.x <= 120 || texture_dimensions.y <= 120) filter = GL_NEAREST;
				props.internalFormat = GL_RGBA8;
				props.mag_filter = filter;
				props.min_filter = filter;
				props.wrap_s = GL_CLAMP_TO_EDGE;
				props.wrap_t = GL_CLAMP_TO_EDGE;
				texture.Create(props);

				glTextureSubImage2D(texture, 0, 0, 0, texture_dimensions.x, texture_dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else WC_ERROR("Could not open file location at path {0}!", source.c_str());

			delete data; // stbi free

			texture_handle = texture;

			return true;
		}

		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
			gl::Texture texture;
			gl::TextureProps props;
			props.SetSize(glm::ivec2(source_dimensions.x, source_dimensions.y));
			props.internalFormat = GL_RGBA8;
			props.mag_filter = GL_NEAREST;
			props.min_filter = GL_NEAREST;
			props.wrap_s = GL_CLAMP_TO_EDGE;
			props.wrap_t = GL_CLAMP_TO_EDGE;
			texture.Create(props);

			glTextureSubImage2D(texture, 0, 0, 0, source_dimensions.x, source_dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, source);

			texture_handle = texture;
			return true;
		}

		void ReleaseTexture(Rml::TextureHandle texture) {
			glDeleteTextures(1, (uint32_t*)&texture);
		}

		glm::mat4 transform = glm::mat4(0.f);
		void SetTransform(const Rml::Matrix4f* rmlTransform) {
			transformEnabled = (bool)rmlTransform;

			if (transformEnabled) {
				if (std::is_same<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>::value) transform = glm::make_mat4(rmlTransform->data());
				else if (std::is_same<Rml::Matrix4f, Rml::RowMajorMatrix4f>::value) transform = glm::make_mat4(rmlTransform->Transpose().data());				
			}
		}
	} render_interface;

	GLFWwindow* system_window = nullptr;
	class SystemInterface : public Rml::SystemInterface
	{

		double GetElapsedTime() override { return glfwGetTime(); }

		bool LogMessage(Rml::Log::Type type, const Rml::String& message) override {
			switch (type) {
				case Rml::Log::Type::LT_ALWAYS:	WC_TRACE(message.c_str());    break;
				case Rml::Log::Type::LT_ERROR:	WC_ERROR(message.c_str());    break;
				case Rml::Log::Type::LT_ASSERT:	WC_INFO(message.c_str());     break;
				case Rml::Log::Type::LT_WARNING:WC_WARN(message.c_str());     break;
				//case Rml::Log::Type::LT_INFO:	WC_INFO(message.c_str());     break;
				case Rml::Log::Type::LT_DEBUG:	WC_DEBUG(message.c_str());    break;
				case Rml::Log::Type::LT_MAX:    WC_CRITICAL(message.c_str()); break;
			}
			return true;
		}

		void SetClipboardText(const std::string& text) {
			glfwSetClipboardString(system_window, text.c_str());
		}

		void GetClipboardText(std::string& text) override {
			text = glfwGetClipboardString(system_window);
		}
	}system_interface;
}