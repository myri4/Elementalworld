#pragma once
#include "world/World.h"

namespace wc {	
	GameInstance gameInstance;

	char serverIP[128] = "";

	class Application {
	private:
		AssetManager m_AssetManager;

		char newWorldName[256];
		Texture background;
		Texture TitleSBox;
		Texture Box;
		Texture LogoLong;
		Texture LogoBox;
		Texture TitleBox;
		Texture Chain1;
		Texture Chain2;
		Texture Chain3;
		Texture LBox;
		Texture MidBox;
		Texture SBox;
		Texture XSBox;

		Clock deltaTimer;
		float deltaTime = 0.f;		

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------

		void Resize() {
			int width = 0, height = 0;
			glfwGetFramebufferSize(window, &width, &height);
			while (width == 0 || height == 0) {
				glfwGetFramebufferSize(window, &width, &height);
				glfwWaitEvents();
			}

			VulkanContext::GetDevice().WaitIdle();
			RendererContext::DestroyDefaultRenderPass();
			window.DestoySwapchain();
			window.CreateSwapchain(VulkanContext::GetPhysicalDevice(), VulkanContext::GetDevice(), VulkanContext::GetInstance());
			RendererContext::CreateDefaultRenderPass(window);

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanContext::GetInstance();
			init_info.PhysicalDevice = VulkanContext::GetPhysicalDevice();
			init_info.Device = VulkanContext::GetDevice();
			init_info.Queue = RendererContext::GetGraphicsQueue();
			init_info.DescriptorPool = descriptorAllocator.GetCurrentPool();
			init_info.MinImageCount = 2;
			init_info.ImageCount = 2;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			ImGui_ImplVulkan_Init(&init_info, RendererContext::GetRenderPass());
			ImGui_ImplGlfw_InitForVulkan(window, false);
		}

		void OnInput() {
			window.poolEvents();

			if (window.hasFocus()) {
				if (menuMode == MenuMode::GAME) gameInstance.OnInput(deltaTime);
				
				if (menuMode == MenuMode::ESCMENU && Keyboard::getKey(Keyboard::Key::Escape))
					ChangeMenu(MenuMode::GAME);


				if (menuMode == MenuMode::GAME && Keyboard::getKey(Keyboard::Key::Escape)) ChangeMenu(MenuMode::ESCMENU);

				if (Keyboard::getKey(Keyboard::Key::F3)) Resize();
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() {
			VulkanContext::Create();
			CreateAudioEngine();


			WindowCreateInfo windowInfo;
			windowInfo.width = 1280;
			windowInfo.height = 720;
			windowInfo.resizeable = false;
			windowInfo.appName = "Elementalworld";
			//windowInfo.startFullscreen = true;
			window.Create(windowInfo);
			
			RendererContext::CreateDefaultRenderPass(window);

			UploadContext::Init();
			RendererContext::CreateCommands();

			wc::descriptorAllocator.Create();
			
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.IniFilename = nullptr;
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

			io.FontDefault = io.Fonts->AddFontFromFileTTF((GetAssetPath() + "/font/Minecraft.ttf").c_str(), 15.f);

			ImGui_ImplGlfw_InitForVulkan(window, false);

			ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return vkGetInstanceProcAddr(VulkanContext::GetInstance(), function_name); });
			//this initializes imgui for Vulkan
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanContext::GetInstance();
			init_info.PhysicalDevice = VulkanContext::GetPhysicalDevice();
			init_info.Device = VulkanContext::GetDevice();
			init_info.Queue = RendererContext::GetGraphicsQueue();
			init_info.DescriptorPool = descriptorAllocator.GetCurrentPool();
			init_info.MinImageCount = 2;
			init_info.ImageCount = 2;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&init_info, RendererContext::GetRenderPass());

			UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
				ImGui_ImplVulkan_CreateFontsTexture(cmd);
				});

			//@TODO: shorten the length
			background = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/screenshot.png");
			TitleSBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/TitleSBox.png");
			Box = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/Box.png");
			LogoLong = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/LongLogo.png");
			LogoBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/LogoBox.png");
			TitleBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/TitleBox.png");
			Chain1 = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/Chain1.png");
			Chain2 = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/Chain2.png");
			Chain3 = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/Chain3.png");
			LBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/LBox.png");
			MidBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/MidBox.png");
			SBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/SBox.png");
			XSBox = m_AssetManager.LoadImage(GetAssetPath() + "/textures/misc/XSBox.png");

			//clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();
			gameInstance.Create(m_AssetManager);
		}
		//----------------------------------------------------------------------------------------------------------------------
		static void HelpMarker(const char* desc)
		{
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
		void OnUpdate() {
			deltaTime = deltaTimer.restart();
			CommandBuffer& cmd = RendererContext::mainCommandBuffer;

			uint32_t swapchainImageIndex = 0;

			VkResult result = RendererContext::AcquireNextImageKHR(window, swapchainImageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				VulkanContext::GetDevice().WaitIdle();
				WC_INFO("Resize 1");
				Resize();
				return;
			}
			
			
			//imgui commands
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//loading crosshair and console
			if (menuMode == MenuMode::GAME)
				gameInstance.RenderGUI(deltaTime, m_AssetManager);			

			//loading escape menu
			if (menuMode == MenuMode::ESCMENU) gameInstance.RenderImGuiEscapeMenu();

			//loading the main menu
			if (menuMode == MenuMode::MAINMENU) {

				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Elemental World", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				//ImGui::Text("Elemental World");
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
				if (ImGui::ImageButton(MidBox, scaleRes(ImVec2(759, 77)))) ChangeMenu(MenuMode::WORLD_SELECTION);				
				//-Multiplayer
				ImGui::PushID(1);
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 600.5)));
				if (ImGui::ImageButton(MidBox, scaleRes(ImVec2(759, 77)))) ChangeMenu(MenuMode::MULTIPLAYER);
				ImGui::PopID();
				//-Settings
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 842.5)));
				if (ImGui::ImageButton(XSBox, scaleRes(ImVec2(279, 77)))) ChangeMenu(MenuMode::SETTINGS);
				
				//-Quit Game
				ImGui::PushID(1);
				ImGui::SetCursorPos(scaleRes(ImVec2(1052, 842.5f)));
				if (ImGui::ImageButton(XSBox, scaleRes(ImVec2(279, 77)))) window.close();
				ImGui::PopID();
				
				ImGui::PopStyleColor(2);

				//background
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}

			//loading world selection
			if (menuMode == MenuMode::WORLD_SELECTION) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Elemental World", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				ImGui::Text("Select a World:");
				for (auto& p : std::filesystem::directory_iterator("worlds")) {
					if (p.is_directory()) {
						if (std::filesystem::exists(p.path() / "world.properties")) {
							if (ImGui::Button(p.path().stem().string().c_str())) {
								ChangeMenu(MenuMode::GAME);
								gameInstance.worldName = p.path().stem().string();
								gameInstance.LoadWorld();
							}
						}
					}
				}

				//create world
				ImGui::SetCursorPos(ImVec2(0, window.GetSize().y - 25));
				if (ImGui::Button("Create World")) ChangeMenu(MenuMode::WORLD_CREATION);

				ImGui::SameLine();
				if (ImGui::Button("Back"))
					ChangeBack();
				

				//background 
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}

			//loading world creation
			if (menuMode == MenuMode::WORLD_CREATION) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("World Creation", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
					ImGui::SetKeyboardFocusHere(0);
				ImGui::InputText("World Name", newWorldName, IM_ARRAYSIZE(newWorldName));
				if (ImGui::Button("Create") || Keyboard::getKey(Keyboard::Key::Enter)) {
					gameInstance.CreateNewWorld(newWorldName);
					gameInstance.LoadWorld();
					ChangeMenu(MenuMode::GAME);

				}

				ImGui::SetCursorPos(ImVec2(0, window.GetSize().y - 25));
				if (ImGui::Button("Back"))
					ChangeBack();

				//background 
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}

			//loading settings menu
			if (menuMode == MenuMode::SETTINGS) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
				ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
				if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
					if (ImGui::BeginTabItem("Gameplay")) {

						ImGui::SliderInt("Render Distance - (in chunks)", &Settings::RenderDistance, 1, 60); ImGui::SameLine(); HelpMarker("[Ctrl + Click] to input value");

						ImGui::InputFloat("Mouse Speed", &Settings::MouseSensitivity, 0.1);
						ImGui::InputFloat("Mouse Zoom Speed", &Settings::ZoomMouseSensitivity, 0.1);


						ImGui::Checkbox("Inverted Mouse", &Settings::InvertMouse);					


						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Graphics")) {
						if (ImGui::TreeNode("Bloom")) {
							ImGui::Checkbox("Toggle", &Settings::bloomEnable);
							ImGui::SliderFloat("Threshold", &Settings::BloomThreshold, 0.f, 10.f, "%.3f");
							ImGui::SliderFloat("Knee", &Settings::BloomKnee, 0.f, 1.f, "%.3f");

							ImGui::Separator();
							ImGui::TreePop();
						}
						ImGui::Checkbox("Color Blind Mode", &Settings::ColorBlindMode);

						ImGui::SliderInt("Max Bounce Count", &Settings::maxBounceCount, 1, 100, "%d%", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderInt("Rays per pixel", &Settings::raysPerPixel, 1, 100, "%d%", ImGuiSliderFlags_AlwaysClamp);


						const char* FPSItems[] = { "Unlimited", "30", "45", "60", "120", "240", "300" };
						if (ImGui::BeginCombo("FPS Cap", FPSItems[Settings::FPSCap]))
						{
							for (int i = 0; i < std::size(FPSItems); i++)
							{
								const bool is_selected = (Settings::FPSCap == i);
								if (ImGui::Selectable(FPSItems[i], is_selected))
									Settings::FPSCap = i;


								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						const char* toneMapNames[] = { "ACES", "Filmic", "Reinhard", "Uncharted 2", "Uchimura", "Lottes", "Unreal" };
						if (ImGui::BeginCombo("Tonemap function", toneMapNames[Settings::toneMapFunctionID]))
						{
							for (int i = 0; i < std::size(toneMapNames); i++)
							{
								const bool is_selected = (Settings::toneMapFunctionID == i);
								if (ImGui::Selectable(toneMapNames[i], is_selected))
									Settings::toneMapFunctionID = i;


								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Screen")) {
						const char* items[] = { "Windowed", "Fullscreen" };
						//item_current_idx2         0             1
						if (ImGui::BeginCombo("Window Mode", items[Settings::WindowMode]))
						{
							for (int i = 0; i < std::size(items); i++)
							{
								const bool is_selected = (Settings::WindowMode == i);
								if (ImGui::Selectable(items[i], is_selected))
									Settings::WindowMode = i;

								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
						ImGui::SameLine();
						HelpMarker("Requires restart");

						//ImGui::SetCursorPos(ImVec2(0, 75));
						const char* ResolutionItems[] = { "640 x 360", "960 x 540", "1920 x 1080", "2560 x 1440", "3840 x 2160", };
						//item_current_idx3           0            1             2              3              4 
						const char* combo_preview_value2 = ResolutionItems[Settings::ResolutionIndex];  // Pass in the preview value visible before opening the combo (it could be anything)
						if (ImGui::BeginCombo("Window Resolution", combo_preview_value2))
						{
							for (int i = 0; i < std::size(ResolutionItems); i++)
							{
								bool is_selected2 = (Settings::ResolutionIndex == i);
								if (ImGui::Selectable(ResolutionItems[i], is_selected2))
									Settings::ResolutionIndex = i;


								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected2)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
						ImGui::SameLine();
						HelpMarker("Requires restart");
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Volume")) {
						ImGui::SliderInt("Master Volume", &Settings::v1, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderInt("Weather Volume", &Settings::v2, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderInt("Music Volume", &Settings::v3, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderInt("Mob Volume", &Settings::v4, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}

				//buttons
				static bool respopup = false;
				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 245, 7));
				if (ImGui::Button("Reset to Defaults")) respopup = true;
				

				if (respopup) {
					ImGui::SetNextWindowSize(ImVec2(105, 50));
					ImGui::SetNextWindowPos(ImVec2(window.GetSize().x - 185, 30));
					ImGui::Begin("Picker", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
					ImGui::Text("Are you sure?");
					if (ImGui::Button(" Yes ")) {
						Settings::Reset();
						respopup = false;
					}
					ImGui::SameLine(60);
					if (ImGui::Button(" No ")) 
						respopup = false;
					
					ImGui::End();
				}
				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 110, 7));
				if (ImGui::Button("Back"))
					ChangeBack();

				//background 
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}

			if (menuMode == MenuMode::MULTIPLAYER) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 110, 7));
				if (ImGui::Button("Back")) 
					ChangeBack();				

				ImGui::SetNextWindowSize(ImVec2(200, 50));
				ImGui::SetNextWindowPos(ImVec2(0, window.GetSize().y - 50));
				ImGui::Begin("IP inp", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
				ImGui::InputTextWithHint("  ", "Server IP", serverIP, IM_ARRAYSIZE(serverIP));
				ImGui::SameLine();
				if (ImGui::Button("Join")) {
					//add server in displayables and in servers.yamal (file)
					YAML::Node servers = YAML::LoadFile("servers.yaml");
					std::string key = serverIP;
					servers["server" + std::to_string(servers.size())] = key;
					YAMLUtils::saveFile("servers.yaml", servers);

					memset(serverIP, 0, sizeof(serverIP));
				}
				ImGui::End();

				//background 
				ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::End();
			}
			ImGui::Render();


			if (menuMode == MenuMode::GAME)
				gameInstance.Update(deltaTime);

			// GUI
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			RendererContext::Begin(swapchainImageIndex, window);


			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

			RendererContext::defaultRenderPass.End(cmd);
			cmd.End();

			RendererContext::ExecuteGraphicsCommands();

			VkResult presentationResult = window.Present(swapchainImageIndex, RendererContext::GetRenderSemaphore(), RendererContext::GetPresentQueue());			


			RendererContext::renderFence.Wait();
			RendererContext::renderFence.Reset();

			cmd.Reset();

			if (presentationResult == VK_ERROR_OUT_OF_DATE_KHR || presentationResult == VK_SUBOPTIMAL_KHR) {
				VulkanContext::GetDevice().WaitIdle();
				WC_INFO("Resize 2");
				Resize();
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() {
			VulkanContext::GetDevice().WaitIdle();

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			gameInstance.Destroy(); 
			m_AssetManager.Destroy();

			UploadContext::Destroy();

			descriptorAllocator.Destroy();

			window.DestoySwapchain();
			RendererContext::Destroy();
			window.Destroy();

			DestroyAudioEngine();
			VulkanContext::Destroy();
		}
		//----------------------------------------------------------------------------------------------------------------------
	public:
		Application() = default;

		void Start() {
			OnCreate();

			while (IsEngineOK()) {
				// Input handler
				OnInput();
				// Game Updates
				OnUpdate();
			}

			OnDelete();
		}
	};
}
