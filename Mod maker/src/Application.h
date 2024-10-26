#pragma once
#include <wc/Utils/Window.h>
#include <wc/Utils/Time.h>
#include <wc/Audio/AudioEngine.h>
#include <wc/vk/RendererContext.h>
#include <filesystem>
#include "Project/Project.h"

namespace wc {

	class Application {
	private:
		Window window;
		Project m_CurrentProject;


		bool simpleItem = false;
		char m_ProjectName[128] = "";
		char m_ProjectLocation[128] = ""; // TODO: default this
		std::filesystem::path pth_respack = "resourcepacks";
		std::filesystem::path pth_scripts = "scripts";
		std::filesystem::path pth_curentPath = "";
		std::filesystem::path pth_preview = "";
		bool m_IsCreateMenuOpen = false;
		bool m_CreateDefaultFolders = true;
		Texture preview;
		Texture FileIcon;
		Texture DirectoryIcon;
		Clock deltaTimer;
		float deltaTime = 0.f;
		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() {
			window.poolEvents();

			if (window.hasFocus()) {

			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() {
			VulkanContext::Create();
			CreateAudioEngine();



			WindowCreateInfo windowInfo;
			windowInfo.width = 1280;
			windowInfo.height = 720;
			windowInfo.appName = "Mod maker"; // maybe should be called toolbox or smth
			//windowInfo.startFullscreen = true;
			windowInfo.startMaximized = false;
			window.Create(windowInfo);

			RendererContext::CreateDefaultRenderPass(window);

			UploadContext::Init();
			RendererContext::CreateCommands();

			descriptorAllocator.Create();

			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

			//clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();

			FileIcon.Load("assets/icons/FileIcon.png");
			DirectoryIcon.Load("assets/icons/DirectoryIcon.png");
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

			if (m_IsCreateMenuOpen) {
				ImGui::SetNextWindowSize(ImVec2(800, 400));
				ImGui::SetNextWindowPos(ImVec2((window.GetSize().x - 800) / 2, (window.GetSize().y - 400) / 2));
				ImGui::SetNextWindowFocus();
				ImGui::Begin("New Project", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove);
				ImGui::PushID(1);
				ImGui::Text("Project name");
				ImGui::InputText("", m_ProjectName, IM_ARRAYSIZE(m_ProjectName));
				ImGui::PopID();
				ImGui::Text("Location");
				ImGui::InputText("", m_ProjectLocation, IM_ARRAYSIZE(m_ProjectLocation));

				ImGui::Checkbox("Create default folders", &m_CreateDefaultFolders);

				std::string projectLocation = std::string(m_ProjectLocation) + '/' + std::string(m_ProjectName);

				ImGui::Text(("Project will be created in " + projectLocation + '/').c_str());
				if (ImGui::Button("Create")) {
					m_IsCreateMenuOpen = false;
					std::filesystem::create_directory(projectLocation);

					if (m_CreateDefaultFolders) {
						std::string assetPath = projectLocation + "/assets";
						std::string scriptPath = projectLocation + "/scripts";
						std::filesystem::create_directory(assetPath);
						std::filesystem::create_directory(assetPath + "/textures");
						std::filesystem::create_directory(assetPath + "/models");
						std::filesystem::create_directory(assetPath + "/sounds");
						std::filesystem::create_directory(scriptPath);
						std::filesystem::create_directory(scriptPath + "/blocks");
						std::filesystem::create_directory(scriptPath + "/items");
					}
				}
				if (ImGui::Button("Cancel")) {
					m_IsCreateMenuOpen = false;
					memset(m_ProjectName, 0, sizeof(m_ProjectName));
					memset(m_ProjectLocation, 0, sizeof(m_ProjectLocation));
				}
				ImGui::End();
			}

			ImGui::SetNextWindowSize(ImVec2(window.GetSize().x / 2, window.GetSize().y - 19));
			ImGui::SetNextWindowPos(ImVec2(0, 19));
			ImGui::Begin("Workspace ", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::BeginMenu("New")) {
						if (ImGui::MenuItem("Project", "Ctrl+Shift+N")) { m_IsCreateMenuOpen = true; ImGui::EndMenu(); }
						if (ImGui::MenuItem("File", "Ctrl+N")) { ImGui::EndMenu(); }
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Open")) {
						if (ImGui::MenuItem("Project", "Ctrl+Shift+O")) { ImGui::EndMenu(); }
						if (ImGui::MenuItem("File", "Ctrl+O")) { ImGui::EndMenu(); }
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Save Current Project", "Ctrl+S")) { ImGui::EndMenu(); }
					
					if (ImGui::MenuItem("Export Project", "Ctrl+Sift+S")) { ImGui::EndMenu(); }
					
				

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Options")) {
					if (ImGui::BeginMenu("View")) {
						if (ImGui::MenuItem("Simple Items", NULL, &simpleItem)) {}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
			ImGui::Text("*Empty*");
			ImGui::End();

			ImGui::SetNextWindowSize(ImVec2(window.GetSize().x / 2, window.GetSize().y / 2 - 19));
			ImGui::SetNextWindowPos(ImVec2(window.GetSize().x / 2, 19));
			ImGui::Begin("Preview", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking);
			if (pth_preview != "")ImGui::Image(preview, ImVec2(200, 200));
			ImGui::End();

			ImGui::SetNextWindowSize(ImVec2(window.GetSize().x / 2, window.GetSize().y / 2));
			ImGui::SetNextWindowPos(ImVec2(window.GetSize().x / 2, window.GetSize().y / 2));
			ImGui::Begin("Content Browser", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
			//ImGui::Text(std::format("{0} /", pth_curentPath).c_str());


			if (pth_curentPath == "") {
				ImGui::Image(DirectoryIcon, ImVec2(20, 20)); ImGui::SameLine();
				if (ImGui::Button("resourcepack")) {
					pth_curentPath = pth_respack;
				}

				ImGui::Image(DirectoryIcon, ImVec2(20, 20)); ImGui::SameLine();
				if (ImGui::Button("scripts")) {
					pth_curentPath = pth_scripts;
				}
			}
			else if (pth_curentPath != "") {
				for (auto& p : std::filesystem::directory_iterator(pth_curentPath)) {
					if (p.is_directory()) {
						ImGui::Image(DirectoryIcon, ImVec2(20, 20)); ImGui::SameLine();
						if (ImGui::Button(p.path().stem().string().c_str())) {
							pth_curentPath = p.path();
						}
					}
					else if (p.is_regular_file()) {
						ImGui::Image(FileIcon, ImVec2(20, 20)); ImGui::SameLine();
						if (ImGui::Button((p.path().stem().string() + std::filesystem::path(p.path()).extension().string()).c_str())) {
							if (std::filesystem::path(p.path()).extension() == ".png" && ".jpg") {
								pth_preview = p.path().string().c_str();
								preview.Load(pth_preview.string().c_str());
							}
						}
					}
				}
			}

			if (pth_curentPath == pth_respack.parent_path() || pth_curentPath == "") {
				ImGui::SameLine();
				ImGui::SetCursorScreenPos(ImVec2(window.GetSize().x - 50, window.GetSize().y / 2 + 25));
				ImGui::BeginDisabled();
				ImGui::ArrowButton("arrowbtn", ImGuiDir_Left);
				ImGui::EndDisabled();
			}
			else {
				ImGui::SameLine();
				ImGui::SetCursorScreenPos(ImVec2(window.GetSize().x - 50, window.GetSize().y / 2 + 25));
				if (ImGui::ArrowButton("arrowbtn", ImGuiDir_Left)) {
					pth_curentPath = pth_curentPath.parent_path();
				}
				if (ImGui::IsItemHovered()) {
					if (pth_curentPath.parent_path() == "")ImGui::SetTooltip("start");
					else ImGui::SetTooltip(pth_curentPath.parent_path().string().c_str());
				}
			}
			ImGui::End();
			

			ImGui::Render();

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
