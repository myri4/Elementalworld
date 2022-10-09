#pragma once

#include "VulkanContext.h"
#include "Commands.h"
#include "Renderpass.h"

namespace RendererContext {

	wc::RenderPass defaultRenderPass;

	wc::DepthBuffer depthBuffer;

	
	wc::Semaphore presentSemaphore, renderSemaphore;
	wc::Fence renderFence;

	wc::CommandBuffer mainCommandBuffer;
	

	wc::CommandPool commandPool;
	wc::CommandPool computeCommandPool;

	void CreateSwapchain(wc::Window& window) {

		depthBuffer.Create(window.GetExtent());


		VkAttachmentDescription attachments[2] ;
		VkAttachmentDescription& color_attachment = attachments[0];
		color_attachment.format = window.swapchainImageFormat;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription& depth_attachment = attachments[1];
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

		VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

		VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		render_pass_info.attachmentCount = std::size(attachments);
		render_pass_info.pAttachments = attachments;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = std::size(dependencies);
		render_pass_info.pDependencies = dependencies;

		defaultRenderPass.Create(render_pass_info);

		VkFramebufferCreateInfo fb_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };

		fb_info.renderPass = defaultRenderPass;
		fb_info.attachmentCount = 1;
		fb_info.width = window.GetExtent().width;
		fb_info.height = window.GetExtent().height;
		fb_info.layers = 1;

		const uint32_t swapchain_imagecount = window.swapchainImages.size();
		window.framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

		for (uint32_t i = 0; i < swapchain_imagecount; i++) {

			VkImageView attachments[2];
			attachments[0] = window.swapchainImageViews[i];
			attachments[1] = depthBuffer.GetImageView();

			fb_info.pAttachments = attachments;
			fb_info.attachmentCount = std::size(attachments);

			vkCreateFramebuffer(VulkanContext::GetDevice(), &fb_info, nullptr, &window.framebuffers[i]);
		}
	}

	void CreateCommands() {
		//create a command pool for commands submitted to the graphics queue.
		//we also want the pool to allow for resetting of individual command buffers
		commandPool.Create(VulkanContext::graphicsQueue.GetFamily());
		computeCommandPool.Create(VulkanContext::computeQueue.GetFamily());

		commandPool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, mainCommandBuffer);

		renderFence.Create();
		renderFence.Reset();

		presentSemaphore.Create();
		renderSemaphore.Create();
	}

	void DestroySwapchain(const wc::Window& window) {
		vkDestroySwapchainKHR(VulkanContext::GetDevice(), window.swapchain, nullptr);

		for (auto& framebuffer : window.framebuffers)
			vkDestroyFramebuffer(VulkanContext::GetDevice(), framebuffer, nullptr);

		//destroy swapchain resources
		for (auto& view : window.swapchainImageViews)
			vkDestroyImageView(VulkanContext::GetDevice(), view, nullptr);

		defaultRenderPass.Destroy();
		depthBuffer.Destroy();
	}

	void Destroy(const wc::Window& window) {
		commandPool.Destroy();
		computeCommandPool.Destroy();
		DestroySwapchain(window);
		renderFence.Destroy();
		renderSemaphore.Destroy();
		presentSemaphore.Destroy();
	}

	void RecreateSwapchain(wc::Window& window) {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(VulkanContext::GetDevice());

		DestroySwapchain(window);

		CreateSwapchain(window);
	}

	uint32_t AcquireNextImageKHR(const wc::Window& window, const uint32_t timeout = 1000000000) {
		uint32_t swapchainImageIndex = 0;
		vkAcquireNextImageKHR(VulkanContext::GetDevice(), window.swapchain, timeout, presentSemaphore, nullptr, &swapchainImageIndex);
		return swapchainImageIndex;
	}

	void Begin(const uint32_t& swapchainImageIndex, const wc::Window& window) {
		VkRenderPassBeginInfo rpInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };

		rpInfo.renderPass = defaultRenderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = window.GetExtent();
		rpInfo.framebuffer = window.framebuffers[swapchainImageIndex];
		//connect clear values
		VkClearValue clearValues[2];

		VkClearValue& clearValue = clearValues[0];
		clearValue.color = { { 0.0f, 0.0f, 0.f, 1.0f } };

		VkClearValue& depthClear = clearValues[1];
		depthClear.depthStencil.depth = 1.f;

		rpInfo.clearValueCount = std::size(clearValues);
		rpInfo.pClearValues = clearValues;


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
	const wc::DepthBuffer& GetDepthBuffer() { return depthBuffer; }
	const wc::CommandPool GetComputePool() { return computeCommandPool; }

	const wc::Queue GetGraphicsQueue() { return VulkanContext::graphicsQueue; }
	const wc::Queue GetComputeQueue() { return VulkanContext::computeQueue; }
	const wc::Queue GetPresentQueue() { return VulkanContext::presentQueue; }

}