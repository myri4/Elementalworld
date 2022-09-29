#pragma once
#include "world/World.h"

namespace wc {	

	GameInstance world;

	class Application : public Engine {
	private:
		Clock deltaTimer;
		float deltaTime = 0.f;

		wc::DescriptorPool imguiPool;

		//----------------------------------------------------------------------------------------------------------------------
		bool IsEngineOK() override { return window.isOpen(); }
		//----------------------------------------------------------------------------------------------------------------------
		void OnInput() override {
			bool hasFocus = window.hasFocus();
			if (hasFocus) {
				if (mode == MenuMode::GAME) world.OnInput(deltaTime);

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

			UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
				ImGui_ImplVulkan_CreateFontsTexture(cmd);
				});

			//clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();

			mode = 1;
			world.Join("25.32.4.119", "321");
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


			ImGui::Begin("Settings");
			if (ImGui::Button("Test button")) mode = 0;
			ImGui::End();
			//ImGui::ShowDemoWindow();

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