#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>
#include "world/World.hpp"

namespace wc {

	class GameEngine : public Engine {
	private:
		sf::RenderWindow window;
		sf::Clock deltaTimer;
		bool CenterMouse = false;
		float deltaTime = 0.0f;

		gl::VertexBuffer scrQuad;
		gl::VertexArray scrQuadA;
		gl::ShadowMap shadowMap;
		gl::FrameBuffer screen;
		gl::Shader screenShader;

		wc::Singleplayer world;

		gl::Text TextRenderer;

		//irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

		//----------------------------------------------------------------------------------------------------------------------
		void OnEvent() override {
			sf::Event windowEvents;
			while (window.pollEvent(windowEvents)) {
				if (windowEvents.type == windowEvents.Closed)window.close();
				if (windowEvents.type == sf::Event::Resized) {
					glViewport(0, 0, window.getSize().x, window.getSize().y);
				}
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		EngineStatus GetEngineStatus() override {

			if (window.isOpen()) return EngineStatus::OK;

			return EngineStatus::FAIL;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {

			world.OnEvent(deltaTime);
			if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::F))
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::R)) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);

			if (!window.hasFocus()) CenterMouse = false;
			else  CenterMouse = true;

			if (CenterMouse) wc::Mouse::ShowMouse(false);
			else wc::Mouse::ShowMouse(true);

		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			sol::state windowScript;
			windowScript.script_file("config/window.lua");
			int width = 0, height = 0;

			bool fullScreen = 0;
			bool vsync = 0;

			uint32_t frameRateLimit = 0;
			uint8_t style = sf::Style::Default;

			fullScreen = windowScript["fullscreen"];
			if (fullScreen == true) {
				style = sf::Style::Fullscreen;
				width = sf::VideoMode::getDesktopMode().width;
				height = sf::VideoMode::getDesktopMode().height;
			}
			else {
				style = sf::Style::Default;
				width = windowScript["screenWidth"];
				height = windowScript["screenHeight"];
			}


			frameRateLimit = windowScript["framerateLimit"];
			vsync = windowScript["vsync"];

			window.create(sf::VideoMode(width, height), "Elementalworld", style, sf::ContextSettings(24, 0, windowScript["antialiasingLevel"]));
			window.setFramerateLimit(frameRateLimit);
			window.setVerticalSyncEnabled(vsync);

			window.setActive();
			if (!gladLoadGL()) WC_ERROR("Failed to initialize GLAD");

			// OpenGL state
			EnableGLDebuging();
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//Anti aliasing
			glEnable(GL_MULTISAMPLE);

			//Depth testing
			glEnable(GL_DEPTH_TEST);

			//Stencil Test
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF); // each bit is written to the stencil buffer as is
			glStencilMask(0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)
			//glStencilFunc(GL_EQUAL, 1, 0xFF);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			//SoundEngine->play2D("assets/sounds/Alan Walker - The Spectre_wJnBTPUQS5A_youtube.mp3");

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			screenShader.Create("shaderpacks/default/screenShader.glsl");
			screen.Create(GetWindowSize().x, GetWindowSize().y);

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
			gl::VertexAttribPointer(0, 2, sizeof(float) * 4, (void*)0);
			gl::VertexAttribPointer(1, 2, sizeof(float) * 4, (void*)(2 * sizeof(float)));
			
			TextRenderer.Create("assets/font/Minecraft.ttf", "shaderpacks/default/text.glsl");

			world.Create();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart().asSeconds();
			//screen.Bind();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			world.Update(GetWindowPos(), GetWindowSize(), CenterMouse, deltaTime);

			glDisable(GL_DEPTH_TEST);
			//screen.unbind();
			// clear all relevant buffers
			//glClear(GL_COLOR_BUFFER_BIT);

			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//screenShader.use();
			//screenShader.setVec4("screenColor", world.screenColor);
			//scrQuad.Bind();
			//scrQuadA.Bind();
			//screen.BindTexture();	// use the color attachment texture as the texture of the quad plane

			//glDrawArrays(GL_TRIANGLES, 0, 6);
			TextRenderer.Draw(std::to_string((int)(1 / deltaTime)), GetWindowSize(), { 25.0f, GetWindowSize().y - 20 });
			TextRenderer.Draw("X: " + std::to_string(world.p.Position.x) + " Y: " + std::to_string(world.p.Position.y) + " Z: " + std::to_string(world.p.Position.z), GetWindowSize(), { 25.0f, GetWindowSize().y - 60 });
			TextRenderer.Draw("Pitch: " + std::to_string(world.p.camera.Pitch) + " Yaw: " + std::to_string(world.p.camera.Yaw), GetWindowSize(), { 25.0f, GetWindowSize().y - 100 });
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {}
		//----------------------------------------------------------------------------------------------------------------------
		const glm::vec2& GetWindowPos() {
			return { window.getPosition().x, window.getPosition().y };
		}
		//----------------------------------------------------------------------------------------------------------------------
		const glm::vec2& GetWindowSize() {
			return { window.getSize().x, window.getSize().y };
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		GameEngine() {}
	};
}