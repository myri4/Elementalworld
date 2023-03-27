#pragma once

#include "VulkanContext.h"
#include "Commands.h"

namespace wc {

	enum BufferUsage {
		UNIFORM_BUFFER = 0x00000010,
		STORAGE_BUFFER = 0x00000020,
		INDEX_BUFFER = 0x00000040,
		VERTEX_BUFFER = 0x00000080,
		INDIRECT_BUFFER = 0x00000100,
	};

	class StagingBuffer : public RendererObject<VkBuffer> {
	private:
		VmaAllocation allocation = VK_NULL_HANDLE;
	public:
		VkResult Create(const VkDeviceSize& bufferSize, const VkBufferUsageFlagBits& usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = usage;

			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			return vmaCreateBuffer(VulkanContext::GetMemoryAllocator(), &bufferInfo, &vmaallocInfo,
				&m_RendererID,
				&allocation,
				nullptr);
		}

		void* Map(VkResult& result) {
			void* data;
			result = vmaMapMemory(VulkanContext::GetMemoryAllocator(), allocation, &data);
			return data;
		}

		void* Map() {
			VkResult res;
			return Map(res);
		}

		void Unmap() { vmaUnmapMemory(VulkanContext::GetMemoryAllocator(), allocation); }

		VkResult SetData(const void* data, const VkDeviceSize& size = VK_WHOLE_SIZE) {
			VkResult result;
			memcpy(Map(result), data, size);
			Unmap();
			return result;
		}

		void Destroy() { 
			vmaDestroyBuffer(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation);
			m_RendererID = VK_NULL_HANDLE;
		}

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)m_RendererID, name.c_str()); }
	};

    class Buffer : public RendererObject<VkBuffer> {
    private:
        VmaAllocation allocation = VK_NULL_HANDLE;
    public:
		VkResult Create(const VkDeviceSize& bufferSize, uint32_t usage = wc::STORAGE_BUFFER) {
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			return vmaCreateBuffer(VulkanContext::GetMemoryAllocator(), &bufferInfo, &vmaallocInfo,
				&m_RendererID,
				&allocation,
				nullptr);
        }

		void SetData(const VkBufferCopy& copy, const StagingBuffer& stagingBuffer) {
			UploadContext::immediate_submit([=](VkCommandBuffer cmd) { vkCmdCopyBuffer(cmd, stagingBuffer, m_RendererID, 1, &copy); });
		}

		void SetData(const VkBufferCopy& copy, const void* data) {
			wc::StagingBuffer stb;
			stb.Create(copy.size);
			stb.SetData(data, copy.size);
			SetData(copy, stb);
			stb.Destroy();
		}

		void SetData(const void* data, uint32_t size, uint32_t offset = 0, uint32_t srcOffset = 0) {
			VkBufferCopy copy;
			copy.size = size;
			copy.dstOffset = offset;
			copy.srcOffset = srcOffset;
			SetData(copy, data);
		}

		void SetData(const StagingBuffer& buffer, uint32_t size, uint32_t offset = 0, uint32_t srcOffset = 0) {
			VkBufferCopy copy;
			copy.size = size;
			copy.dstOffset = offset;
			copy.srcOffset = srcOffset;
			SetData(copy, buffer);
		}

		void Destroy() { vmaDestroyBuffer(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation); }

		VkDescriptorBufferInfo GetDescriptorInfo(const VkDeviceSize& size = VK_WHOLE_SIZE, const VkDeviceSize& offset = 0) {
			VkDescriptorBufferInfo info;
			info.buffer = m_RendererID;
			info.offset = offset;
			info.range = size;
			return info;
		}

		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)m_RendererID, name.c_str()); }
    };

	template<typename T>
	class CPUBuffer {
	protected:
		StagingBuffer buffer;
		T* data = nullptr;
	public:
		void Create(const VkDeviceSize& bufferSize) {
			buffer.Create(bufferSize);
		}

		void Map() {
			data = (T*)buffer.Map();
		}

		void Unmap() {
			buffer.Unmap();
		}

		const StagingBuffer& GetBuffer() {
			return buffer;
		}

		void Destroy() {
			buffer.Destroy();
		}

		inline operator T* () { return data; }
		inline operator T* () const { return data; }
	};

	template<typename T>
	class CPUBufferManager : public CPUBuffer<T> {
	private:
		uint32_t counter = 0;
	public:
		void Add(const T& object) {
			this->data[counter] = object;
			counter++;
		}

		void Remove(uint32_t id) {
			counter--;
			this->data[id] = this->data[counter];
		}

		uint32_t GetCounter() { return counter; }
	};
}