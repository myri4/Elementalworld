#pragma once
#include <wclibs/pch.hpp>
#include <gl/glErrors.hpp>
#include <GUI/Renderer2D.hpp>
#define Game 1

#if (Game == 0)

namespace wc {

	class AssetManager {
	public:
		AssetManager() {}
		void Create(const uint32_t& arraySize, const uint32_t& width, const uint32_t& height, const uint8_t& nrOfComponents = 4) { texArr.Create(arraySize, width, height, nrOfComponents); }

		uint32_t LoadTexture(const std::string& file)
		{
			if (m_TextureCache.find(file) != m_TextureCache.end()) return m_TextureCache[file];  // If this texture exist

			uint32_t location = 0;

			int fnrComponents = 0, fwidth = 0, fheight = 0;
			auto* data = stbi_load(file.c_str(), &fwidth, &fheight, &fnrComponents, 0);
			if (data) {
				texArr.AddTexture(data);
				location = texArr.GetGeneretedTextures() - 1;
				m_TextureCache[file] = location;
			}
			else WC_ERROR("Cannot find file at location: {0}", file);
			return location;
		}

		void Bind() { texArr.Bind(); }
	private:
		std::unordered_map<std::string, int> m_TextureCache;
		gl::TextureArray texArr;
	};

	class Application : public Engine {
	private:
		Window window;

		Clock deltaTimer;
		float deltaTime = 0.0f;

		gl::Text TextRenderer;
		gl::Texture tex;
		AssetManager texArr;

		Renderer2D render;
		Quad quad;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {

			if (window.isOpen()) return true;

			return false;
		}
		int loc = 0;
		int loc2 = 0;
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
			tex.load("assets/awesomeface.png");

			quad.SetSpriteRect(glm::vec2(0.0f), glm::vec2(512));
			quad.SetPos(glm::vec2(150, 200));
			quad.SetSize(glm::vec2(128));

			int fnrComponents = 0, fwidth = 0, fheight = 0;
			auto* data = stbi_load("assets/textures/block/glass.png", &fwidth, &fheight, &fnrComponents, 0);
			texArr.Create(60, 32, 32);
			loc = texArr.LoadTexture("assets/textures/block/gravel.png");
			loc2 = texArr.LoadTexture("assets/textures/block/water.png");

			tex.SetData(data, fwidth, fheight);

			render.Create();
			textPosX = 25.0f;
			textPosY = 328.0f;
		}

		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			glClear(GL_COLOR_BUFFER_BIT);

			if (textPosX > 278.f) { textPosX = 150.0f; textPosY -= 20.0f; }
			if (textPosX > 200.f) textPosY = 328;
			textPosX += 25 * deltaTime;

			TextRenderer.Draw("FPS: " + std::to_string((int)(1 / deltaTime)), window.GetSize(), { textPosX, textPosY });

			texArr.Bind();
			glm::mat4 proj = glm::ortho(0.0f, window.GetSize().x, 0.0f, window.GetSize().y, -0.5f, 0.5f);
			render.DrawQuad(quad, tex, loc2);
			render.Draw(proj);

			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			glfwTerminate();
		}
		float textPosX;
		float textPosY;
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
			TextRenderer.Draw("FPS: " + std::to_string((int)(1 / deltaTime)) + " Frametime: " + std::to_string(deltaTime), window.GetSize(), { 25.0f, window.GetSize().y - 20 });
			TextRenderer.Draw("X: " + std::to_string(world.p.Position.x) + " Y: " + std::to_string(world.p.Position.y) + " Z: " + std::to_string(world.p.Position.z), window.GetSize(), { 25.0f, window.GetSize().y - 60 });
			TextRenderer.Draw("Pitch: " + std::to_string(world.p.camera.Pitch) + " Yaw: " + std::to_string(world.p.camera.Yaw), window.GetSize(), { 25.0f, window.GetSize().y - 100 });
			TextRenderer.Draw(
				 "ChunkX: " + std::to_string(glm::floor(world.p.Position.x / chunkSize)) +
				" ChunkY: " + std::to_string(glm::floor(world.p.Position.y / chunkSize)) +
				" ChunkZ: " + std::to_string(glm::floor(world.p.Position.z / chunkSize)),
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