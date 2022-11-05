#pragma once

#include "VulkanContext.h"
#include "Commands.h"
#include "Renderpass.h"

namespace RendererContext {

	wc::RenderPass defaultRenderPass;

	std::vector<wc::Framebuffer> framebuffers;
	
	wc::Semaphore presentSemaphore, renderSemaphore;
	wc::Semaphore computeSemaphore;

	wc::Fence renderFence;
	wc::Fence computeFence;

	wc::CommandBuffer mainCommandBuffer;
	wc::CommandBuffer computeCommandBuffer;	

	wc::CommandPool commandPool;
	wc::CommandPool computeCommandPool;

	void CreateDefaultRenderPass(const wc::Window& window) {
		VkAttachmentDescription color_attachment;
		color_attachment.format = window.swapchainImageFormat;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//we are going to create 1 subpass, which is the minimum you can do
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = nullptr;

		//1 dependency, which is from "outside" into the subpass. And we can read or write color
		VkSubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		defaultRenderPass.Create(render_pass_info);

		VkFramebufferCreateInfo fb_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };

		fb_info.renderPass = defaultRenderPass;
		fb_info.attachmentCount = 1;
		fb_info.width = window.GetExtent().width;
		fb_info.height = window.GetExtent().height;
		fb_info.layers = 1;
		fb_info.attachmentCount = 1;

		uint32_t swapchain_imagecount = (uint32_t)window.swapchainImages.size();
		framebuffers = std::vector<wc::Framebuffer>(swapchain_imagecount);

		for (uint32_t i = 0; i < swapchain_imagecount; i++) {
			fb_info.pAttachments = (VkImageView*)&window.swapchainImageViews[i];

			framebuffers[i].Create(fb_info);
		}
	}

	void CreateCommands() {
		//create a command pool for commands submitted to the graphics queue.
		//we also want the pool to allow for resetting of individual command buffers
		commandPool.Create(VulkanContext::graphicsQueue.GetFamily());
		computeCommandPool.Create(VulkanContext::computeQueue.GetFamily());

		commandPool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, mainCommandBuffer);
		computeCommandPool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeCommandBuffer);

		renderFence.Create();
		renderFence.Reset();

		computeFence.Create();
		computeFence.Reset();

		presentSemaphore.Create();
		renderSemaphore.Create();
		computeSemaphore.Create();
	}

	void DestroyDefaultRenderPass(wc::Window& window) {
		window.DestoySwapchain();

		for (auto& framebuffer : framebuffers)
			framebuffer.Destroy();

		defaultRenderPass.Destroy();
	}

	void RecreateDefaultRenderPass(wc::Window& window) {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(VulkanContext::GetDevice());
		DestroyDefaultRenderPass(window);
		window.CreateSwapchain(VulkanContext::GetPhysicalDevice(), VulkanContext::GetDevice(), VulkanContext::GetInstance());
		CreateDefaultRenderPass(window);
	}

	void Destroy(wc::Window& window) {
		DestroyDefaultRenderPass(window);

		commandPool.Destroy();
		computeCommandPool.Destroy();

		renderFence.Destroy();
		renderSemaphore.Destroy();
		presentSemaphore.Destroy();

		computeSemaphore.Destroy();
		computeFence.Destroy();
	}

	VkResult AcquireNextImageKHR(const wc::Window& window, uint32_t& swapchainImageIndex, const uint32_t timeout = 1000000000) {
		return vkAcquireNextImageKHR(VulkanContext::GetDevice(), window.swapchain, timeout, presentSemaphore, nullptr, &swapchainImageIndex);
	}

	void Begin(const uint32_t& swapchainImageIndex, const wc::Window& window) {
		VkRenderPassBeginInfo rpInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };

		rpInfo.renderPass = defaultRenderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = window.GetExtent();
		rpInfo.framebuffer = framebuffers[swapchainImageIndex];

		//connect clear values
		VkClearValue clearValue;
		clearValue.color = { { 0.0f, 0.0f, 0.f, 1.0f } };

		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearValue;


		defaultRenderPass.Begin(mainCommandBuffer, rpInfo);
	}

	void ExecuteGraphicsCommands() {
		VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = mainCommandBuffer.GetPointer();

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = presentSemaphore.GetPointer();

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = renderSemaphore.GetPointer();

		//submit command buffer to the queue and execute it.
		// renderFence will now block until the graphic commands finish execution
		VulkanContext::graphicsQueue.Submit(submit, renderFence);
	}

	wc::Semaphore GetRenderSemaphore() { return renderSemaphore; }

	const wc::RenderPass& GetRenderPass() { return defaultRenderPass; }
	const wc::CommandPool GetComputePool() { return computeCommandPool; }

	const wc::Queue GetGraphicsQueue() { return VulkanContext::graphicsQueue; }
	const wc::Queue GetComputeQueue() { return VulkanContext::computeQueue; }
	const wc::Queue GetPresentQueue() { return /*VulkanContext::presentQueue*/VulkanContext::graphicsQueue; }

}