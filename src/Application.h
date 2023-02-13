#pragma once
#include "world/World.h"

namespace wc {	
	GameInstance gameInstance;

	char username[256] = "";
	char password[256] = "";
	char serverIP[128] = "";

	class Application {
	private:
		char newWorldName[256];
		std::string joinIp = "some ip idk";
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
		Texture MidBox2;
		Texture SBox;
		Texture XSBox;
		Texture XSBox2;

		Clock deltaTimer;
		float deltaTime = 0.f;

		ImVec2 scaleRes(const ImVec2& position) {
			const ImVec2 malenRes = ImVec2(1920, 1080);
			ImVec2 windowRes = ImVec2(window.GetSize().x, window.GetSize().y);
			ImVec2 finalPos;
			finalPos.x = position.x / malenRes.x * windowRes.x;
			finalPos.y = position.y / malenRes.y * windowRes.y;
			return finalPos;
		}

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() {
			window.poolEvents();

			if (window.hasFocus()) {
				if (menuMode == MenuMode::GAME) gameInstance.OnInput(deltaTime);
				
				if (menuMode == MenuMode::ESCMENU && Keyboard::getKey(Keyboard::Key::Escape)) ChangeMenu(MenuMode::GAME);
					//TODO - doesnt work, doesnt want to change back if in esc menu


				if (Keyboard::getKey(Keyboard::Key::Escape)) ChangeMenu(MenuMode::ESCMENU);
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() {
			VulkanContext::Create();
			CreateAudioEngine();


			WindowCreateInfo windowInfo;
			windowInfo.width = 1280;//1280 x 720 && 1920 x 1080
			windowInfo.height = 720;
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
			//this initializes the core structures of imgui

			ImGui_ImplGlfw_InitForVulkan(window, false);

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

			//clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();					

			gameInstance.Create();

			//@TODO: shorten the lenght
			background.Load(GetAssetPath() + "/textures/misc/screenshot.png");
			TitleSBox.Load( GetAssetPath() + "/textures/misc/TitleSBox.png");
			Box.Load(       GetAssetPath() + "/textures/misc/Box.png");
			LogoLong.Load(  GetAssetPath() + "/textures/misc/LongLogo.png");
			LogoBox.Load(   GetAssetPath() + "/textures/misc/LogoBox.png");
			TitleBox.Load(  GetAssetPath() + "/textures/misc/TitleBox.png");
			Chain1.Load(    GetAssetPath() + "/textures/misc/Chain1.png");
			Chain2.Load(    GetAssetPath() + "/textures/misc/Chain2.png");
			Chain3.Load(    GetAssetPath() + "/textures/misc/Chain3.png");
			LBox.Load(      GetAssetPath() + "/textures/misc/LBox.png");
			MidBox.Load(    GetAssetPath() + "/textures/misc/MidBox.png");
			MidBox2.Load(   GetAssetPath() + "/textures/misc/MidBox.png");
			SBox.Load(      GetAssetPath() + "/textures/misc/SBox.png");
			XSBox.Load(     GetAssetPath() + "/textures/misc/XSBox.png");
			XSBox2.Load(    GetAssetPath() + "/textures/misc/XSBox.png");
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

			if (result == VK_ERROR_OUT_OF_DATE_KHR)
				RendererContext::RecreateDefaultRenderPass(window);
			
			
			//imgui commands
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//loading crosshair and console
			if (menuMode == MenuMode::GAME)
				gameInstance.RenderGUI(deltaTime);			

			//loading escape menu
			if (menuMode == MenuMode::ESCMENU) gameInstance.RenderImGuiEscapeMenu();

			//loading the main menu
			if (menuMode == MenuMode::MAINMENU) {

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
				if (ImGui::ImageButton(MidBox, scaleRes(ImVec2(759, 77)))) ChangeMenu(MenuMode::WORLD_SELECTION);				
				//-Multiplayer
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 600.5)));
				if (ImGui::ImageButton(MidBox2, scaleRes(ImVec2(759, 77)))) ChangeMenu(MenuMode::MULTIPLAYER);
				
				//-Settings
				ImGui::SetCursorPos(scaleRes(ImVec2(581, 842.5)));
				if (ImGui::ImageButton(XSBox, scaleRes(ImVec2(279, 77)))) ChangeMenu(MenuMode::SETTINGS);
				
				//-Quit Game
				ImGui::SetCursorPos(scaleRes(ImVec2(1052, 842.5f)));
				if (ImGui::ImageButton(XSBox2, scaleRes(ImVec2(279, 77)))) window.close();
				
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

				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 110, 7));
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

						ImGui::SliderInt("Render Distance - (in chunks)", &Settings::i1, 1, 60); ImGui::SameLine(); HelpMarker("[Ctrl + Click] to input value");

						ImGui::InputFloat("Mouse Speed", &Settings::MouseSensitivity, 0.1);
						ImGui::InputFloat("Mouse Zoom Speed", &Settings::ZoomMouseSensitivity, 0.1);


						ImGui::Checkbox("Inverted Mouse", &Settings::InvertMouse);					


						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Graphics")) {
						if (ImGui::TreeNode("Bloom")) {
							ImGui::Checkbox("Bloom Toggle", &Settings::bloomEnable);
							ImGui::SliderFloat("BloomThreshold", &Settings::BloomThreshold, 0.f, 100.f, "%.3f");
							ImGui::SliderFloat("BloomKnee", &Settings::BloomKnee, 0.f, 100.f, "%.3f");

							ImGui::Separator();
							ImGui::TreePop();
						}
						ImGui::Checkbox("Colour Blind Mode", &Settings::ColorBlindMode);
						ImGui::Checkbox("Sky", &Settings::sky);


						const char* FPSItems[] = { "unlimited", "30", "45", "60", "120", "240", "300" };
						//item_current_idx          0         1     2     3     4      5      6
						if (ImGui::BeginCombo("FPS Cap", FPSItems[Settings::item_current_idx]))
						{
							for (int i = 0; i < std::size(FPSItems); i++)
							{
								const bool is_selected = (Settings::item_current_idx == i);
								if (ImGui::Selectable(FPSItems[i], is_selected))
									Settings::item_current_idx = i;


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

			static bool islogged = false;
			static bool isplmenopen = false;
			static bool showpas = false;
			static bool require = false;
			if (menuMode == MenuMode::MULTIPLAYER) {
				ImGui::SetNextWindowSize(ImVec2(window.GetSize().x, window.GetSize().y));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 150, 7));
				if (ImGui::Button("[*]")) {
					isplmenopen = !isplmenopen;
				}

				if (isplmenopen) {
					ImGui::SetNextWindowSize(ImVec2(130, 130));
					ImGui::SetNextWindowPos(ImVec2(window.GetSize().x - 185, 30));
					ImGui::Begin("Player Menu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

					if (username[0] != 0 && password[0] != 0) require = true;
					else require = false;
					ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
					if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
						if (ImGui::BeginTabItem("Log-In")) {
							ImGuiInputTextFlags input_flags = 0;
							if (!showpas) input_flags = ImGuiInputTextFlags_Password;
							ImGui::InputTextWithHint("  ", "<username>", username, IM_ARRAYSIZE(username));
							if (showpas) {
								ImGui::BeginDisabled();
								ImGui::InputTextWithHint(" ", "<password>", password, IM_ARRAYSIZE(password), input_flags);
								ImGui::EndDisabled();
							}
							else ImGui::InputTextWithHint(" ", "<password>", password, IM_ARRAYSIZE(password), input_flags);
							ImGui::Checkbox("Show Password", &showpas);
							if (!require) {
								ImGui::BeginDisabled();
								ImGui::Button("Done");
								ImGui::EndDisabled();
								ImGui::SameLine(); HelpMarker("Fill the fields!");
							}
							else if (ImGui::Button("Done")) {
								//proverqva dali ima takuv account v data bazata
								// ako ne - error i kazva da promeni "username or password"
								// log in
								//islogged = true;
								isplmenopen = false;
							}
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Sign-Up")) {
							ImGuiInputTextFlags input_flags = 0;
							if (!showpas) input_flags = ImGuiInputTextFlags_Password;
							ImGui::InputTextWithHint("  ", "<username>", username, IM_ARRAYSIZE(username));
							if (showpas) {
								ImGui::BeginDisabled();
								ImGui::InputTextWithHint(" ", "<password>", password, IM_ARRAYSIZE(password), input_flags);
								ImGui::EndDisabled();
							}
							else ImGui::InputTextWithHint(" ", "<password>", password, IM_ARRAYSIZE(password), input_flags);
							//ImGui::InputTextMultiline();
							ImGui::Checkbox("Show Password", &showpas);
							if (!require) {
								ImGui::BeginDisabled();
								ImGui::Button("Done");
								ImGui::EndDisabled();
								ImGui::SameLine(); HelpMarker("Fill the fields!");
							}
							else if (ImGui::Button("Done")) {
								//suzdava account i save-va acc info
								// log in the new acc
								//islogged = true;
								isplmenopen = false;
							}
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}

					ImGui::End();
				}

				ImGui::SetCursorPos(ImVec2(window.GetSize().x - 110, 7));
				if (ImGui::Button("Back")) {
					ChangeBack();
					isplmenopen = false;
				}

				ImGui::SetNextWindowSize(ImVec2(200, 50));
				ImGui::SetNextWindowPos(ImVec2(0, window.GetSize().y - 50));
				ImGui::Begin("IP inp", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
				ImGui::InputTextWithHint("  ", "<server IP>", serverIP, IM_ARRAYSIZE(serverIP));
				ImGui::SameLine();
				if (ImGui::Button("Add")) {
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

			if (presentationResult == VK_ERROR_OUT_OF_DATE_KHR || presentationResult == VK_SUBOPTIMAL_KHR) 
				RendererContext::RecreateDefaultRenderPass(window);
			


			RendererContext::renderFence.Wait();
			RendererContext::renderFence.Reset();

			cmd.Reset();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() {
			VulkanContext::GetDevice().WaitIdle();

			TextureDeletionQueue.flush();
			ImGui_ImplVulkan_Shutdown();
			gameInstance.Destroy();

			UploadContext::Destroy();

			descriptorAllocator.Destroy();

			RendererContext::Destroy(window);
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
