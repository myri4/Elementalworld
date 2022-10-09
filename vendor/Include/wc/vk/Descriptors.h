#pragma once

#include "VulkanContext.h"
#include <array>

namespace wc {

	using DescriptorSet = VkDescriptorSet;

	void UpdateDescriptorSets(const uint32_t& descriptorWriteCount,	const VkWriteDescriptorSet* pDescriptorWrites, const uint32_t& descriptorCopyCount = 0,	const VkCopyDescriptorSet* pDescriptorCopies = nullptr) {
		vkUpdateDescriptorSets(VulkanContext::GetDevice(), descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
	}

	struct DescriptorSetLayout : public RendererObject<VkDescriptorSetLayout> {

		VkResult Create(const VkDescriptorSetLayoutCreateInfo& setinfo) {
			return vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &setinfo, nullptr, &m_RendererID);
		}

		void Destroy() { vkDestroyDescriptorSetLayout(VulkanContext::GetDevice(), m_RendererID, nullptr); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)m_RendererID, name.c_str()); }
	};

	struct DescriptorPool : public RendererObject<VkDescriptorPool> {

		DescriptorPool() = default;
		DescriptorPool(const VkDescriptorPool& handle) { m_RendererID = handle; }

		VkResult Create(const VkDescriptorPoolCreateInfo& pool_info) { return vkCreateDescriptorPool(VulkanContext::GetDevice(), &pool_info, nullptr, &m_RendererID); }

		void Destroy() { vkDestroyDescriptorPool(VulkanContext::GetDevice(), m_RendererID, nullptr); }

		VkResult Reset(const VkDescriptorPoolResetFlags& flags = 0) { return vkResetDescriptorPool(VulkanContext::GetDevice(), m_RendererID, flags); }

		VkResult Allocate(const DescriptorSetLayout& layout, DescriptorSet& set) {
			//allocate one descriptor set for each frame
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			//using the pool we just set
			allocInfo.descriptorPool = m_RendererID;
			//only 1 descriptor
			allocInfo.descriptorSetCount = 1;
			//using the global data layout
			allocInfo.pSetLayouts = layout.GetPointer();


			return vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, &set);
		}

		VkResult Free(const DescriptorSet& set) { return vkFreeDescriptorSets(VulkanContext::GetDevice(), m_RendererID, 1, &set); }

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_RendererID, name.c_str()); }
	};	

	class DescriptorAllocator {
	public:

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

			VkResult allocResult = currentPool.Allocate(layout, set);
			switch (allocResult) {
				case VK_SUCCESS: return true;
				case VK_ERROR_FRAGMENTED_POOL:
				case VK_ERROR_OUT_OF_POOL_MEMORY:
					//allocate a new pool and retry
					currentPool = grab_pool();
					usedPools.push_back(currentPool);

					allocResult = currentPool.Allocate(layout, set);

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

				std::array<std::pair<VkDescriptorType, float>, 11> dSizes;

				
				dSizes[0]  = { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f };
				dSizes[1]  = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f };
				dSizes[2]  = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f };
				dSizes[3]  = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f };
				dSizes[4]  = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f };
				dSizes[5]  = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f };
				dSizes[6]  = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f };
				dSizes[7]  = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f };
				dSizes[8]  = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f };
				dSizes[9]  = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f };
				dSizes[10] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f };


				std::array<VkDescriptorPoolSize, 11> sizes;
				for (int i = 0; i < dSizes.size(); i++)
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
	};

	class DescriptorLayoutCache {
	public:
		void Destroy() {
			for (auto& pair : layoutCache) pair.second.Destroy();
		}

		DescriptorSetLayout create_descriptor_layout(const VkDescriptorSetLayoutCreateInfo& info) {
			DescriptorLayoutInfo layoutinfo;
			layoutinfo.bindings.reserve(info.bindingCount);
			bool isSorted = true;
			int32_t lastBinding = -1;
			for (uint32_t i = 0; i < info.bindingCount; i++) {
				layoutinfo.bindings.emplace_back(info.pBindings[i]);

				//check that the bindings are in strict increasing order
				//if (static_cast<int32_t>(info.pBindings[i].binding) > lastBinding)	lastBinding = info.pBindings[i].binding;				
				//else isSorted = false;
			}
			//if (!isSorted)			
			//	std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) { return a.binding < b.binding; });			

			auto it = layoutCache.find(layoutinfo);
			if (it != layoutCache.end()) return (*it).second;			
			else {
				DescriptorSetLayout layout;
				layout.Create(info);

				layoutCache[layoutinfo] = layout;
				return layout;
			}
		}

	private:
		struct DescriptorLayoutInfo {
			//good idea to turn this into a inlined array
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const {
				if (other.bindings.size() != bindings.size())				
					return false;				
				else {
					//compare each of the bindings is the same. Bindings are sorted so they will match
					for (int i = 0; i < bindings.size(); i++) {
						if (other.bindings[i].binding != bindings[i].binding) return false;						
						if (other.bindings[i].descriptorType != bindings[i].descriptorType)	return false;						
						if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) return false;						
						if (other.bindings[i].stageFlags != bindings[i].stageFlags)	return false;						
					}
					return true;
				}
			}

			size_t hash() const {
				using std::size_t;
				using std::hash;

				size_t result = hash<size_t>()(bindings.size());

				for (const VkDescriptorSetLayoutBinding& b : bindings)
				{
					//pack the binding data into a single int64. Not fully correct but its ok
					size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

					//shuffle the packed binding data and xor it with the main hash
					result ^= hash<size_t>()(binding_hash);
				}

				return result;
			}
		};

		struct DescriptorLayoutHash { std::size_t operator()(const DescriptorLayoutInfo& k) const { return k.hash(); } };

		std::unordered_map<DescriptorLayoutInfo, DescriptorSetLayout, DescriptorLayoutHash> layoutCache;
	};

	DescriptorLayoutCache descriptorLayoutCache;
	DescriptorAllocator descriptorAllocator;

	class DescriptorWriter {
	public:
		wc::DescriptorSet dstSet;
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

			newWrite.descriptorCount = imageInfo.size();
			newWrite.descriptorType = type;
			newWrite.pImageInfo = imageInfo.data();
			newWrite.dstBinding = binding;
			newWrite.dstSet = dstSet;

			writes.push_back(newWrite);
			return *this;
		}

		std::vector<VkWriteDescriptorSet> writes;
	private:
	};
}