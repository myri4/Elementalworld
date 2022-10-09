#pragma once

#include "VulkanContext.h"
#include "Commands.h"
#include "Images.h"

namespace wc {
	struct RenderPass : public RendererObject<VkRenderPass> {

		VkResult Create(const VkRenderPassCreateInfo& render_pass_info) { return vkCreateRenderPass(VulkanContext::GetDevice(), &render_pass_info, nullptr, &m_RendererID); }

		void Destroy() { vkDestroyRenderPass(VulkanContext::GetDevice(), m_RendererID, nullptr); }

		void Begin(const CommandBuffer& commandBuffer, const VkRenderPassBeginInfo& render_pass_begin_info, const VkSubpassContents& contents = VK_SUBPASS_CONTENTS_INLINE) const {
			vkCmdBeginRenderPass(commandBuffer, &render_pass_begin_info, contents);
		}

		void End(const CommandBuffer& commandBuffer) const { vkCmdEndRenderPass(commandBuffer); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct Framebuffer : public RendererObject<VkFramebuffer> {

		void Create(const VkFramebufferCreateInfo& framebuffer_create_info) { vkCreateFramebuffer(VulkanContext::GetDevice(), &framebuffer_create_info, nullptr, &m_RendererID); }

		void Destroy() { vkDestroyFramebuffer(VulkanContext::GetDevice(), m_RendererID, nullptr); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)m_RendererID, name.c_str()); }
	};
}