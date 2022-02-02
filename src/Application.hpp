#pragma once
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_INTRINSICS 
#include "world/World.hpp"
#include "GUI/Textbox.hpp"
#include "GUI/Button.hpp"

namespace wc {

	class Application : public Engine {
	private:
		Clock deltaTimer;
		float deltaTime = 0.f;

		// FrameBuffer stuff
		gl::VertexBuffer scrQuad;
		gl::VertexArray scrQuadA;
		gl::FrameBuffer screen;
		gl::Shader screenShader;
		gl::Texture scrTexture;

		//Menus
		MainMenu mainMenu;
		EscMenu escMenu;
		MultiplayerMenu multiplayerMenu;

		Singleplayer world;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {
			return window.isOpen();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(hasFocus, deltaTime);
				else if (mode == MenuMode::INVENTORY) {
					world.p.inventory.OnInput(); 
					world.p.crafting.OnInput();
				}
				else if (mode == MenuMode::MAINMENU) 
					mainMenu.OnInput();
				else if (mode == MenuMode::ESCMENU)
					escMenu.OnInput();
				else if (mode == MenuMode::MULTIPLAYER)
					multiplayerMenu.OnInput();
			}

			if (resized) { 
				screen.Destroy();
				scrTexture.Destroy(); 
				gl::TextureProps scrProps;
				scrProps.internalFormat = GL_RGB16F;
				scrProps.SetSize(window.GetSize());
				scrTexture.Create(scrProps);
				screen.Create(scrProps.Width, scrProps.Height, scrProps.samples);
				screen.addTexture(scrTexture);
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create("config/window.lua", "Elementalworld");
			// OpenGL state
			Renderer::enableDebuging();
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//Anti aliasing
			glEnable(GL_MULTISAMPLE);

			//Depth testing
			glEnable(GL_DEPTH_TEST);

			screenShader.Create("shaderpacks/default/screenShader.glsl");

			gl::TextureProps scrProps;
			scrProps.internalFormat = GL_RGB16F;
			scrProps.SetSize(window.GetSize());

			scrTexture.Create(scrProps);
			screen.Create(scrProps.Width, scrProps.Height, scrProps.samples);
			screen.addTexture(scrTexture);

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions   // texCoords
				-1.0f, -1.0f,  0.0f, 0.0f,
				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				 1.0f, -1.0f,  1.0f, 0.0f,
				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f,  1.0f,  1.0f, 1.0f,
			};

			scrQuad.Create(quadVertices, sizeof(quadVertices), 0);
			scrQuadA.Create();
			scrQuadA.VertexAttribPointer(0, 2, 0);
			scrQuadA.VertexAttribPointer(1, 2, 2 * sizeof(float));
			scrQuadA.AddVertexBuffer(scrQuad, sizeof(float) * 4);
			world.font.Load("assets/font/Minecraft.ttf", 128);

			Renderer2D::Init();

			world.Create();
			world.p.inventory.Create();
			world.p.crafting.Create();
			mainMenu.OnCreate(world.font, 0.4f);
			escMenu.OnCreate(world.font, 0.4f);
			multiplayerMenu.OnCreate(world.font, 0.4f);
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			auto windsize = window.GetSize();
			Renderer2D::SetProjection(Renderer2D::Get2DProj(windsize));
			screen.Bind();
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			if (mode == MenuMode::GAME) { 
				if (multiplayerMenu.shouldConnect) {
					world.Connect(multiplayerMenu.ipTextbox.text, multiplayerMenu.playerName.text);
					multiplayerMenu.shouldConnect = false;
					world.multiPlayer = true;
				}
				world.Update(deltaTime); 
			}
			else if(mode == MenuMode::INVENTORY) {
				world.p.inventory.Update(windsize, window.GetPos(), deltaTime, world.font); 
				world.p.crafting.Update(windsize, window.GetPos(), deltaTime, world.font);
			}
			else if (mode == MenuMode::MAINMENU) {
				mainMenu.OnUpdate(world.font, 0.4f);
				window.ShowMouse(true);
			}
			else if (mode == MenuMode::ESCMENU) {
				escMenu.OnUpdate(world.font, 0.4f);
				window.ShowMouse(true);
			}
			else if (mode == MenuMode::MULTIPLAYER) {
				multiplayerMenu.OnUpdate(world.font, 0.4f);
				window.ShowMouse(true);
			}
			
			
			glDisable(GL_DEPTH_TEST);
			screen.blit(windsize.x, windsize.y);
			screen.unbind();

			screenShader.use();
			scrQuadA.Bind();
			scrTexture.Bind(); // use the color attachment texture as the texture of the quad plane			
			
			Renderer::DrawArrays(6);			
			Renderer2D::Flush();
			Renderer2D::FlushLines();
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			world.Destroy();
			window.SaveConfig("config/window.lua");
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() {}
	};
}