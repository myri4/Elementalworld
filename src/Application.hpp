#pragma once
#define GLM_FORCE_INTRINSICS 
#include "world/World.hpp"

namespace wc {	

		GameInstance world;

		struct MyData {
			int fps = 60;
			Rml::String ip = "25.32.4.119";
			Rml::String playerName;
		} my_data;
	class Application : public Engine {
	private:
		Clock deltaTimer;
		float deltaTime = 0.f;		

		//Menus
		EscMenu escMenu;


		//Rml
		Rml::ElementDocument* document = nullptr;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override {
			return window.isOpen();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(hasFocus, deltaTime);
				//else if (mode == MenuMode::INVENTORY) {
				//	world.p.inventory.OnInput(); 
				//	world.p.crafting.OnInput();
				//}
				//else if (mode == MenuMode::ESCMENU)
				//	escMenu.OnInput();
			}

			if (wc::Keyboard::getKey(wc::Keyboard::Key::F8)) 
				Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());			

			if (resized) { 
				auto size = window.GetSize();
				context->SetDimensions({ size.x, size.y});
				Renderer2D::m_Data.windowSize = size;
				world.DestroyScreen();
				world.CreateScreen();
			}
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

			world.font.Load("assets/font/Minecraft.ttf");

			auto size = window.GetSize();

			Renderer2D::Init();
			Renderer2D::m_Data.windowSize = size;

			world.Create();

			escMenu.OnCreate(world.font, 0.4f);

			// Begin by installing the custom interfaces.
			render_interface.Create();
			Rml::SetRenderInterface(&render_interface);
			Rml::SetSystemInterface(&system_interface);
			system_interface.window = window;
			// Now we can initialize RmlUi.
			Rml::Initialise();

			// Create a context next.
			context = Rml::CreateContext("main", Rml::Vector2i(size.x, size.y));
			if (!context)
			{
				Rml::Shutdown();
				WC_ERROR("Failed to initialize RML UI!");
			}
			context->SetDensityIndependentPixelRatio(window.getContentScale());
			context->SetDimensions({ size.x, size.y });

			// If you want to use the debugger, initialize it now.
			if (!Rml::Debugger::Initialise(context)) {
				Rml::Shutdown();
				WC_ERROR("Failed to initialize RML Debugger!");
			}

			// Fonts should be loaded before any documents are loaded.
			Rml::LoadFontFace("assets/font/Minecraft.ttf");


			Rml::DataModelConstructor constructor = context->CreateDataModel("my_model");
			if (constructor) {
				constructor.Bind("FPS", &my_data.fps);
				constructor.Bind("mode", &mode);
				constructor.Bind("ip", &my_data.ip);
				constructor.Bind("playerName", &my_data.playerName);
				constructor.BindEventCallback("JoinSinglePlayer", &Application::JoinSinglePlayer);
				constructor.BindEventCallback("JoinMultiPlayer", &Application::JoinMultiPlayer);
				constructor.BindEventCallback("JoinMultiPlayerMenu", &Application::JoinMultiPlayerMenu);
				my_model = constructor.GetModelHandle();
			}

			// Now we are ready to load our document.
			document = context->LoadDocument("scripts/gui_scripts/test.html");
			if (!document)
			{
				Rml::Shutdown();
				WC_ERROR("Failed to load document!");
			}
			document->Show();
		}


		static void JoinSinglePlayer(Rml::DataModelHandle model_handle, Rml::Event& ev, const Rml::VariantList& arguments) {
			mode = MenuMode::GAME;
			world.Join(my_data.ip.c_str(), my_data.playerName.c_str());
		}

		static void JoinMultiPlayerMenu(Rml::DataModelHandle model_handle, Rml::Event& ev, const Rml::VariantList& arguments) {
			mode = MenuMode::MULTIPLAYER;
		}

		static void JoinMultiPlayer(Rml::DataModelHandle model_handle, Rml::Event& ev, const Rml::VariantList& arguments) {
			mode = MenuMode::GAME;
			world.multiPlayer = true;
			world.Join(my_data.ip.c_str(), my_data.playerName.c_str());
		}

		Rml::DataModelHandle my_model;

		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			auto windsize = window.GetSize();

			//Update Rml
			my_data.fps = (int)(1.f / deltaTime);
			my_model.DirtyVariable("FPS");
			my_model.DirtyVariable("mode");

			context->Update();

			if (mode == MenuMode::GAME)
				world.Update(deltaTime);			
			//else if(mode == MenuMode::INVENTORY) {
			//	world.p.inventory.Update(windsize, window.GetPos(), deltaTime, world.font); 
			//	world.p.crafting.Update(windsize, window.GetPos(), deltaTime, world.font);
			//}
			//else if (mode == MenuMode::ESCMENU) {
			//	escMenu.OnUpdate(world.font, 0.4f);
			//	window.ShowMouse(true);
			//}
			if (mode != MenuMode::GAME) Renderer::Clear();
			context->Render();
			render_interface.Flush();
			Renderer2D::Flush();
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