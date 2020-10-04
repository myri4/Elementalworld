#pragma once
#include <wclibs/pch.hpp>
#include <Utilitiess/Lua.hpp>
#include "world/Chunk.h"
#include <Utilitiess/State.h>
#include "Renderer2D.hpp"
#include "entityes/Player.h"
#include <gl/glErrors.h>

namespace wc {

	class GameEngine : public Engine {
	private:
		sf::RenderWindow window;
		uint32 frameCount, FPS;
		sf::Clock frameTimer, deltaTimer;
		bool CenterMouse;
		float deltaTime;

		wc::Player p;

		Chunk mainChunk;

		gl::Skybox mainSkybox;
		gl::Shader mainShader;
		gl::VertexBuffer chunkMeshBuffer;

		gl::Text TextRenderer;
		gl::Shader textShader;


		irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();


		//----------------------------------------------------------------------------------------
		void loadFromFile(const char* file) {
			Lua windowScript(file);
			float width, height;

			bool fullScreen = 0;
			bool vsync = 0;

			uint32 frameRateLimit = 0;
			uint32 style = sf::Style::Default;

			fullScreen = windowScript.GetBool("fullscreen");
			if (fullScreen == true) {
				style = sf::Style::Fullscreen;
				width = sf::VideoMode::getDesktopMode().width;
				height = sf::VideoMode::getDesktopMode().height;
			}
			else {
				style = sf::Style::Default;
				width = (float)windowScript.GetNumber("screenWidth");
				height = (float)windowScript.GetNumber("screenHeight");
			}


			frameRateLimit = windowScript.GetNumber("framerateLimit");
			vsync = windowScript.GetBool("vsync");

			int32 nrComponents, imgWidth, imgHeight;
			stbi_set_flip_vertically_on_load(false);

			window.create(sf::VideoMode(width, height), "Elementalworld", style, sf::ContextSettings(24, 0, windowScript.GetNumber("antialiasingLevel"), windowScript.GetNumber("majorVersion"), windowScript.GetNumber("minorVersion")));
			window.setIcon(imgWidth, imgHeight, stbi_load(windowScript.GetString("iconPath"), &imgWidth, &imgHeight, &nrComponents, 0));
			window.setFramerateLimit(frameRateLimit);
			window.setVerticalSyncEnabled(vsync);



		}
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

			p.UpdatePlayerInput(deltaTime);
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) mainChunk.IndexCount++;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))mainChunk.IndexCount--;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);


			if (!window.hasFocus()) CenterMouse = false;
			else  CenterMouse = true;

			if (CenterMouse) ew::Mouse::ShowMouse(false);
			else ew::Mouse::ShowMouse(true);

		}

		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			loadFromFile("config/window.lua");
			window.setActive();
			if (!gladLoadGL()) std::cout << "Failed to initialize GLAD\n";

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
			mainShader.Create("shaderpacks/default/core.vs", "shaderpacks/default/core.fs");
			mainShader.use();
			int samplers[MaxTextures];
			for (int i = 0; i < MaxTextureUnits(); i++) samplers[i] = i;
			mainShader.SetArray("u_Textures", MaxTextureUnits(), samplers);


			mainSkybox.Create("scripts/skybox.lua", 1);
			mainChunk.Create({ 0,0,0 });
			grassBlock.Create("scripts/grassblock.lua");

			textShader.Create("shaderpacks/default/text.vs", "shaderpacks/default/text.fs");
			TextRenderer.Create("assets/font/Minecraft.ttf", textShader, glm::ortho(0.0f, (float)window.getSize().x, 0.0f, (float)window.getSize().y));

			p.InitPlayer({ 0,0,0 });
		}

		//----------------------------------------------------------------------------------------------------------------------

		void OnUpdate() override {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			p.UpdatePlayer({ window.getPosition().x, window.getPosition().y }, CenterMouse, deltaTime);

			// activate shader
			mainShader.use();

			// pass projection matrix to shader (note that in this case it could change every frame)
			mainShader.setMat4("projection", p.projection);

			// camera/view transformation
			mainShader.setMat4("view", p.view);

			// create transformations

			// pass them to the shaders (3 different ways)
			mainShader.setMat4("view", p.view);
			mainShader.setMat4("projection", p.projection);
			//mainChunk.UpdateMesh();
			mainChunk.Draw(mainShader);

			mainSkybox.Draw(window.getSize().x, window.getSize().y, glm::mat4(glm::mat3(p.camera.GetViewMatrix())), p.projection);
			deltaTime = deltaTimer.restart().asSeconds();

			glDisable(GL_DEPTH_TEST);
			TextRenderer.Draw(std::to_string(FPS), { 25.0f, 700.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			glEnable(GL_DEPTH_TEST);
			window.display();

		}

	public:
		GameEngine() {}
		~GameEngine() {}

		void Start() override {
			OnCreate();
			while (GetEngineStatus() == EngineStatus::OK) {
				//Game Events
				OnUpdate();
				// Events
				OnEvent();
				// Input handler
				OnInput();


				//Get the framerate
				frameCount++;
				if (frameTimer.getElapsedTime().asSeconds() > 1) {
					uint32 secs = frameTimer.getElapsedTime().asSeconds();
					FPS = frameCount / secs;
					frameCount = 0;
					frameTimer.restart();
				}
			}
		}
	};
}