#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include <wc/pch.hpp>
#include <gl/glErrors.hpp>
#include "world/World.hpp"

namespace wc {

	class Application : public Engine {
	private:
		wc::Window window;

		wc::Clock deltaTimer;
		bool CenterMouse = false;
		float deltaTime = 0.0f;

		// Framebuffer stuff
		gl::VertexBuffer scrQuad;
		gl::VertexArray scrQuadA;
		gl::FrameBuffer screen;
		gl::Shader screenShader;
		gl::Texture scrTexture;

		wc::Singleplayer world;

		//irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();
		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {

			if (window.isOpen()) return true;

			return false;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			if (window.hasFocus())	world.OnInput(deltaTime);

			if (wc::Keyboard::isKeyPressed(wc::Keyboard::Key::F)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (wc::Keyboard::isKeyPressed(wc::Keyboard::Key::R)) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);

			if (!window.hasFocus()) CenterMouse = false;
			else  CenterMouse = true;
			
			if (CenterMouse) wc::Mouse::ShowMouse(false);
			else wc::Mouse::ShowMouse(true);			
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create("config/window.lua", "Elementalworld");
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) WC_ERROR("Failed to initialize GLAD");
			// OpenGL state
			EnableGLDebuging();
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//Anti aliasing
			glEnable(GL_MULTISAMPLE);

			//Depth testing
			glEnable(GL_DEPTH_TEST);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			Renderer::setClearColor(glm::vec4(0.1f, 3.5f, 5.0f, 1.0f));

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

			glm::mat4 proj = glm::ortho(0.0f, window.GetSize().x, window.GetSize().y, 0.0f);

			Renderer2D::SetProjection(proj);

			world.Create();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			screen.Bind();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			world.Update(window.GetPos(), window.GetSize(), CenterMouse, deltaTime);

			glDisable(GL_DEPTH_TEST);
			screen.unbind();
			// clear all relevant buffers
			Renderer::Clear();

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			screenShader.use();			
			scrQuad.Bind();
			scrQuadA.Bind();
			scrTexture.Bind(); // use the color attachment texture as the texture of the quad plane

			Renderer::DrawArrays(6);
			Renderer2D::DrawTexts("FPS: " + std::to_string((int)(1 / deltaTime)) + " Frametime: " + std::to_string(deltaTime * 1000), world.font, { 25.0f, 20 });

			Renderer2D::Flush();
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() {}
	};
}
#endif