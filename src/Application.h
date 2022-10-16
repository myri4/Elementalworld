#pragma once
#include "world/World.h"

namespace wc {	

	GameInstance world;

	class Application {
	private:
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
		bool IsEngineOK() { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() {
			window.poolEvents();

			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(deltaTime);

				Mouse::ShowMouse(false); // @TODO: rework		
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnCreate() {
			wc::WindowCreateInfo windowInfo;
			windowInfo.width = 1280;
			windowInfo.height = 720;
			windowInfo.appName = "Elementalworld";
			window.Create(windowInfo);
			VulkanContext::Create();
			window.CreateSwapchain(VulkanContext::GetPhysicalDevice(), VulkanContext::GetDevice(), windowInfo.width, windowInfo.height);
			RendererContext::Create(window);

			wc::UploadContext::Init();
			RendererContext::CreateCommands();

			auto size = window.GetSize();
			render_interface.windowSize = size;

			render_interface.Create(RendererContext::GetRenderPass());

			world.CreateScreen();

			world.Create(size);


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
			
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.IniFilename = nullptr;
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
			//this initializes the core structures of imgui

			//this initializes imgui for SDL
			ImGui_ImplGlfw_InitForVulkan(window, false);

			//this initializes imgui for Vulkan
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanContext::GetInstance();
			init_info.PhysicalDevice = VulkanContext::GetPhysicalDevice();
			init_info.Device = VulkanContext::GetDevice();
			init_info.Queue = RendererContext::GetGraphicsQueue();
			init_info.DescriptorPool = imguiPool;
			init_info.MinImageCount = 3;
			init_info.ImageCount = 3;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			ImGui_ImplVulkan_Init(&init_info, RendererContext::GetRenderPass()); 

			UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
				ImGui_ImplVulkan_CreateFontsTexture(cmd);
				});

			//clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();

			mode = 0;
			world.Join("25.32.4.119", "321");
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnUpdate() {
			deltaTime = deltaTimer.restart();
			wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;

			uint32_t swapchainImageIndex = RendererContext::AcquireNextImageKHR(window);
			
			//imgui commands
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::ShowMetricsWindow();
			ImGui::Render();


			if (mode == MenuMode::GAME)
				world.Update(deltaTime);

			// GUI
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			RendererContext::Begin(swapchainImageIndex, window);

			if (mode == MenuMode::GAME)
				world.RenderGUI();

			render_interface.Flush();

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

			RendererContext::defaultRenderPass.End(cmd);
			cmd.End();

			RendererContext::ExecuteGraphicsCommands();

			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			window.Present(swapchainImageIndex, RendererContext::GetRenderSemaphore(), RendererContext::GetPresentQueue());

			RendererContext::renderFence.Wait();
			RendererContext::renderFence.Reset();

			cmd.Reset();
		}
		//----------------------------------------------------------------------------------------------------------------------
		void OnDelete() {
			vkDeviceWaitIdle(VulkanContext::GetDevice());

			imguiPool.Destroy();
			ImGui_ImplVulkan_Shutdown();
			world.Destroy();
			world.DestroyScreen();


			wc::render_interface.Destroy();

			wc::UploadContext::Destroy();

			wc::descriptorLayoutCache.Destroy();
			wc::descriptorAllocator.Destroy();

			RendererContext::Destroy(window);
			wc::window.Destroy(VulkanContext::GetInstance());

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
