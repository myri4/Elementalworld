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

	struct BufferView : public RendererObject<VkBufferView> {
		VkResult Create(const VkBufferViewCreateInfo& createInfo) {
			return vkCreateBufferView(VulkanContext::GetDevice(), &createInfo, nullptr, &m_RendererID);
		}

		void Destroy() {
			vkDestroyBufferView(VulkanContext::GetDevice(), m_RendererID, nullptr);
		}
	};

	class StagingBuffer : public RendererObject<VkBuffer> {
	private:
		VmaAllocation allocation;
	public:
		VkResult Create(const VkDeviceSize& bufferSize) {
			//allocate vertex buffer
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			//this is the total size, in bytes, of the buffer we are allocating
			bufferInfo.size = bufferSize;
			//this buffer is going to be used as a Vertex Buffer
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			//allocate the buffer
			return vmaCreateBuffer(VulkanContext::GetMemoryAllocator(), &bufferInfo, &vmaallocInfo,
				&m_RendererID,
				&allocation,
				nullptr);
		}

		void* Map(VkResult& result) {
			void* data;
			vmaMapMemory(VulkanContext::GetMemoryAllocator(), allocation, &data);
			return data;
		}

		void* Map() {
			VkResult res;
			return Map(res);
		}

		void Unmap() { vmaUnmapMemory(VulkanContext::GetMemoryAllocator(), allocation); }

		VkResult SetData(const void* data, const VkDeviceSize& size = VK_WHOLE_SIZE) { // @TODO: implement offset parameter
			VkResult result;
			memcpy(Map(result), data, size);
			Unmap();
			return result;
		}

		void Destroy() { vmaDestroyBuffer(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation); }
	};

    class Buffer : public RendererObject<VkBuffer> {
    private:
        VmaAllocation allocation;
    public:
        VkResult Create(const VkDeviceSize& bufferSize, const uint32_t& usage) {
			//allocate vertex buffer
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			//this is the total size, in bytes, of the buffer we are allocating
			bufferInfo.size = bufferSize;
			//this buffer is going to be used as a Vertex Buffer
			bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			//allocate the buffer
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

		void SetData(const void* data, const uint32_t& size, const uint32_t& offset = 0, const uint32_t& srcOffset = 0) {
			VkBufferCopy copy;
			copy.size = size;
			copy.dstOffset = offset;
			copy.srcOffset = srcOffset;
			SetData(copy, data);
		}

		void SetData(const StagingBuffer& buffer, const uint32_t& size, const uint32_t& offset = 0, const uint32_t& srcOffset = 0) {
			VkBufferCopy copy;
			copy.size = size;
			copy.dstOffset = offset;
			copy.srcOffset = srcOffset;
			SetData(copy, buffer);
		}

		void Destroy() { vmaDestroyBuffer(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation); }

		VkDescriptorBufferInfo GetDescriptorInfo(const VkDeviceSize& size = VK_WHOLE_SIZE, const VkDeviceSize& offset = 0) {
			VkDescriptorBufferInfo info;
			//it will be the camera buffer
			info.buffer = m_RendererID;
			//at 0 offset
			info.offset = offset;
			//of the size of a camera data struct
			info.range = size;
			return info;
		}
    };

	template<class T>
	struct CPUBuffer {
	private:
		StagingBuffer buffer;
		T* data;
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
}