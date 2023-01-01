#pragma once

#include "VulkanContext.h"
#include <array>

namespace wc {

	using DescriptorSet = VkDescriptorSet;	

	void UpdateDescriptorSets(const uint32_t& descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, const uint32_t& descriptorCopyCount = 0, const VkCopyDescriptorSet* pDescriptorCopies = nullptr) {
		vkUpdateDescriptorSets(VulkanContext::GetDevice(), descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
	}

	struct DescriptorSetLayout : public RendererObject<VkDescriptorSetLayout> {

		DescriptorSetLayout() = default;
		DescriptorSetLayout(const VkDescriptorSetLayout& layout) { m_RendererID = layout; }

		VkResult Create(const VkDescriptorSetLayoutCreateInfo& setinfo) {
			return vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &setinfo, VulkanContext::GetAllocator(), &m_RendererID);
		}

		void Destroy() { 
			vkDestroyDescriptorSetLayout(VulkanContext::GetDevice(), m_RendererID, VulkanContext::GetAllocator());
			m_RendererID = VK_NULL_HANDLE;
		}

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct DescriptorPool : public RendererObject<VkDescriptorPool> {

		DescriptorPool() = default;
		DescriptorPool(const VkDescriptorPool& handle) { m_RendererID = handle; }

		VkResult Create(const VkDescriptorPoolCreateInfo& pool_info) { return vkCreateDescriptorPool(VulkanContext::GetDevice(), &pool_info, VulkanContext::GetAllocator(), &m_RendererID); }

		void Destroy() { vkDestroyDescriptorPool(VulkanContext::GetDevice(), m_RendererID, VulkanContext::GetAllocator()); }

		VkResult Reset(const VkDescriptorPoolResetFlags& flags = 0) { return vkResetDescriptorPool(VulkanContext::GetDevice(), m_RendererID, flags); }

		VkResult Allocate(const VkDescriptorSetLayout* layouts, DescriptorSet& set, const void* pNext = nullptr, const uint32_t& descriptorCount = 1) {
			VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfo.descriptorPool = m_RendererID;
			allocInfo.descriptorSetCount = descriptorCount;
			allocInfo.pSetLayouts = layouts;
			allocInfo.pNext = pNext;


			return vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, &set);
		}

		VkResult Free(const DescriptorSet& set) { return vkFreeDescriptorSets(VulkanContext::GetDevice(), m_RendererID, 1, &set); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_RendererID, name.c_str()); }
	};	

	class DescriptorAllocator {
	public:

		void Create() {
			currentPool = grab_pool();
			usedPools.push_back(currentPool);
		}

		void reset_pools() {
			for (auto& p : usedPools)
				p.Reset();

			freePools = usedPools;
			usedPools.clear();
			currentPool = VK_NULL_HANDLE;
		}

		bool allocate(DescriptorSet& set, const DescriptorSetLayout& layout) {
			if (currentPool == VK_NULL_HANDLE)
			{
				currentPool = grab_pool();
				usedPools.push_back(currentPool);
			}	

			VkResult allocResult = currentPool.Allocate(layout.GetPointer(), set);
			switch (allocResult) {
				case VK_SUCCESS: return true;
				case VK_ERROR_FRAGMENTED_POOL:
				case VK_ERROR_OUT_OF_POOL_MEMORY:
					//allocate a new pool and retry
					currentPool = grab_pool();
					usedPools.push_back(currentPool);

					allocResult = currentPool.Allocate(layout.GetPointer(), set);

					//if it still fails then we have big issues
					if (allocResult == VK_SUCCESS) return true;
			}

			return false;
		}

		bool allocate(DescriptorSet& set, const VkDescriptorSetLayout* layouts, const void* pNext, const uint32_t& descriptorCount) {
			if (currentPool == VK_NULL_HANDLE)
			{
				currentPool = grab_pool();
				usedPools.push_back(currentPool);
			}

			VkResult allocResult = currentPool.Allocate(layouts, set, pNext, descriptorCount);
			switch (allocResult) {
			case VK_SUCCESS: return true;
			case VK_ERROR_FRAGMENTED_POOL:
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				//allocate a new pool and retry
				currentPool = grab_pool();
				usedPools.push_back(currentPool);

				allocResult = currentPool.Allocate(layouts, set, pNext, descriptorCount);

				//if it still fails then we have big issues
				if (allocResult == VK_SUCCESS) return true;
			}

			return false;
		}

		void Destroy() {
			//delete every pool held
			for (auto& p : freePools)			
				p.Destroy();
			
			for (auto& p : usedPools)
				p.Destroy();
		}

		DescriptorPool GetCurrentPool() { return currentPool; }

	private:
		DescriptorPool grab_pool() {
			if (freePools.size() > 0)
			{
				DescriptorPool pool = freePools.back();
				freePools.pop_back();
				return pool;
			}
			else {
				uint32_t count = 1000;

				std::pair<VkDescriptorType, float> dSizes[] = {				
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
					//{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
					//{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
					//{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
					//{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
					//{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f },
				};


				std::array<VkDescriptorPoolSize, std::size(dSizes)> sizes;
				for (int i = 0; i < std::size(dSizes); i++)
					sizes[i] = { dSizes[i].first, uint32_t(dSizes[i].second * count)};

				VkDescriptorPoolCreateInfo pool_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
				pool_info.flags = 0;
				pool_info.maxSets = count;
				pool_info.poolSizeCount = (uint32_t)sizes.size();
				pool_info.pPoolSizes = sizes.data();

				DescriptorPool descriptorPool;
				descriptorPool.Create(pool_info);

				return descriptorPool;
			}
		}

		DescriptorPool currentPool;
		std::vector<DescriptorPool> usedPools;
		std::vector<DescriptorPool> freePools;
	}descriptorAllocator;

	class DescriptorWriter {
	public:
		wc::DescriptorSet dstSet = VK_NULL_HANDLE;
		DescriptorWriter& write_buffer(const uint32_t& binding, const VkDescriptorBufferInfo& bufferInfo, const VkDescriptorType& type) {

			VkWriteDescriptorSet newWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			newWrite.descriptorCount = 1;
			newWrite.descriptorType = type;
			newWrite.pBufferInfo = &bufferInfo;
			newWrite.dstBinding = binding;
			newWrite.dstSet = dstSet;

			writes.push_back(newWrite);
			return *this;
		}

		DescriptorWriter& write_image(const uint32_t& binding, const VkDescriptorImageInfo& imageInfo, const VkDescriptorType& type) {
			VkWriteDescriptorSet newWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			newWrite.descriptorCount = 1;
			newWrite.descriptorType = type;
			newWrite.pImageInfo = &imageInfo;
			newWrite.dstBinding = binding;
			newWrite.dstSet = dstSet;

			writes.push_back(newWrite);
			return *this;
		}

		DescriptorWriter& write_images(const uint32_t& binding, const std::vector<VkDescriptorImageInfo>& imageInfo, const VkDescriptorType& type) {
			VkWriteDescriptorSet newWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			newWrite.descriptorCount = (uint32_t)imageInfo.size();
			newWrite.descriptorType = type;
			newWrite.pImageInfo = imageInfo.data();
			newWrite.dstBinding = binding;
			newWrite.dstSet = dstSet;

			writes.push_back(newWrite);
			return *this;
		}

		void Update() {
			vkUpdateDescriptorSets(VulkanContext::GetDevice(), writes.size(), writes.data(), 0, nullptr);
		}

		std::vector<VkWriteDescriptorSet> writes;
	};
}