#pragma once
#include <wclibs/wclibspch.h>
#include <wclibs/Lua.hpp>
#include "world/Chunk.h"
#include <Utilitiess/State.h>

namespace wc{
class GameEngine : public Engine {
private:
	sf::RenderWindow window;
	uint32 frameCount, FPS;
	sf::Clock frameTimer, deltaTimer;
	bool CenterMouse;
	float deltaTime;
	
	glm::mat4 view;
	glm::mat4 projection;
	
	Chunk mainChunk;
	

	Camera mainCamera;
	gl::Skybox mainSkybox;
	gl::Text text;
	gl::Shader mainShader;
	gl::VertexBuffer chunkMeshBuffer;
	

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
	void Reload(const char* file) {
		window.close();
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

		window.create(sf::VideoMode(width, height), "Elementalworld", style, sf::ContextSettings(24, 0, windowScript.GetNumber("antialiasingLevel"), windowScript.GetNumber("majorVersion"), windowScript.GetNumber("minorVersion")));
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
	
	EngineStatus GetEngineStatus() override{
	
		if (window.isOpen()) return EngineStatus::OK;

		return EngineStatus::FAIL;
	}

	void OnInput() override {

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))      mainCamera.Move(FORWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))      mainCamera.Move(BACKWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))      mainCamera.Move(LEFT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))      mainCamera.Move(RIGHT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))  mainCamera.Move(UP, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) mainCamera.Move(DOWN, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) { mainCamera.Zoom = 10; mainCamera.MouseSensitivity = 18; }
		else {
			mainCamera.MouseSensitivity = 5;
			mainCamera.Zoom = 90;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) glDisable(GL_CULL_FACE);
		else glEnable(GL_CULL_FACE);


		if (!window.hasFocus()) CenterMouse = false;
		else  CenterMouse = true; 

		if (CenterMouse) window.setMouseCursorVisible(false);
		else window.setMouseCursorVisible(true);

	}

	//----------------------------------------------------------------------------------------------------------------------

	void OnCreate() {
		loadFromFile("config/window.lua");
		window.setActive();
		if (!gladLoadGL()) std::cout << "Failed to initialize GLAD" << "\n";

		// OpenGL state
		// ------------
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Stencil Test
		glCall(glStencilMask(0xFF)); // each bit is written to the stencil buffer as is
		glCall(glStencilMask(0x00)); // each bit ends up as 0 in the stencil buffer (disabling writes)
		glCall(glStencilFunc(GL_EQUAL, 1, 0xFF));

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);



		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		mainShader.Create("shaderpacks/default/core.vs", "shaderpacks/default/core.fs");
		mainShader.use();
		int samplers[MaxTextures];
		for (int i = 0; i < MaxTextures; i++) samplers[i] = i;
		mainShader.SetArray("u_Textures", MaxTextures, samplers);


		mainCamera.Create({ 0,0,0 });
		mainSkybox.Create("scripts/skybox.lua", 1);
		mainChunk.Create({ 0,0,0 });
		grassBlock.Create("scripts/grassblock.lua");
		text.Create("assets/font/Minecraft.ttf","shaderpacks/default/text.vs","shaderpacks/default/text.fs", glm::ortho(0.0f, static_cast<float>(window.getSize().x), 0.0f, static_cast<float>(window.getSize().y)));
	}
	
	//----------------------------------------------------------------------------------------------------------------------

	void OnUpdate() {
		mainCamera.UpdateCameraAngles({ sf::Mouse::getPosition().x, sf::Mouse::getPosition().y }, { window.getPosition().x, window.getPosition().y }, CenterMouse);
		

		// activate shader
		mainShader.use();

		view = mainCamera.GetViewMatrix();
		projection = glm::perspective(glm::radians(mainCamera.Zoom), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.0f);

		// pass projection matrix to shader (note that in this case it could change every frame)
		mainShader.setMat4("projection", projection);

		// camera/view transformation
		mainShader.setMat4("view", view);

		// create transformations
		view = mainCamera.GetViewMatrix();
		projection = glm::perspective(glm::radians(mainCamera.Zoom), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		
		// pass them to the shaders (3 different ways)
		mainShader.setMat4("view", view);
		mainShader.setMat4("projection", projection);
		//mainChunk.UpdateMesh();
		mainChunk.Draw(mainShader);

		view = glm::mat4(glm::mat3(mainCamera.GetViewMatrix()));
		mainSkybox.Draw(window.getSize().x, window.getSize().y, view, projection);

	}

public:
	GameEngine() {}
	~GameEngine() {}
	
	void Start() override {
		OnCreate();
		while (GetEngineStatus() == EngineStatus::OK){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//Game Events
		OnUpdate();
		// Events
		OnEvent();
		// Input handler
		OnInput();

		deltaTime = deltaTimer.restart().asSeconds();		
			glFrontFace(GL_CCW);
			glDisable(GL_DEPTH_TEST);
			text.Draw(std::to_string(1), { 25.0f, 700.0f }, 0.4f, glm::vec3(0.5, 0.8f, 0.2f));
			text.Draw("FPS: " + std::to_string(FPS), { 25.0f, 670.0f }, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
			//{ 25.0f, 670.0f }, 0.5f, glm::vec3(0.5, 0.8f, 0.2f)

			glEnable(GL_DEPTH_TEST);
			glFrontFace(GL_CW);
		window.display();

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