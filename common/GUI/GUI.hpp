#pragma once

//#include <wc/pch.hpp>

#include <glm/glm.hpp>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi/Lua.h>

#include <gl/Buffer.hpp>
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/VertexArray.hpp>
#include <Renderer/Renderer.hpp>

namespace wc {	

	class RenderInterface : public Rml::RenderInterface
	{
		static const uint32_t MaxQuadCount = 100;
		static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
		static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;

		static const uint8_t MaxTextures = 32;

		bool transformEnabled = false;

		gl::VertexArray VAO;
		gl::Buffer VBO;
		gl::Buffer EBO;
		gl::Shader shader;

		struct RmlVertex {
			glm::vec2 position = glm::vec2(0.f);
			uint32_t color = 0;
			glm::vec3 tex_coord = glm::vec3(0.f);
		};

		uint32_t IndexCount = 0;
		uint32_t TextureSlots[MaxTextures] = { 0 };
		uint32_t byteOffset = 0;
		uint32_t indByteOffset = 0;
		uint32_t iOffset = 0;
		uint8_t TextureSlotIndex = 0;

		gl::Texture whiteTexture;

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

	public:
		void Create() {
			VAO.Create();
			EBO.Create(nullptr, sizeof(uint32_t) * MaxQuadIndexCount, GL_DYNAMIC_STORAGE_BIT);
			VBO.Create(nullptr, sizeof(RmlVertex) * MaxQuadVertexCount, GL_DYNAMIC_STORAGE_BIT);
			VAO.VertexAttribPointer(0, 2, offsetof(RmlVertex, position));
			VAO.VertexAttribPointer(1, 1, offsetof(RmlVertex, color));
			VAO.VertexAttribPointer(2, 3, offsetof(RmlVertex, tex_coord));
			VAO.AddVertexBuffer(VBO, sizeof(RmlVertex));
			VAO.AddIndexBuffer(EBO);
			shader.Create("resourcepacks/default/shaders/RmlRenderer.glsl");

			float color[] = { 1.f, 1.f, 1.f };
			whiteTexture.Create(color, 1, 1);
		}

		void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override {
			if (IndexCount + num_indices >= MaxQuadIndexCount || TextureSlotIndex >= MaxTextures) Flush();

			glm::vec2 windowSize = glm::vec2(this->GetContext()->GetDimensions().x, this->GetContext()->GetDimensions().y);

			uint32_t texID = texture;
			if (texID == 0) texID = whiteTexture;
			float textureIndex = getTexture(texID);

			for (uint32_t i = 0; i < num_vertices; i++) {
				RmlVertex vertex;
				vertex.position = (glm::vec2(vertices[i].position.x + translation.x, vertices[i].position.y + translation.y) / windowSize) * 2.f - 1.f;
				vertex.position.y = -vertex.position.y;

				glm::vec4 color = glm::vec4(vertices[i].colour.red, vertices[i].colour.green, vertices[i].colour.blue, vertices[i].colour.alpha);
				uint32_t Color = (uint32_t)(color.r) << 24 | (uint32_t)(color.g) << 16 | (uint32_t)(color.b) << 8 | (uint32_t)(color.a);
				vertex.color = Color;

				vertex.tex_coord = glm::vec3(vertices[i].tex_coord.x, vertices[i].tex_coord.y, 0.f);
				VBO.SetData(byteOffset, sizeof(RmlVertex), &vertex);
				byteOffset += sizeof(RmlVertex);
			}

			for (uint32_t i = 0; i < num_indices; i++) {
				EBO.SetData(indByteOffset, sizeof(uint32_t), &indices[i]);
				indByteOffset += sizeof(uint32_t);
			}

			iOffset += num_vertices;
			IndexCount += num_indices;

			Flush();
		}

		void Flush() {
			if (!IndexCount) return;
			shader.use();

			for (uint8_t i = 0; i < TextureSlotIndex; i++)
				glBindTextureUnit(i, TextureSlots[i]);

			VAO.Bind();
			Renderer::DrawIndexed(IndexCount);
			TextureSlotIndex = 0;
			IndexCount = 0;
			indByteOffset = 0;
			byteOffset = 0;
			iOffset = 0;
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
			//WC_INFO("SetScissorRegion");
			if (!transformEnabled)
				glScissor(x, /*window.GetSize()*/this->GetContext()->GetDimensions().y - (y + height), width, height);
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
				RmlVertex vertices[4];
				vertices[0].position = glm::vec2(fx, fy);
				vertices[1].position = glm::vec2(fx, fy + fheight);
				vertices[2].position = glm::vec2(fx + fwidth, fy + fheight);
				vertices[3].position = glm::vec2(fx + fwidth, fy);
				
				GLushort indices[] = { 0,1,2,2,3,0 };
				VBO.SetData(0, sizeof(vertices), vertices);
				EBO.SetData(0, sizeof(indices), indices);
				VAO.Bind();
				shader.use();
				Renderer::DrawIndexed(6);
			
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

			if (data) texture.CreateRml(glm::ivec2(texture_dimensions.x, texture_dimensions.y), data);
			else WC_ERROR("Could not open file location at path {0}!", source.c_str());

			delete data; // stbi free

			texture_handle = texture;

			return true;
		}

		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
			gl::Texture texture;
			texture.CreateRml(glm::vec2(source_dimensions.x, source_dimensions.y), source);
			texture_handle = texture;
			return true;
		}

		void ReleaseTexture(Rml::TextureHandle texture) {
			glDeleteTextures(1, (uint32_t*)&texture);
		}

		void SetTransform(const Rml::Matrix4f* rmlTransform) {
			WC_INFO("SetTransform");
			transformEnabled = (bool)rmlTransform;
		}
	} render_interface;

	class SystemInterface : public Rml::SystemInterface
	{

		double GetElapsedTime() override
		{
			return glfwGetTime();
		}

		//virtual int TranslateString(Rml::String& translated, const Rml::String& input);

		//virtual void JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path);

		bool LogMessage(Rml::Log::Type type, const Rml::String& message) override {
			switch (type) {
			case Rml::Log::Type::LT_ALWAYS:	WC_TRACE(message.c_str()); break;
			case Rml::Log::Type::LT_ERROR:	WC_ERROR(message.c_str()); break;
			case Rml::Log::Type::LT_ASSERT:	WC_INFO(message.c_str()); break;
			case Rml::Log::Type::LT_WARNING:WC_WARN(message.c_str()); break;
			case Rml::Log::Type::LT_INFO:	WC_INFO(message.c_str()); break;
			case Rml::Log::Type::LT_DEBUG:	WC_DEBUG(message.c_str()); break;
			case Rml::Log::Type::LT_MAX:    WC_CRITICAL(message.c_str()); break;
			}
			return true;
		}

		//virtual void SetMouseCursor(const Rml::String& cursor_name);

		void SetClipboardText(const Rml::String& text) {
			glfwSetClipboardString(window, text.c_str());
		}

		void GetClipboardText(Rml::String& text) override {
			text = glfwGetClipboardString(window);
		}

		//virtual void ActivateKeyboard();

		//virtual void DeactivateKeyboard();
	public:
		GLFWwindow* window = nullptr;
	}system_interface;
}