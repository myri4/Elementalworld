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
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	public:
		VkResult Create(VkDeviceSize bufferSize, const VkBufferUsageFlagBits& usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = usage;

			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			return vmaCreateBuffer(VulkanContext::GetMemoryAllocator(), &bufferInfo, &vmaallocInfo,
				&m_RendererID,
				&m_Allocation,
				nullptr);
		}

		void* Map(VkResult& result) {
			void* data;
			result = vmaMapMemory(VulkanContext::GetMemoryAllocator(), m_Allocation, &data);
			return data;
		}

		void* Map() {
			VkResult res;
			return Map(res);
		}

		void Unmap() { vmaUnmapMemory(VulkanContext::GetMemoryAllocator(), m_Allocation); }

		VkResult SetData(const void* data, VkDeviceSize size = VK_WHOLE_SIZE) {
			VkResult result;
			memcpy(Map(result), data, size);
			Unmap();
			return result;
		}

		void Destroy() { 
			vmaDestroyBuffer(VulkanContext::GetMemoryAllocator(), m_RendererID, m_Allocation);
			m_RendererID = VK_NULL_HANDLE;
		}		
	};

    class Buffer : public RendererObject<VkBuffer> {
    private:
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
    public:
		VkResult Create(VkDeviceSize bufferSize, uint32_t usage = wc::STORAGE_BUFFER) {
			VkBufferCreateInfo bufferInfo = { 
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = bufferSize,
				.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			};

			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			return vmaCreateBuffer(VulkanContext::GetMemoryAllocator(), &bufferInfo, &vmaallocInfo,
				&m_RendererID,
				&m_Allocation,
				nullptr);
        }

		VkDeviceAddress GetDeviceAddress() {
			VkBufferDeviceAddressInfo pInfo = { 
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.buffer = m_RendererID
			};
			return vkGetBufferDeviceAddress(VulkanContext::GetDevice(), &pInfo);
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

		void Destroy() { vmaDestroyBuffer(VulkanContext::GetMemoryAllocator(), m_RendererID, m_Allocation); }

		VkDescriptorBufferInfo GetDescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
			VkDescriptorBufferInfo info;
			info.buffer = m_RendererID;
			info.offset = offset;
			info.range = size;
			return info;
		}
    };

	template<typename T>
	class CPUBuffer {
	protected:
		StagingBuffer m_Buffer;
		T* m_Data = nullptr;
	public:
		void Create(VkDeviceSize bufferSize) {
			m_Buffer.Create(bufferSize);
		}

		void Map() {
			m_Data = (T*)m_Buffer.Map();
		}

		void Unmap() {
			m_Buffer.Unmap();
		}

		const StagingBuffer& GetBuffer() {
			return m_Buffer;
		}

		void Destroy() {
			m_Buffer.Destroy();
		}

		inline operator T* () { return m_Data; }
		inline operator T* () const { return m_Data; }
	};

	template<typename T>
	class CPUBufferManager : public CPUBuffer<T> {
	private:
		uint32_t m_Counter = 0;
	public:
		void Add(const T& object) {
			this->m_Data[m_Counter] = object;
			m_Counter++;
		}

		void Remove(uint32_t id) {
			m_Counter--;
			this->m_Data[id] = this->m_Data[m_Counter];
		}

		uint32_t GetCounter() { return m_Counter; }
	};
}