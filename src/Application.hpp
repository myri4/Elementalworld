#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>
#define Game 1

#if (Game == 0)

namespace wc {

	class Quad {
	private:
		glm::vec2 size;
		glm::vec2 pos;
		glm::vec2 startPos;
		glm::vec2 endPos;
	public:
		void SetSize(const glm::vec2& size) {this->size = size; }
		void SetPos(const glm::vec2& pos) { this->pos = pos; }
		void SetSpriteRect(const glm::vec2& startPos, const glm::vec2& endPos) { this->startPos = startPos; this->endPos = endPos;	}
		glm::vec2 GetSize() { return size; }
		glm::vec2 GetPos() { return pos; }
		glm::vec2 GetSpriteStart() { return startPos; }
		glm::vec2 GetSpriteEnd() { return endPos; }
	};

	class Renderer2D {
	private:
		static gl::VertexBuffer rVBO;
		static gl::VertexArray rVAO;
		static gl::IndexBuffer rEBO;
		static gl::Shader rShader;
	public:
		static void Create() {
			uint32_t indicies[] = { 0, 1, 2, 2, 3, 0 };
			rEBO.Create(indicies, sizeof(indicies));
			rVAO.Create();
			int maxSize = 1000; // bytes
			rVBO.Create(nullptr, maxSize);
			rVAO.VertexAttribPointer(0, 2, 4 * sizeof(float), (void*)0);
			rVAO.VertexAttribPointer(1, 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			rShader.Create("shaderpacks/default/2DRendererShader.glsl");
		}

		static void DrawQuad(Quad& quad, gl::Texture& tex) {
			float vertices[] = {
				// positions  // texture coords
				quad.GetPos().x + quad.GetSize().x, quad.GetPos().y + quad.GetSize().y, quad.GetSpriteEnd().x /    tex.GetSize().x, quad.GetSpriteStart().y / tex.GetSize().y,   // top right
				quad.GetPos().x + quad.GetSize().x, quad.GetPos().y,					 quad.GetSpriteEnd().x /   tex.GetSize().x, quad.GetSpriteEnd().y /   tex.GetSize().y,   // bottom right
				quad.GetPos().x,					 quad.GetPos().y,					 quad.GetSpriteStart().x / tex.GetSize().x, quad.GetSpriteEnd().y /   tex.GetSize().y,   // bottom left
				quad.GetPos().x,					 quad.GetPos().y + quad.GetSize().y, quad.GetSpriteStart().x / tex.GetSize().x, quad.GetSpriteStart().y / tex.GetSize().y,   // top left 
			};
			rVBO.Update(0, sizeof(vertices), vertices);
		}

		static void Draw() {
			
		}
	};

	class Application : public Engine {
	private:
		Window window;

		Clock deltaTimer;
		float deltaTime = 0.0f;

		gl::Text TextRenderer;
		gl::Texture tex;
		gl::VertexBuffer quadVBO;
		gl::VertexArray quadVAO;
		gl::Shader quadShader;
		gl::IndexBuffer quadEBO;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {

			if (window.isOpen()) return true;

			return false;
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create("config/window.lua", "Elementalworld");

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) WC_ERROR("Failed to initialize GLAD");

			// OpenGL state
			EnableGLDebuging();
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			
			TextRenderer.Create("assets/font/Minecraft.ttf", "shaderpacks/default/text.glsl");
			tex.load("assets/textures/misc/widgets.png");

			Quad quad;
			quad.SetSpriteRect(glm::vec2(0.0f, 0.0f), glm::vec2(200.0f, 60.0f));
			quad.SetPos(glm::vec2(150, 50));
			quad.SetSize(glm::vec2(200.0f, 60.0f));

			uint32_t indicies[] = { 0, 1, 2, 2, 3, 0 };
			quadEBO.Create(indicies, sizeof(indicies));
			quadVAO.Create();
			float vertices[] = {
				// positions  // texture coords
				quad.GetPos().x + quad.GetSize().x, quad.GetPos().y + quad.GetSize().y, quad.GetSpriteEnd().x / tex.GetSize().x, quad.GetSpriteStart().y / tex.GetSize().y,   // top right
				quad.GetPos().x + quad.GetSize().x, quad.GetPos().y,					 quad.GetSpriteEnd().x / tex.GetSize().x, quad.GetSpriteEnd().y / tex.GetSize().y,   // bottom right
				quad.GetPos().x,					 quad.GetPos().y,					 quad.GetSpriteStart().x / tex.GetSize().x, quad.GetSpriteEnd().y / tex.GetSize().y,   // bottom left
				quad.GetPos().x,					 quad.GetPos().y + quad.GetSize().y, quad.GetSpriteStart().x / tex.GetSize().x, quad.GetSpriteStart().y / tex.GetSize().y,   // top left 
			};
			quadVBO.Create(vertices, sizeof(vertices));
			quadVAO.VertexAttribPointer(0, 2, 4 * sizeof(float), (void*)0);
			quadVAO.VertexAttribPointer(1, 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			quadShader.Create("shaderpacks/default/2DRendererShader.glsl");
		}

		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			glClear(GL_COLOR_BUFFER_BIT);

			TextRenderer.Draw("FPS: " + std::to_string((int)(1 / deltaTime)), window.GetSize(), { 25.0f, window.GetSize().y - 20 });

			glm::mat4 proj = glm::ortho(0.0f, window.GetSize().x, 0.0f, window.GetSize().y, -0.5f, 0.5f);
			quadShader.use();
			quadShader.setMat4("proj", proj);
			tex.Bind();
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
		Application() {}
	};
}
#endif

#if (Game == 1)
#pragma once
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

		gl::Text TextRenderer;

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
			scrQuad.Bind();
			scrQuadA.Bind();
			screen.BindTexture();	// use the color attachment texture as the texture of the quad plane

			glDrawArrays(GL_TRIANGLES, 0, 6);
			TextRenderer.Draw("FPS: " + std::to_string((int)(1 / deltaTime)), window.GetSize(), { 25.0f, window.GetSize().y - 20 });
			TextRenderer.Draw("X: " + std::to_string(world.p.Position.x) + " Y: " + std::to_string(world.p.Position.y) + " Z: " + std::to_string(world.p.Position.z), window.GetSize(), { 25.0f, window.GetSize().y - 60 });
			TextRenderer.Draw("Pitch: " + std::to_string(world.p.camera.Pitch) + " Yaw: " + std::to_string(world.p.camera.Yaw), window.GetSize(), { 25.0f, window.GetSize().y - 100 });
			TextRenderer.Draw(
				 "ChunkX: " + std::to_string(glm::floor(world.p.Position.x / chunkSize)) +
				" ChunkY: " + std::to_string(glm::floor(world.p.Position.y / chunkSize)) +
				" ChunkZ: " + std::to_string(glm::floor(world.p.Position.z / chunkSize)) + 
				" WaveTime: " + std::to_string(world.waveTime),
				window.GetSize(), { 25.0f, window.GetSize().y - 140 });
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
#endif