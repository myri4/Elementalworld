#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.h>
#include "entityes/Player.h"
#include "world/Chunk.h"

namespace wc {

	class GameEngine : public Engine {
	private:
		sf::RenderWindow window;
		uint32_t frameCount, FPS;
		sf::Clock frameTimer, deltaTimer;
		bool CenterMouse;
		float deltaTime;

		wc::Player p;

		std::array<Chunk, 10> chunks;

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
				width = (float)windowScript.GetNumber("screenWidth");
				height = (float)windowScript.GetNumber("screenHeight");
			}


			frameRateLimit = windowScript.GetNumber("framerateLimit");
			vsync = windowScript.GetBool("vsync");

			int32_t nrComponents, imgWidth, imgHeight;
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
			if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::F))
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::R)) glDisable(GL_CULL_FACE);
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
			mainShader.Create("shaderpacks/default/core.glsl");
			mainShader.use();
			int samplers[MaxTextures];
			for (int i = 0; i <= MaxTextureUnits(); i++) samplers[i] = i;
			mainShader.SetArray("u_Textures", MaxTextureUnits(), samplers);

			mainSkybox.Create("scripts/skybox.lua");
			CreateChunks();
			grassBlock.Create("scripts/grassblock.lua");
			grassBlock.material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.shininess = 32.0f;
			grassBlock.SendMaterialToShader(mainShader, "material");

			textShader.Create("shaderpacks/default/text.glsl");
			TextRenderer.Create("assets/font/Minecraft.ttf", textShader);

			p.InitPlayer({ 0,0,0 });
		}

		void DrawChunks() {
			for (int i = 0; i < chunks.size(); i++)
				chunks[i].Draw(mainShader);
		}

		void CreateChunks() {
			for (int i = 0; i < chunks.size(); i++)	chunks[i].Create({ 0,0,i });
		}
		
		//----------------------------------------------------------------------------------------------------------------------

		void OnUpdate() override {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			p.UpdatePlayer({ window.getPosition().x, window.getPosition().y }, CenterMouse, deltaTime);

			// activate shader
			mainShader.use();
			mainShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			mainShader.setMat4("projection", p.projection);

			// camera/view transformation
			mainShader.setMat4("view", p.GetView());		

			DrawChunks();

			//mainSkybox.Draw(glm::mat4(glm::mat3(p.GetView())), p.projection);
			deltaTime = deltaTimer.restart().asSeconds();

			glDisable(GL_DEPTH_TEST);
			TextRenderer.Draw(std::to_string(FPS), glm::vec2((float)window.getSize().x, (float)window.getSize().y) , { 25.0f, 700.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			TextRenderer.Draw("X: " + std::to_string(p.Position.x) + " Y: " + std::to_string(p.Position.y) + " Z: " + std::to_string(p.Position.z), glm::vec2((float)window.getSize().x, (float)window.getSize().y), { 25.0f, 660.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
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
					uint32_t secs = frameTimer.getElapsedTime().asSeconds();
					FPS = frameCount / secs;
					frameCount = 0;
					frameTimer.restart();
				}
			}
		}
	};
}