#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>
#include "world/World.hpp"

namespace wc {

	class Application : public Engine {
	private:
		wc::Window window;

		wc::Clock deltaTimer;
		bool CenterMouse = false;
		float deltaTime = 0.0f;

		gl::VertexBuffer scrQuad;
		gl::VertexArray scrQuadA;
		gl::FrameBuffer screen;
		gl::Shader screenShader;

		wc::Singleplayer world;

		Font font;

		//irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();
		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {

			if (window.isOpen()) return true;

			return false;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {

			world.OnInput(deltaTime);
			if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::F)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::R)) glDisable(GL_CULL_FACE);
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

			window.setClearColor(glm::vec4(0.1f, 3.5f, 5.0f, 1.0f));

			screenShader.Create("shaderpacks/default/screenShader.glsl");
			screen.Create(window.GetSize().x, window.GetSize().y);

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
			scrQuadA.VertexAttribPointer(0, 2, sizeof(float) * 4, (void*)0);
			scrQuadA.VertexAttribPointer(1, 2, sizeof(float) * 4, (void*)(2 * sizeof(float)));

			font.Load("assets/font/Minecraft.ttf", 128);

			Renderer2D::Init();

			glm::mat4 proj = glm::ortho(0.0f, window.GetSize().x, 0.0f, window.GetSize().y);

			Renderer2D::SetProjection(proj);
			Renderer2D::SetTextProjection(proj);

			world.Create();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			screen.Bind();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			window.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			world.Update(window.GetPos(), window.GetSize(), CenterMouse, deltaTime);

			glDisable(GL_DEPTH_TEST);
			screen.unbind();
			// clear all relevant buffers
			window.clear();

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			screenShader.use();			
			scrQuad.Bind();
			scrQuadA.Bind();
			screen.BindTexture();	// use the color attachment texture as the texture of the quad plane

			glDrawArrays(GL_TRIANGLES, 0, 6);
			Renderer2D::DrawTexts("FPS: " + std::to_string((int)(1 / deltaTime)) + " Frametime: " + std::to_string(deltaTime * 1000), font, { 25.0f, window.GetSize().y - 20 });
			Renderer2D::DrawTexts("X: " + std::to_string(world.p.Position.x) + " Y: " + std::to_string(world.p.Position.y) + " Z: " + std::to_string(world.p.Position.z), font, { 25.0f, window.GetSize().y - 60 });
			Renderer2D::DrawTexts("Pitch: " + std::to_string(world.p.camera.Pitch) + " Yaw: " + std::to_string(world.p.camera.Yaw), font, { 25.0f, window.GetSize().y - 100 });
			Renderer2D::DrawTexts(
				 "ChunkX: " + std::to_string(glm::floor(world.p.Position.x / chunkSize)) +
				" ChunkY: " + std::to_string(glm::floor(world.p.Position.y / chunkSize)) +
				" ChunkZ: " + std::to_string(glm::floor(world.p.Position.z / chunkSize)), font,{ 25.0f, window.GetSize().y - 140 });

			Renderer2D::DrawQuad({ 0,200 }, { 200,-200 }, screen.GetRendererID());
			Renderer2D::Flush();
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			glfwTerminate();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() {}
	};
}
