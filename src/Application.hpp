#pragma once
#define GLM_FORCE_INTRINSICS 
#include "world/World.hpp"

namespace wc {	

	class Application : public Engine {
	private:
		Clock deltaTimer;
		float deltaTime = 0.f;		

		//Menus
		MainMenu mainMenu;
		EscMenu escMenu;
		MultiplayerMenu multiplayerMenu;

		GameInstance world;

		//Rml
		Rml::ElementDocument* document = nullptr;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {
			return window.isOpen();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			//if (hasFocus) {
			//	if (mode == MenuMode::GAME) world.OnInput(hasFocus, deltaTime);
			//	else if (mode == MenuMode::INVENTORY) {
			//		world.p.inventory.OnInput(); 
			//		world.p.crafting.OnInput();
			//	}
			//	else if (mode == MenuMode::MAINMENU) 
			//		mainMenu.OnInput();
			//	else if (mode == MenuMode::ESCMENU)
			//		escMenu.OnInput();
			//	else if (mode == MenuMode::MULTIPLAYER)
			//		multiplayerMenu.OnInput();
			//}

			if (wc::Keyboard::getKey(wc::Keyboard::Key::F8)) 
				Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());			

			if (resized) { 
				auto size = window.GetSize();
				render_interface.viewportSize = size;
				context->SetDimensions({ size.x, size.y});
				Renderer2D::SetProjection(Renderer2D::Get2DProj(size));
				world.DestroyScreen();
				world.CreateScreen();
			}
			window.ShowMouse(true);
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create("config/window.yaml", "Elementalworld");
			// OpenGL state
			Renderer::enableDebuging();
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			world.CreateScreen();

			world.font.Load("assets/font/Minecraft.ttf", 128);

			glm::vec2 size = window.GetSize();

			Renderer2D::Init();
			Renderer2D::SetProjection(Renderer2D::Get2DProj(size));

			world.Create();

			mainMenu.OnCreate(world.font, 0.4f);
			escMenu.OnCreate(world.font, 0.4f);
			multiplayerMenu.OnCreate(world.font, 0.4f);


			// Begin by installing the custom interfaces.
			render_interface.Create();
			Rml::SetRenderInterface(&render_interface);
			Rml::SetSystemInterface(&system_interface);
			render_interface.viewportSize = size;
			// Now we can initialize RmlUi.
			Rml::Initialise();

			// Create a context next.
			context = Rml::CreateContext("main", Rml::Vector2i(size.x, size.y));
			if (!context)
			{
				Rml::Shutdown();
				WC_ERROR("Failed to initialize RML UI!");
			}

			// If you want to use the debugger, initialize it now.
			if (!Rml::Debugger::Initialise(context)) {
				Rml::Shutdown();
				WC_ERROR("Failed to initialize RML Debugger!");
			}

			// Fonts should be loaded before any documents are loaded.
			Rml::LoadFontFace("assets/font/Minecraft.ttf");

			// Now we are ready to load our document.
			document = context->LoadDocument("scripts/gui_scripts/test.rml");
			if (!document)
			{
				Rml::Shutdown();
				WC_ERROR("Failed to load document!");
			}
			document->Show();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			auto windsize = window.GetSize();
			
			context->Update();

			//if (mode == MenuMode::GAME) { 
			//	if (mainMenu.bSinglePlayer) {
			//		mainMenu.bSinglePlayer = false;
			//		world.Join(multiplayerMenu.ipTextbox.text, multiplayerMenu.playerName.text);
			//	}
			//
			//	if (bShouldConnect) {
			//		world.multiPlayer = true;
			//		world.Join(multiplayerMenu.ipTextbox.text, multiplayerMenu.playerName.text);
			//		bShouldConnect = false;
			//	}
			//	
			//	world.Update(deltaTime); 
			//}
			//else if(mode == MenuMode::INVENTORY) {
			//	world.p.inventory.Update(windsize, window.GetPos(), deltaTime, world.font); 
			//	world.p.crafting.Update(windsize, window.GetPos(), deltaTime, world.font);
			//}
			//else if (mode == MenuMode::MAINMENU) {
			//	//mainMenu.OnUpdate(world.font, 0.4f);
			//	window.ShowMouse(true);
			//}
			//else if (mode == MenuMode::ESCMENU) {
			//	escMenu.OnUpdate(world.font, 0.4f);
			//	window.ShowMouse(true);
			//}
			//else if (mode == MenuMode::MULTIPLAYER) {
			//	multiplayerMenu.OnUpdate(world.font, 0.4f);
			//	window.ShowMouse(true);
			//}

			glDisable(GL_DEPTH_TEST);
			Renderer::Clear();
			context->Render();
			//Renderer2D::Flush();
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			world.Destroy();
			Rml::Shutdown();
			window.SaveConfig("config/window.lua");
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() {}
	};
}