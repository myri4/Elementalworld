#pragma once

#include "VulkanContext.h"
#include "Commands.h"
#include "Renderpass.h"

namespace RendererContext {
	constexpr uint32_t FRAME_OVERLAP = 2;

	namespace {
		VkSwapchainKHR swapchain; // from other articles

		// image format expected by the windowing system
		VkFormat swapchainImageFormat;

		//array of images from the swapchain
		std::vector<VkImage> swapchainImages;

		//array of image-views from the swapchain
		std::vector<VkImageView> swapchainImageViews;

		std::vector<wc::Framebuffer> framebuffers;

		wc::RenderPass defaultRenderPass;

		wc::DepthBuffer depthBuffer;

		
		wc::Semaphore presentSemaphore[FRAME_OVERLAP], renderSemaphore[FRAME_OVERLAP];
		wc::Fence renderFence[FRAME_OVERLAP];

		wc::CommandBuffer mainCommandBuffer[FRAME_OVERLAP];
		

		wc::CommandPool commandPool;
		uint32_t frameIndex = 0;
	}

	wc::Queue graphicsQueue;
	wc::Queue computeQueue;
	wc::Queue transferQueue;

	void CreateQueues(const vkb::Device& device) {
		graphicsQueue.GetIndex(vkb::QueueType::graphics, device);
		computeQueue.GetIndex(vkb::QueueType::compute, device);
		transferQueue.GetIndex(vkb::QueueType::transfer, device);
	}

	void CreateSwapchain(const wc::Window& window) {
		vkb::SwapchainBuilder swapchainBuilder{ VulkanContext::GetPhysicalDevice(), VulkanContext::GetDevice(), VulkanContext::GetSurface()};

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			//use vsync present mode
			.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
			.set_desired_extent(window.GetExtent().width, window.GetExtent().height)
			.build()
			.value();

		//store swapchain and its related images
		swapchain = vkbSwapchain.swapchain;
		swapchainImages = vkbSwapchain.get_images().value();
		swapchainImageViews = vkbSwapchain.get_image_views().value();

		swapchainImageFormat = vkbSwapchain.image_format;



		depthBuffer.Create(window.GetExtent());


		VkAttachmentDescription color_attachment = {};
		color_attachment.format = swapchainImageFormat;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth_attachment = {};
		// Depth attachment
		depth_attachment.flags = 0;
		depth_attachment.format = depthBuffer.GetFormat();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref = {};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//we are going to create 1 subpass, which is the minimum you can do
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		//1 dependency, which is from "outside" into the subpass. And we can read or write color
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency depth_dependency = {};
		depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depth_dependency.dstSubpass = 0;
		depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.srcAccessMask = 0;
		depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };
		VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

		VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		render_pass_info.attachmentCount = ARRAYSIZE(attachments);
		render_pass_info.pAttachments = attachments;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = ARRAYSIZE(dependencies);
		render_pass_info.pDependencies = dependencies;

		defaultRenderPass.Create(render_pass_info);

		VkFramebufferCreateInfo fb_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };

		fb_info.renderPass = defaultRenderPass;
		fb_info.attachmentCount = 1;
		fb_info.width = window.GetExtent().width;
		fb_info.height = window.GetExtent().height;
		fb_info.layers = 1;

		const uint32_t swapchain_imagecount = swapchainImages.size();
		framebuffers = std::vector<wc::Framebuffer>(swapchain_imagecount);

		for (int i = 0; i < swapchain_imagecount; i++) {

			VkImageView attachments[2];
			attachments[0] = swapchainImageViews[i];
			attachments[1] = depthBuffer.GetImageView();

			fb_info.pAttachments = attachments;
			fb_info.attachmentCount = ARRAYSIZE(attachments);

			framebuffers[i].Create(fb_info);
		}
	}

	void CreateCommands() {
		//create a command pool for commands submitted to the graphics queue.
		//we also want the pool to allow for resetting of individual command buffers
		commandPool.Create(graphicsQueue.GetFamily());
		for (int i = 0; i < FRAME_OVERLAP; i++) {

			//allocate the default command buffer that we will use for rendering
			commandPool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, mainCommandBuffer[i]);

			//create syncronization structures
			//one fence to control when the gpu has finished rendering the frame,
			//and 2 semaphores to syncronize rendering with swapchain
			//we want the fence to start signalled so we can wait on it on the first frame
			renderFence[i].Create(VK_FENCE_CREATE_SIGNALED_BIT);

			presentSemaphore[i].Create();
			renderSemaphore[i].Create();

		}
	}

	void DestroySwapchain() {
		vkDestroySwapchainKHR(VulkanContext::GetDevice(), swapchain, nullptr);

		for (int i = 0; i < framebuffers.size(); i++)
			framebuffers[i].Destroy();

		//destroy swapchain resources
		for (int i = 0; i < swapchainImageViews.size(); i++)
			vkDestroyImageView(VulkanContext::GetDevice(), swapchainImageViews[i], nullptr);

		defaultRenderPass.Destroy();
		depthBuffer.Destroy();
	}

	void Destroy() {
		commandPool.Destroy();
		DestroySwapchain();
		for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
			renderFence[i].Destroy();
			renderSemaphore[i].Destroy();
			presentSemaphore[i].Destroy();
		}
	}

	void Reset() {
		renderFence[frameIndex].Wait();
		renderFence[frameIndex].Reset();

		mainCommandBuffer[frameIndex].Reset();
	}

	void RecreateSwapchain(const wc::Window& window) {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		VulkanContext::GetDevice().waitIdle();

		DestroySwapchain();

		CreateSwapchain(window);
	}

	uint32_t AcquireNextImageKHR(const uint32_t timeout = 1000000000) {
		uint32_t swapchainImageIndex = 0;
		vkAcquireNextImageKHR(VulkanContext::GetDevice(), swapchain, timeout, presentSemaphore[frameIndex], nullptr, &swapchainImageIndex);
		return swapchainImageIndex;
	}

	void Begin(const uint32_t& swapchainImageIndex, const wc::Window& window) {
		VkRenderPassBeginInfo rpInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };

		rpInfo.renderPass = defaultRenderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = window.GetExtent();
		rpInfo.framebuffer = framebuffers[swapchainImageIndex];
		//connect clear values
		VkClearValue clearValues[2];

		VkClearValue& clearValue = clearValues[0];
		clearValue.color = { { 0.0f, 0.0f, 0.f, 1.0f } };

		VkClearValue& depthClear = clearValues[1];
		depthClear.depthStencil.depth = 1.f;

		rpInfo.clearValueCount = ARRAYSIZE(clearValues);
		rpInfo.pClearValues = clearValues;


		defaultRenderPass.Begin(mainCommandBuffer[frameIndex], rpInfo);
	}

	void End() {
		defaultRenderPass.End(mainCommandBuffer[frameIndex]);
	}

	void ExecuteGraphicsCommands() {
		//prepare the submission to the queue. 
		//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
		//we will signal the _renderSemaphore, to signal that rendering has finished

		VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = mainCommandBuffer[frameIndex].GetPointer();

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = presentSemaphore[frameIndex].GetPointer();

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = renderSemaphore[frameIndex].GetPointer();

		//submit command buffer to the queue and execute it.
		// renderFence will now block until the graphic commands finish execution
		graphicsQueue.Submit(submit, renderFence[frameIndex]);
	}

	void Present(const uint32_t& swapchainImageIndex) {
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

		presentInfo.pSwapchains = &swapchain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = renderSemaphore[frameIndex].GetPointer();
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &swapchainImageIndex;

		graphicsQueue.PresentKHR(presentInfo);


		//increase the number of frames drawn
		frameIndex++;
		frameIndex = frameIndex % FRAME_OVERLAP;
	}

	wc::CommandBuffer& GetCommandBuffer() { return mainCommandBuffer[frameIndex]; }

	const VkSwapchainKHR& GetSwapchain() { return swapchain; }

	const std::vector<VkImage>& GetSwapchainImages() { return swapchainImages; }
	const std::vector<VkImageView>& GetSwapchainImageViews() { return swapchainImageViews; }
	const std::vector<wc::Framebuffer>& GetFramebuffers() { return framebuffers; }
	VkFormat GetSwapchainImageFormat() { return swapchainImageFormat; }
	const wc::RenderPass& GetRenderPass() { return defaultRenderPass; }
	const wc::DepthBuffer& GetDepthBuffer() { return depthBuffer; }
	const uint32_t& GetFrameIndex() { return frameIndex; }

}