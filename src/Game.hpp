#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>
#define Game 1

#if (Game == 0)

namespace wc {

	class GameEngine : public Engine {
	private:
		Window window;

		Clock deltaTimer;
		float deltaTime = 0.0f;

		gl::Text TextRenderer;
		gl::Texture guiTex;
		gl::VertexBuffer quadVBO;
		gl::VertexArray quadVAO;
		gl::Shader quadShader;
		gl::IndexBuffer quadEBO;

		//irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

		//----------------------------------------------------------------------------------------------------------------------
		void OnEvent() override {
			
		}
		//----------------------------------------------------------------------------------------------------------------------
		EngineStatus GetEngineStatus() override {

			if (window.isOpen()) return EngineStatus::OK;

			return EngineStatus::FAIL;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {

			if (Keyboard::isButtonPressed(Keyboard::Key::F)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (Keyboard::isButtonPressed(Keyboard::Key::R)) glDisable(GL_CULL_FACE);
			else glEnable(GL_CULL_FACE);
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

			//Face culling
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			
			TextRenderer.Create("assets/font/Minecraft.ttf", "shaderpacks/default/text.glsl");
			guiTex.load("assets/textures/misc/widgets.png");
			quadVAO.Create();
			quadVAO.Bind();
			glm::vec2 size = glm::vec2(200.0f, 60.0f);
			glm::vec2 pos = glm::vec2(150, 50);
			glm::vec2 startPos(0.0f, 0.0f);
			glm::vec2 endPos(200.0f, 60.0f);

			float excord = 200.0f / 200.0f;
			float eycord = 1.0f;

			float sxcord = 0.0f;
			float sycord = 0.0f;

			float vertices[] = {
				// positions  // texture coords
				 pos.x + size.x,  pos.y + size.y, excord, sycord,   // top right
				 pos.x + size.x,  pos.y,          excord, eycord,   // bottom right
				 pos.x,			  pos.y,          sxcord, eycord,   // bottom left
				 pos.x,			  pos.y + size.y, sxcord, sycord,   // top left 
			};

			glm::vec2 TexCoords[] = {
				glm::vec2(1.0f, 1.0f),
				glm::vec2(1.0f, 0.0f),
				glm::vec2(0.0f, 0.0f),
				glm::vec2(0.0f, 1.0f),
			};


			uint32_t indicies[] = { 0, 1, 2, 2, 3, 0 };
			quadEBO.Create(indicies, sizeof(indicies));
			quadVBO.Create(vertices, sizeof(vertices));
			gl::VertexAttribPointer(0, 2, 4 * sizeof(float), (void*)0);
			gl::VertexAttribPointer(1, 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			quadShader.Create("shaderpacks/default/2DRendererShader.glsl");
		}

		int width = 200, height = 60;

		//std::array<glm::vec2, 4> GetSpriteIndexCoords(const glm::vec2& startPos, const glm::vec2& endPos) {
		//	return {
		//		glm::vec2((coords.x * sprSize.x) / width, (coords.y * sprSize.y) / height),
		//		glm::vec2((coords.x * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
		//		glm::vec2(((coords.x + 1) * sprSize.x) / width, ((coords.y + 1) * sprSize.y) / height),
		//		glm::vec2(((coords.x + 1) * sprSize.x) / width, (coords.y * sprSize.y) / height)
		//	};
		//}

		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_DEPTH_TEST);
			TextRenderer.Draw("FPS: " + std::to_string((int)(1 / deltaTime)), window.GetSize(), { 25.0f, window.GetSize().y - 20 });

			glm::mat4 proj = glm::ortho(0.0f, window.GetSize().x, 0.0f, window.GetSize().y, -0.5f, 0.5f);
			quadShader.use();
			quadShader.setMat4("proj", proj);
			guiTex.Bind();
			quadVAO.Bind();
			quadEBO.Bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			glfwTerminate();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		GameEngine() {}
	};
}
#endif

#if (Game == 1)
#pragma once
#include "world/World.hpp"

namespace wc {

	class GameEngine : public Engine {
	private:
		wc::Window window;

		wc::Clock deltaTimer;
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
		void OnEvent() override {}
		//----------------------------------------------------------------------------------------------------------------------
		EngineStatus GetEngineStatus() override {

			if (window.isOpen()) return EngineStatus::OK;

			return EngineStatus::FAIL;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {

			world.OnEvent(deltaTime);
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

			//Stencil Test
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF); // each bit is written to the stencil buffer as is
			glStencilMask(0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)
			//glStencilFunc(GL_EQUAL, 1, 0xFF);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			//SoundEngine->play2D("assets/sounds/Alan Walker - The Spectre_wJnBTPUQS5A_youtube.mp3");

			window.setClearColor(glm::vec4(0.0f));

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
			gl::VertexAttribPointer(0, 2, sizeof(float) * 4, (void*)0);
			gl::VertexAttribPointer(1, 2, sizeof(float) * 4, (void*)(2 * sizeof(float)));

			TextRenderer.Create("assets/font/Minecraft.ttf", "shaderpacks/default/text.glsl");

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
			screenShader.setVec4("screenColor", world.screenColor);
			scrQuad.Bind();
			scrQuadA.Bind();
			screen.BindTexture();	// use the color attachment texture as the texture of the quad plane

			glDrawArrays(GL_TRIANGLES, 0, 6);
			TextRenderer.Draw(std::to_string((int)(1 / deltaTime)), window.GetSize(), { 25.0f, window.GetSize().y - 20 });
			TextRenderer.Draw("X: " + std::to_string(world.p.Position.x) + " Y: " + std::to_string(world.p.Position.y) + " Z: " + std::to_string(world.p.Position.z), window.GetSize(), { 25.0f, window.GetSize().y - 60 });
			TextRenderer.Draw("Pitch: " + std::to_string(world.p.camera.Pitch) + " Yaw: " + std::to_string(world.p.camera.Yaw), window.GetSize(), { 25.0f, window.GetSize().y - 100 });
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			glfwTerminate();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		GameEngine() {}
	};
}
#endif