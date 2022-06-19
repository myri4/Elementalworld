#pragma once
#define GLM_FORCE_INTRINSICS 
#include "world/World.hpp"

void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
	const char* src;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             src = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     src = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     src = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           src = "Other"; break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		WC_ERROR("[{0}] {1}", src, message);
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		WC_WARN("[{0}] {1}", src, message);
		break;

	case GL_DEBUG_SEVERITY_LOW:
		WC_INFO("[{0}] {1}", src, message);
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		// WC_TRACE("[{0} {1} TRACE] {2}", src, typeStr, message);
		break;
	}
}

namespace wc {	

	GameInstance world;

	struct MyData {
		int fps = 0;
		float frametime = 0.f;
		Rml::String ip = "25.32.4.119";
		Rml::String playerName = "321";
		Rml::String chatArguments = "";
		glm::ivec3 chunkPosition = glm::ivec3(0);
	} my_data;

	enum MenuMode { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU };
	uint32_t mode = MenuMode::MAINMENU; // @TODO: Remove it from here and make a file that needs to be include everywhere

	int changeMenu(lua_State* L) {
		mode = lua_tonumber(L, 1);
		return 1;
	}

	int JoinGame(lua_State* L) {
		world.multiPlayer = (bool)lua_tonumber(L, 1);
		world.Join(my_data.ip.c_str(), my_data.playerName.c_str());
		return 1;
	}

	lua_State* guiState = nullptr;

	class Application : public Engine {
	private:
		Clock deltaTimer;
		float deltaTime = 0.f;

		gl::Fence fence;

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
				if (mode == MenuMode::GAME) world.OnInput(deltaTime);

				if (wc::Keyboard::getKey(wc::Keyboard::Key::F8))
					Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
			}
			Mouse::ShowMouse(mode != MenuMode::GAME || !hasFocus);

			if (resized) { 
				auto size = window.GetSize();
				glViewport(0, 0, size.x, size.y);
				context->SetDimensions({ size.x, size.y});
				render_interface.windowSize = size;
				world.DestroyScreen();
				world.CreateScreen();
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create({1280, 720}, "Elementalworld");

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) WC_ERROR("Failed to initialize GLAD");
			// OpenGL state
			int flags;
			glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
			//if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) 
			{
				// initialize debug output 
				glDebugMessageCallback(OpenGLDebugMessege, nullptr);
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
			}
			// ------------
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			world.CreateScreen();

			auto size = window.GetSize();
			render_interface.windowSize = size;

			world.Create();

			// Begin by installing the custom interfaces.
			render_interface.Create();
			Rml::SetRenderInterface(&render_interface);
			Rml::SetSystemInterface(&system_interface);
			system_interface.window = window;
			// Now we can initialize RmlUi.
			Rml::Initialise();
			
			//Initialize lua
			guiState = luaL_newstate();
			
			luaL_openlibs(guiState);
			
			lua_register(guiState, "changeMenu", changeMenu);
			lua_register(guiState, "JoinGame", JoinGame);
			
			Rml::Lua::Initialise(guiState);
			// Create a context next.
			context = Rml::CreateContext("main", Rml::Vector2i(size.x, size.y));
			if (!context)
			{
				Rml::Shutdown();
				WC_ERROR("Failed to initialize RML UI!");
			}
			context->SetDensityIndependentPixelRatio(window.getContentScale());
			
			// If you want to use the debugger, initialize it now.
			if (!Rml::Debugger::Initialise(context)) {
				Rml::Shutdown();
				WC_ERROR("Failed to initialize RML Debugger!");
			}
			
			// Fonts should be loaded before any documents are loaded.
			Rml::LoadFontFace("resourcepacks/default/font/Minecraft.ttf");
			
			Rml::DataModelConstructor constructor = context->CreateDataModel("globalData");
			if (constructor) {
				constructor.Bind("FPS", &my_data.fps);
				constructor.Bind("FRAME_TIME", &my_data.frametime);
				constructor.Bind("mode", &mode);
				constructor.Bind("posX", &world.p.Position.x);
				constructor.Bind("posY", &world.p.Position.y);
				constructor.Bind("posZ", &world.p.Position.z);
				constructor.Bind("chunkX", &my_data.chunkPosition.x);
				constructor.Bind("chunkY", &my_data.chunkPosition.y);
				constructor.Bind("chunkZ", &my_data.chunkPosition.z);
				constructor.Bind("pitch", &world.p.rotation.x);
				constructor.Bind("yaw", &world.p.rotation.y);
				constructor.Bind("ip", &my_data.ip);
				constructor.Bind("playerName", &my_data.playerName);
				constructor.Bind("chatArguments", &my_data.chatArguments);
				my_model = constructor.GetModelHandle();
			}
			
			// Now we are ready to load our document.
			document = context->LoadDocument("resourcepacks/default/gui/gui.html");
			if (!document)
			{
				Rml::Shutdown();
				WC_ERROR("Failed to load document!");
			}
			document->Show();
		}

		Rml::DataModelHandle my_model;

		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			
			//Update Rml
			my_data.fps = (int)(1.f / deltaTime);
			my_data.frametime = deltaTime;
			my_data.chunkPosition = getChunkPos(world.p.Position);
			my_model.DirtyVariable("FPS");
			my_model.DirtyVariable("FRAME_TIME");
			my_model.DirtyVariable("mode");
			my_model.DirtyVariable("posX");
			my_model.DirtyVariable("posY");
			my_model.DirtyVariable("posZ");
			my_model.DirtyVariable("chunkX");
			my_model.DirtyVariable("chunkY");
			my_model.DirtyVariable("chunkZ");
			my_model.DirtyVariable("pitch");
			my_model.DirtyVariable("yaw");
			
			context->Update();
			fence.wait();
			if (mode == MenuMode::GAME)
				world.Update(deltaTime);
			if (mode != MenuMode::GAME)
				glClear(GL_COLOR_BUFFER_BIT);

			context->Render();
			render_interface.Flush();
			fence.lock();
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			world.Destroy();
			Rml::Shutdown();
			wc::window.Destroy();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() {}
	};
}