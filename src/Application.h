#pragma once
#include "world/World.h"

namespace wc {	
	bool debug_menu = false;
	GameInstance world;

	class Application : public Engine {
	private:
		ImGuiTexture background;
		ImGuiTexture TitleSBox;
		ImGuiTexture Box;
		ImGuiTexture LogoLong;
		ImGuiTexture LogoBox;
		ImGuiTexture TitleBox;
		ImGuiTexture Chain1;
		ImGuiTexture Chain2;
		ImGuiTexture Chain3;
		ImGuiTexture LBox;
		ImGuiTexture MidBox;
		ImGuiTexture MidBox2;
		ImGuiTexture SBox;
		ImGuiTexture XSBox;
		ImGuiTexture XSBox2;

		Clock deltaTimer;
		float deltaTime = 0.f;

		wc::DescriptorPool imguiPool;

		ImVec2 scaleRes(ImVec2 position) {
			const ImVec2 malenRes = ImVec2(1920, 1080);
			ImVec2 windowRes = ImVec2(window.GetSize().x, window.GetSize().y);
			ImVec2 finalPos;
			finalPos.x = position.x / malenRes.x * windowRes.x;
			finalPos.y = position.y / malenRes.y * windowRes.y;
			return finalPos;
		}

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(deltaTime);
				if (Keyboard::getKey(Keyboard::Key::F8)) debug_menu = !debug_menu;
				if (mode == MenuMode::GAME && Keyboard::getKey(Keyboard::Key::Escape)) mode = MenuMode::ESCMENU;
				else if (mode == MenuMode::ESCMENU && Keyboard::getKey(Keyboard::Key::Escape)) {
					mode = MenuMode::GAME;
					Mouse::SetMousePosition(glm::vec2(window.GetSize().x / 2, window.GetSize().y / 2));
				}

			}
			Mouse::ShowMouse(mode != MenuMode::GAME || !hasFocus);			
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() override {
			window.Create({ 1280, 720 }, "Elementalworld");
			auto dev = VulkanContext::Create("Elementalworld", window);
			RendererContext::CreateSwapchain(window);
			RendererContext::CreateQueues(dev);

			wc::UploadContext::Init(RendererContext::graphicsQueue);
			RendererContext::CreateCommands();

			auto size = window.GetSize();
			render_interface.windowSize = size;

			render_interface.Create(RendererContext::GetRenderPass());

			world.CreateScreen();

			


			window.SetFramebufferSizeCallback([](GLFWwindow* windowHnadle, int width, int height) {
				glm::ivec2 size = {width, height};
				RendererContext::RecreateSwapchain(window);
				render_interface.windowSize = size;
				world.DestroyScreen();
				world.CreateScreen();
				});




			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000;
			pool_info.poolSizeCount = std::size(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;

			imguiPool.Create(pool_info);

			//this initializes the core structures of imgui
			ImGui::CreateContext();

			//this initializes imgui for SDL
			ImGui_ImplGlfw_InitForVulkan(window, false);

			//this initializes imgui for Vulkan
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanContext::GetInstance();
			init_info.PhysicalDevice = VulkanContext::GetPhysicalDevice();
			init_info.Device = VulkanContext::GetDevice();
			init_info.Queue = RendererContext::graphicsQueue;
			init_info.DescriptorPool = imguiPool;
			init_info.MinImageCount = 3;
			init_info.ImageCount = 3;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			ImGui_ImplVulkan_Init(&init_info, RendererContext::GetRenderPass());
			world.Create(size);

			UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
				ImGui_ImplVulkan_CreateFontsTexture(cmd);
				});

			//clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();


			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			//change background .img location from C: to in game forlders
			//[TO DO]: shorten the lenght
			background.Load("resourcepacks/default/textures/misc/screenshot.png");
			TitleSBox.Load("resourcepacks/default/textures/misc/TitleSBox.png");
			Box.Load("resourcepacks/default/textures/misc/Box.png");
			LogoLong.Load("resourcepacks/default/textures/misc/LongLogo.png");
			LogoBox.Load("resourcepacks/default/textures/misc/LogoBox.png");
			TitleBox.Load("resourcepacks/default/textures/misc/TitleBox.png");
			Chain1.Load("resourcepacks/default/textures/misc/Chain1.png");
			Chain2.Load("resourcepacks/default/textures/misc/Chain2.png");
			Chain3.Load("resourcepacks/default/textures/misc/Chain3.png");
			LBox.Load("resourcepacks/default/textures/misc/LBox.png");
			MidBox.Load("resourcepacks/default/textures/misc/MidBox.png");
			MidBox2.Load("resourcepacks/default/textures/misc/MidBox.png");
			SBox.Load("resourcepacks/default/textures/misc/SBox.png");
			XSBox.Load("resourcepacks/default/textures/misc/XSBox.png");
			XSBox2.Load("resourcepacks/default/textures/misc/XSBox.png");
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() override {
			deltaTime = deltaTimer.restart();
			wc::CommandBuffer& cmd = RendererContext::GetCommandBuffer();

			RendererContext::Reset();

			//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			uint32_t swapchainImageIndex = RendererContext::AcquireNextImageKHR();

			//imgui commands
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//loading debug menu
			if (mode == MenuMode::GAME && debug_menu == true) {
				world.RenderImGuiDebugMenu((int)(1.f / deltaTime));
			}

			//loading crosshair and console
			if (mode == MenuMode::GAME) {
				world.RenderImGuiCrosshair();
				world.RenderImGuiConsole();
			}

			//loading escape menu
			if (mode == MenuMode::ESCMENU) {
				world.RenderImGuiEscapeMenu();
			}

			//loading the main menu
			if (mode == MenuMode::MAINMENU) {

				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Elemental World", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				//ImGui::Text("Elemental World");
				//ImGui::ShowDemoWindow();
				//texture 
				{
				ImGui::SetCursorPos(scaleRes(ImVec2(412, 0)));
				ImGui::Image(TitleSBox, scaleRes(ImVec2(130, 117)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(439.5, 23.5)));
				ImGui::Image(Box, scaleRes(ImVec2(75, 70)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(1381, 0)));
				ImGui::Image(TitleSBox, scaleRes(ImVec2(130, 117)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(1408.5, 23.5)));
				ImGui::Image(Box, scaleRes(ImVec2(75, 70)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(540, 0)));
				ImGui::Image(TitleBox, scaleRes(ImVec2(841, 266)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(864, 23.5)));
				ImGui::Image(LogoBox, scaleRes(ImVec2(192, 173)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 52)));
				ImGui::Image(LogoLong, scaleRes(ImVec2(759, 83)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 208)));
				ImGui::Image(Chain1, scaleRes(ImVec2(46, 110)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(1294, 208)));
				ImGui::Image(Chain1, scaleRes(ImVec2(46, 110)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(540, 318)));
				ImGui::Image(LBox, scaleRes(ImVec2(841, 160)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 478)));
				ImGui::Image(Chain2, scaleRes(ImVec2(46, 82)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(1294, 478)));
				ImGui::Image(Chain2, scaleRes(ImVec2(46, 82)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(540, 559)));
				ImGui::Image(LBox, scaleRes(ImVec2(841, 160)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(720, 719)));
				ImGui::Image(Chain3, scaleRes(ImVec2(46, 82)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(1174, 719)));
				ImGui::Image(Chain3, scaleRes(ImVec2(46, 82)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(540, 801)));
				ImGui::Image(SBox, scaleRes(ImVec2(370, 160)));
				//-
				ImGui::SetCursorPos(scaleRes(ImVec2(1011, 801)));
				ImGui::Image(SBox, scaleRes(ImVec2(370, 160)));
				}

				//main menu buttons
				//-Singleplayer:
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 1.f));
				//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 359.5)));
				if (ImGui::ImageButton(MidBox, scaleRes(ImVec2(759, 77)))) {
					mode = MenuMode::WORLD_SELECTION;
				}
				//-Multiplayer
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 600.5)));
				if (ImGui::ImageButton(MidBox2, scaleRes(ImVec2(759, 77)))) {
					mode = MenuMode::MULTIPLAYER;
				}
				//-Settings
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 842.5)));
				if (ImGui::ImageButton(XSBox, scaleRes(ImVec2(279, 77)))) {
					mode = MenuMode::SETTINGS;
				}
				//-Quit Game
				ImGui::SetCursorPos(scaleRes(ImVec2(1052, 842.5)));
				if (ImGui::ImageButton(XSBox2, scaleRes(ImVec2(279, 77)))) {
					window.close();
				}
				ImGui::PopStyleColor(2);

				//debug menu
				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 100, 5));
				ImGui::Checkbox("Debug Menu", &debug_menu);
				if (debug_menu) {
					std::string text = "FPS: " + std::to_string((int)(1.f / deltaTime));
					ImGui::SetCursorPos(ImVec2(window.GetSize().x - 100, 25));
					ImGui::Text(text.c_str());
				}

				//background
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}

			//loading world selection
			if (mode == MenuMode::WORLD_SELECTION) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Elemental World", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				ImGui::Text("Select a World:");
				for (auto& p : std::filesystem::directory_iterator("worlds")) {
					if (p.is_directory()) {
						if (ImGui::Button(p.path().stem().string().c_str())) {
							mode = MenuMode::GAME;
							world.worldName = p.path().stem().string();
							world.Join("25.32.4.119", "321");
						}
					}
				}
				
				//create world
				ImGui::SetCursorPos(ImVec2(0, window.GetSize().y - 25));
				if (ImGui::Button("Create World")) {
					mode = MenuMode::WORLD_CREATION;
				}

				//background 
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}

			//loading world creation
			if (mode == MenuMode::WORLD_CREATION) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("World Creation", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				char str[256];
				if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
					ImGui::SetKeyboardFocusHere(0);
				ImGui::InputText("World Name", str, IM_ARRAYSIZE(str));
				if (ImGui::Button("Create")) {
					world.CreateNewWorld(str);
					world.Join("25.32.4.119", "321");
					mode = MenuMode::GAME;
					
				}
				//background 
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}
			ImGui::Render();

			if (mode == MenuMode::GAME)
				world.Update(deltaTime);

			RendererContext::Begin(swapchainImageIndex, window);

			if (mode == MenuMode::GAME)
				world.RenderGUI();

			render_interface.Flush();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
			//finalize the render pass
			RendererContext::End();
			//finalize the command buffer (we can no longer add commands, but it can now be executed)
			cmd.End();

			RendererContext::ExecuteGraphicsCommands();
			RendererContext::Present(swapchainImageIndex);
			
			window.display();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() override {
			VulkanContext::GetDevice().waitIdle();
			background.Destroy();
			TitleSBox.Destroy();
			Box.Destroy();
			LogoLong.Destroy();
			LogoBox.Destroy();
			TitleBox.Destroy();
			Chain1.Destroy();
			Chain2.Destroy();
			Chain3.Destroy();
			LBox.Destroy();
			MidBox.Destroy();
			MidBox2.Destroy();
			SBox.Destroy();
			XSBox.Destroy();
			XSBox2.Destroy();
			imguiPool.Destroy();
			ImGui_ImplVulkan_Shutdown();
			world.Destroy();
			world.DestroyScreen();


			wc::render_interface.Destroy();

			wc::UploadContext::Destroy();

			wc::descriptorLayoutCache.Destroy();
			wc::descriptorAllocator.Destroy();

			RendererContext::Destroy();
			VulkanContext::Destroy();

			wc::window.Destroy();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() = default;
	};
}