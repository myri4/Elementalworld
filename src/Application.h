#pragma once
#include "world/World.h"

//void GLAPIENTRY OpenGLDebugMessege(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char* message, const void* userParam) {
//	const char* src;
//	switch (source)
//	{
//	case GL_DEBUG_SOURCE_API:             src = "API"; break;
//	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = "Window System"; break;
//	case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "Shader Compiler"; break;
//	case GL_DEBUG_SOURCE_THIRD_PARTY:     src = "Third Party"; break;
//	case GL_DEBUG_SOURCE_APPLICATION:     src = "Application"; break;
//	case GL_DEBUG_SOURCE_OTHER:           src = "Other"; break;
//	}
//
//	switch (severity)
//	{
//	case GL_DEBUG_SEVERITY_HIGH:
//		WC_ERROR("[{0}] {1}", src, message);
//		break;
//
//	case GL_DEBUG_SEVERITY_MEDIUM:
//		WC_WARN("[{0}] {1}", src, message);
//		break;
//
//	case GL_DEBUG_SEVERITY_LOW:
//		WC_INFO("[{0}] {1}", src, message);
//		break;
//
//	case GL_DEBUG_SEVERITY_NOTIFICATION:
//		// WC_TRACE("[{0} {1} TRACE] {2}", src, typeStr, message);
//		break;
//	}
//}

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

		//Rml
		Rml::ElementDocument* document = nullptr;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(deltaTime);

				if (wc::Keyboard::getKey(wc::Keyboard::Key::F8))
					Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
			}
			Mouse::ShowMouse(mode != MenuMode::GAME || !hasFocus);			
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create({ 1280, 720 }, "Elementalworld");
			auto dev = VulkanContext::Create("Elementalworld", window);
			RendererContext::CreateSwapchain(window);
			RendererContext::CreateQueues(dev);

			vk::UploadContext::Init(RendererContext::graphicsQueue);
			RendererContext::CreateCommands();

			auto size = window.GetSize();
			render_interface.windowSize = size;

			// Begin by installing the custom interfaces.
			render_interface.Create(RendererContext::GetRenderPass());

			world.CreateScreen();

			world.Create(size);

			Rml::SetRenderInterface(&render_interface);
			Rml::SetSystemInterface(&system_interface);
			system_window = window;
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

			window.SetFramebufferSizeCallback([](GLFWwindow* windowHnadle, int width, int height) {
				glm::ivec2 size = {width, height};
				RendererContext::RecreateSwapchain(window);
				context->SetDimensions({ size.x, size.y });
				render_interface.windowSize = size;
				world.DestroyScreen();
				world.CreateScreen();
				});
		}

		Rml::DataModelHandle my_model;

		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			vk::CommandBuffer& cmd = RendererContext::GetCommandBuffer();

			RendererContext::Reset();

			//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			uint32_t swapchainImageIndex = RendererContext::AcquireNextImageKHR();

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
			if (mode == MenuMode::GAME)
				world.Update(deltaTime);

			RendererContext::Begin(swapchainImageIndex, window);

			if (mode == MenuMode::GAME)
				world.RenderGUI();

			if (world.renderGUI) 
				context->Render();

			render_interface.Flush();

			//finalize the render pass
			RendererContext::End();
			//finalize the command buffer (we can no longer add commands, but it can now be executed)
			cmd.End();

			RendererContext::Present(swapchainImageIndex);
			
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			VulkanContext::WaitIdle();

			Rml::Shutdown();
			world.Destroy();
			world.DestroyScreen();


			wc::render_interface.Destroy();

			vk::UploadContext::Destroy();

			vk::descriptorLayoutCache.Destroy();
			vk::descriptorAllocator.Destroy();

			RendererContext::Destroy();
			VulkanContext::Destroy();

			wc::window.Destroy();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() = default;
	};
}