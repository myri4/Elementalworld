#pragma once

//#include <wc/pch.hpp>

#include <glm/glm.hpp>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include <gl/Buffer.hpp>
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/VertexArray.hpp>
#include <Renderer/Renderer.hpp>
#include "Renderer2D.hpp"

namespace wc {

	Rml::Context* context = nullptr;

	class RenderInterface : public Rml::RenderInterface
	{
	private:
		bool transformEnabled = false;
		gl::VertexArray VAO;
		gl::VertexBuffer VBO;
		gl::IndexBuffer EBO;

		struct RmlVertex {
			/// Two-dimensional position of the vertex (usually in pixels).
			glm::vec2 position;
			/// RGBA-ordered 8-bit / channel colour.
			glm::vec4 colour;
			/// Texture coordinate for any associated texture.
			glm::vec2 tex_coord;
		};
	public:
		gl::Shader shader;
		glm::vec2 viewportSize = glm::vec2(0.f);

		void Create() {
			EBO.Create(nullptr, sizeof(uint32_t) * 10000, GL_DYNAMIC_STORAGE_BIT);
			VAO.Create();
			VBO.Create(nullptr, 10000 * sizeof(RmlVertex), GL_DYNAMIC_STORAGE_BIT);
			VAO.VertexAttribPointer(0, 2, offsetof(RmlVertex, position));
			VAO.VertexAttribPointer(1, 4, offsetof(RmlVertex, colour));
			VAO.VertexAttribPointer(2, 2, offsetof(RmlVertex, tex_coord));
			VAO.AddVertexBuffer(VBO, sizeof(RmlVertex));
			VAO.AddIndexBuffer(EBO);
			shader.Create("shaderpacks/default/RmlRenderer.glsl");
		}

		void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override {
			EBO.SetData(0, num_indices * sizeof(int), indices);
			uint32_t offset = 0;
			for (int i = 0; i < num_vertices; i++) {
				RmlVertex vertex;
				glm::vec2 pos = glm::vec2(vertices[i].position.x + translation.x, vertices[i].position.y + translation.y);
				float x =  ((pos.x / viewportSize.x) * 2.f - 1.f);
				float y = -((pos.y / viewportSize.y) * 2.f - 1.f);
				vertex.position = pos;// glm::vec2(x, y);
				vertex.colour = glm::vec4(vertices[i].colour.red, vertices[i].colour.green, vertices[i].colour.blue, vertices[i].colour.alpha);
				vertex.tex_coord = glm::vec2(vertices[i].tex_coord.x, vertices[i].tex_coord.y);
				VBO.SetData(offset, sizeof(RmlVertex), &vertex);
				offset += sizeof(RmlVertex);
			}

			shader.use();
			shader.setVec2(0, viewportSize);

			if (texture) 
				glBindTextureUnit(0, texture);			
			else 
				Renderer2D::m_Data.whiteTexture.Bind(0);
			
			VBO.Bind();
			EBO.Bind();
			VAO.Bind();
			Renderer::DrawIndexed(num_indices);
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
				glScissor(x, /*window.GetSize()*/viewportSize.y - (y + height), width, height);
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
				//Rml::Vertex vertices[4];
				//vertices[0].position = Rml::Vector2f(fx, fy);// Rml::Vertex(Rml::Vector2f(fx, fy), 0, Rml::Vector2f(0, 0));
				//vertices[1].position = Rml::Vector2f(fx, fy + fheight);//Rml::Vertex({ fx, fy + fheight }, 0, { 0,0 });
				//vertices[2].position = Rml::Vector2f(fx + fwidth, fy + fheight);//Rml::Vertex({ fx + fwidth, fy + fheight }, 0, { 0,0 });
				//vertices[3].position = Rml::Vector2f(fx + fwidth, fy);//Rml::Vertex({ fx + fwidth, fy }, 0, { 0,0 });
				//
				//vertices[0].colour = Rml::Colourb(0.f);
				//vertices[1].colour = Rml::Colourb(0.f);
				//vertices[2].colour = Rml::Colourb(0.f);
				//vertices[3].colour = Rml::Colourb(0.f);
				//
				//GLushort indices[] = { 1, 2, 0, 3 };
				//VBO.SetData(0, sizeof(vertices), vertices);
				//EBO.SetData(0, sizeof(indices), indices);
				//VAO.Bind();
				//colorShader.use();
				//glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);

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
			//glfwSetClipboardString(window, text.c_str());
		}

		void GetClipboardText(Rml::String& text) override {
			//text = window.getClipboard();
		}

		//virtual void ActivateKeyboard();

		//virtual void DeactivateKeyboard();
	}system_interface;
}