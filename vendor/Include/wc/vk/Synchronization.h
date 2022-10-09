#pragma once
#include "VulkanContext.h"

namespace wc {
	struct Fence : public RendererObject<VkFence> {

		VkResult Create(const VkFenceCreateInfo& fenceCreateInfo, const VkAllocationCallbacks* pAllocator = nullptr) { return vkCreateFence(VulkanContext::GetDevice(), &fenceCreateInfo, pAllocator, &m_RendererID); }

		VkResult Create(const VkFenceCreateFlags& flags = VK_FENCE_CREATE_SIGNALED_BIT, const VkAllocationCallbacks* pAllocator = nullptr) {
			VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

			fenceCreateInfo.flags = flags;

			return Create(fenceCreateInfo, pAllocator);
		}

		VkResult Wait(const uint64_t timeout = UINT64_MAX) { return vkWaitForFences(VulkanContext::GetDevice(), 1, &m_RendererID, true, timeout); }

		VkResult Reset() { return vkResetFences(VulkanContext::GetDevice(), 1, &m_RendererID); }

		VkResult GetStatus() { return vkGetFenceStatus(VulkanContext::GetDevice(), m_RendererID); }

		void Destroy(const VkAllocationCallbacks* pAllocator = nullptr) { vkDestroyFence(VulkanContext::GetDevice(), m_RendererID, pAllocator); }


		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct Semaphore : public RendererObject<VkSemaphore>{

		VkResult Create(const VkSemaphoreCreateInfo& semaphoreCreateInfo, const VkAllocationCallbacks* pAllocator = nullptr) {	return vkCreateSemaphore(VulkanContext::GetDevice(), &semaphoreCreateInfo, pAllocator, &m_RendererID); }

		VkResult Create(const VkSemaphoreCreateFlags& flags = 0, const VkAllocationCallbacks* pAllocator = nullptr) {
			VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			semaphoreCreateInfo.flags = flags;

			return Create(semaphoreCreateInfo, pAllocator);
		}

		void Destroy(const VkAllocationCallbacks* pAllocator = nullptr) { vkDestroySemaphore(VulkanContext::GetDevice(), m_RendererID, pAllocator); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct Event : public RendererObject<VkEvent> {

		VkResult Create(const VkEventCreateInfo& eventCreateInfo, const VkAllocationCallbacks* pAllocator = nullptr) { return vkCreateEvent(VulkanContext::GetDevice(), &eventCreateInfo, pAllocator, &m_RendererID); }

		VkResult Create(const VkEventCreateFlags& flags = 0, const VkAllocationCallbacks* pAllocator = nullptr) {
			VkEventCreateInfo eventCreateInfo = { VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
			eventCreateInfo.flags = flags;

			return Create(eventCreateInfo, pAllocator);
		}

		VkResult GetStatus() { return vkGetEventStatus(VulkanContext::GetDevice(), m_RendererID); }

		VkResult Set() { return vkSetEvent(VulkanContext::GetDevice(), m_RendererID); }

		VkResult Reset() { return vkResetEvent(VulkanContext::GetDevice(), m_RendererID); }

		void Destroy(const VkAllocationCallbacks* pAllocator = nullptr) { vkDestroyEvent(VulkanContext::GetDevice(), m_RendererID, pAllocator); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_EVENT, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_EVENT, (uint64_t)m_RendererID, name.c_str()); }
	};
}