#ifndef APPLICATION_HPP
#define APPLICATION_HPP
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "world/World.hpp"
#include "GUI/Textbox.hpp"
#include "GUI/Button.hpp"

namespace wc {

	class Application : public Engine {
	private:
		Window window;

		Clock deltaTimer;
		float deltaTime = 0.f;

		// FrameBuffer stuff
		gl::VertexBuffer scrQuad;
		gl::VertexArray scrQuadA;
		gl::FrameBuffer screen;
		gl::Shader screenShader;
		gl::Texture scrTexture;

		Singleplayer world;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {

			if (window.isOpen()) return true;

			return false;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(window.GetPos(), window.GetSize(), hasFocus, deltaTime);
				else world.OnInputInventory();
			}				
				//world.OnInput(window.GetPos(), window.GetSize(), hasFocus);
			if (wc::Keyboard::isKeyPressed(wc::Keyboard::Key::F)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create("config/window.lua", "Elementalworld");
			// OpenGL state
			Renderer::enableDebuging();
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//Anti aliasing
			glEnable(GL_MULTISAMPLE);

			//Depth testing
			glEnable(GL_DEPTH_TEST);

			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			//Renderer::setClearColor(glm::vec4(0.1f, 3.5f, 5.0f, 1.0f));

			screenShader.Create("shaderpacks/default/screenShader.glsl");
			screen.Create(window.GetSize().x, window.GetSize().y);

			gl::TextureProps scrProps;
			scrProps.data = nullptr;
			scrProps.SetSize(window.GetSize());
			scrProps.internalFormat = GL_RGB;
			scrProps.format = GL_RGB;
			scrProps.type = GL_UNSIGNED_BYTE;
			scrProps.mag_filter = GL_LINEAR;
			scrProps.min_filter = GL_LINEAR;

			scrTexture.Create(scrProps);

			screen.Bind();
			screen.addTexture(scrTexture);
			screen.unbind();

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions   // texCoords
				-1.0f, -1.0f,  0.0f, 0.0f,
				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				 1.0f, -1.0f,  1.0f, 0.0f,
				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f,  1.0f,  1.0f, 1.0f,
			};

			scrQuad.Create(quadVertices, sizeof(quadVertices));
			scrQuadA.Create();
			scrQuadA.Bind();
			Renderer::VertexAttribPointer(0, 2, sizeof(float) * 4, (void*)0);
			Renderer::VertexAttribPointer(1, 2, sizeof(float) * 4, (void*)(2 * sizeof(float)));

			world.font.Load("assets/font/Minecraft.ttf", 128);

			Renderer2D::Init();

			world.Create();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			Renderer2D::SetProjection(Renderer2D::Get2DProj(window.GetSize()));
			screen.Bind();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto windsize = window.GetSize();
			if (mode == MenuMode::GAME) world.Update(windsize, deltaTime);
			else world.UpdateInventory(windsize, window.GetPos(), deltaTime);

			glDisable(GL_DEPTH_TEST);
			screen.unbind();
			// clear all relevant buffers
			Renderer::Clear(GL_COLOR_BUFFER_BIT);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			screenShader.use();
			scrQuad.Bind();
			scrQuadA.Bind();
			scrTexture.Bind(); // use the color attachment texture as the texture of the quad plane			

			Renderer::DrawArrays(6);			

			Renderer2D::Flush();
			Renderer2D::FlushLines();
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() {}
	};
}
#endif