#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>
#include "world/World.hpp"

namespace wc {

	class GameEngine : public Engine {
	private:
		sf::RenderWindow window;
		sf::Clock deltaTimer;
		bool CenterMouse = true;
		float deltaTime = 0.0f;

		wc::Singleplayer world;

		gl::Text TextRenderer;

		//irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

		//----------------------------------------------------------------------------------------
		void OnEvent() override {
			sf::Event windowEvents;
			while (window.pollEvent(windowEvents)) {
				if (windowEvents.type == windowEvents.Closed)window.close();
				if (windowEvents.type == sf::Event::Resized) {
					glViewport(0, 0, window.getSize().x, window.getSize().y);
				}
			}
		}
		//----------------------------------------------------------------------------------------
		EngineStatus GetEngineStatus() override {

			if (window.isOpen()) return EngineStatus::OK;

			return EngineStatus::FAIL;
		}
		//----------------------------------------------------------------------------------------
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
			Lua windowScript("config/window.lua");
			int width = 0, height = 0;

			bool fullScreen = 0;
			bool vsync = 0;

			uint32_t frameRateLimit = 0;
			uint8_t style = sf::Style::Default;

			fullScreen = windowScript.GetBool("fullscreen");
			if (fullScreen == true) {
				style = sf::Style::Fullscreen;
				width = sf::VideoMode::getDesktopMode().width;
				height = sf::VideoMode::getDesktopMode().height;
			}
			else {
				style = sf::Style::Default;
				width = windowScript.GetNumber("screenWidth");
				height = windowScript.GetNumber("screenHeight");
			}


			frameRateLimit = windowScript.GetNumber("framerateLimit");
			vsync = windowScript.GetBool("vsync");

			window.create(sf::VideoMode(width, height), "Elementalworld", style, sf::ContextSettings(24, 0, windowScript.GetNumber("antialiasingLevel"), windowScript.GetNumber("majorVersion"), windowScript.GetNumber("minorVersion")));
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
			glStencilFunc(GL_EQUAL, 1, 0xFF);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			//SoundEngine->play2D("assets/sounds/Alan Walker - The Spectre_wJnBTPUQS5A_youtube.mp3");

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			
			world.Create();

			TextRenderer.Create("assets/font/Minecraft.ttf", "shaderpacks/default/text.glsl");
		}
		
		//----------------------------------------------------------------------------------------------------------------------

		void OnUpdate() override {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			world.Update(window, CenterMouse, deltaTime);

			deltaTime = deltaTimer.restart().asSeconds();

			glDisable(GL_DEPTH_TEST);
			TextRenderer.Draw(std::to_string((int)(1 / deltaTime)), glm::vec2((float)window.getSize().x, (float)window.getSize().y) , { 25.0f, 700.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			TextRenderer.Draw("X: " + std::to_string(world.p.Position.x) + " Y: " + std::to_string(world.p.Position.y) + " Z: " + std::to_string(world.p.Position.z), glm::vec2((float)window.getSize().x, (float)window.getSize().y), { 25.0f, 660.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			TextRenderer.Draw("Pitch: " + std::to_string(world.p.camera.Pitch) + " Yaw: " + std::to_string(world.p.camera.Yaw), glm::vec2((float)window.getSize().x, (float)window.getSize().y), { 25.0f, 620.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			//TextRenderer.Draw(
			//"ChunkX: " + std::to_string(GetChunkPos(world.p.Position.x)) +
			//" ChunkY: " + std::to_string(GetChunkPos(world.p.Position.y)) +
			//" ChunkZ: " + std::to_string(GetChunkPos(world.p.Position.z))
			//	,
			//	
			//glm::vec2((float)window.getSize().x, (float)window.getSize().y), { 25.0f, 620.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			glEnable(GL_DEPTH_TEST);
			window.display();
		}
		void OnDelete() override {
		}

	public:
		GameEngine() {}
	};
}