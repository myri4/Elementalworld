#pragma once
#include "VulkanContext.h"

namespace wc {
	struct Fence : public RendererObject<VkFence> {

		VkResult Create(const VkFenceCreateInfo& fenceCreateInfo) { return vkCreateFence(VulkanContext::GetDevice(), &fenceCreateInfo, VulkanContext::GetAllocator(), &m_RendererID); }

		VkResult Create(const VkFenceCreateFlags& flags = VK_FENCE_CREATE_SIGNALED_BIT) {
			VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

			fenceCreateInfo.flags = flags;

			return Create(fenceCreateInfo);
		}

		VkResult Wait(const uint64_t timeout = UINT64_MAX) { return vkWaitForFences(VulkanContext::GetDevice(), 1, &m_RendererID, true, timeout); }

		VkResult Reset() { return vkResetFences(VulkanContext::GetDevice(), 1, &m_RendererID); }

		VkResult GetStatus() { return vkGetFenceStatus(VulkanContext::GetDevice(), m_RendererID); }

		void Destroy() { 
			vkDestroyFence(VulkanContext::GetDevice(), m_RendererID, VulkanContext::GetAllocator());
			m_RendererID = VK_NULL_HANDLE;
		}


		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct Semaphore : public RendererObject<VkSemaphore>{

		VkResult Create(const VkSemaphoreCreateInfo& semaphoreCreateInfo) {	return vkCreateSemaphore(VulkanContext::GetDevice(), &semaphoreCreateInfo, VulkanContext::GetAllocator(), &m_RendererID); }

		VkResult Create(const VkSemaphoreCreateFlags& flags = 0) {
			VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			semaphoreCreateInfo.flags = flags;

			return Create(semaphoreCreateInfo);
		}

		void Destroy() { 
			vkDestroySemaphore(VulkanContext::GetDevice(), m_RendererID, VulkanContext::GetAllocator());
			m_RendererID = VK_NULL_HANDLE;
		}

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct Event : public RendererObject<VkEvent> {

		VkResult Create(const VkEventCreateInfo& eventCreateInfo) { return vkCreateEvent(VulkanContext::GetDevice(), &eventCreateInfo, VulkanContext::GetAllocator(), &m_RendererID); }

		VkResult Create(const VkEventCreateFlags& flags = 0) {
			VkEventCreateInfo eventCreateInfo = { VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
			eventCreateInfo.flags = flags;

			return Create(eventCreateInfo);
		}

		VkResult GetStatus() { return vkGetEventStatus(VulkanContext::GetDevice(), m_RendererID); }

		VkResult Set() { return vkSetEvent(VulkanContext::GetDevice(), m_RendererID); }

		VkResult Reset() { return vkResetEvent(VulkanContext::GetDevice(), m_RendererID); }

		void Destroy() { 
			vkDestroyEvent(VulkanContext::GetDevice(), m_RendererID, VulkanContext::GetAllocator());
			m_RendererID = VK_NULL_HANDLE;
		}

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_EVENT, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_EVENT, (uint64_t)m_RendererID, name.c_str()); }
	};
}