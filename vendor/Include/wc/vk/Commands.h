#pragma once

#include "VulkanContext.h"
#include "Synchronization.h"
#include "Pipeline.h"
#include "Descriptors.h"

namespace wc {

	class CommandBuffer : public RendererObject<VkCommandBuffer> {
	public:
		
		CommandBuffer() = default;
		CommandBuffer(const VkCommandBuffer& handle) { m_RendererID = handle; }

		VkResult Begin(const VkCommandBufferBeginInfo& info) const { return vkBeginCommandBuffer(m_RendererID, &info); }

		VkResult Begin(const VkCommandBufferUsageFlags& flags) const {
			VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

			info.pInheritanceInfo = nullptr;
			info.flags = flags;
			
			return vkBeginCommandBuffer(m_RendererID, &info);
		}

		void BindPipeline(const Pipeline& pipeline) const {
			vkCmdBindPipeline(m_RendererID, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}

		void BindPipeline(const ComputePipeline& pipeline) const {
			vkCmdBindPipeline(m_RendererID, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		}

		void BindVertexBuffer(const VkBuffer& vertex_buffer) const {
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(m_RendererID, 0, 1, &vertex_buffer, &offset);
		}

		void BindIndexBuffer(const VkBuffer& index_buffer, const VkIndexType& indexType = VK_INDEX_TYPE_UINT32) const {
			vkCmdBindIndexBuffer(m_RendererID, index_buffer, 0, indexType);
		}

		void BindDescriptorSet(const VkPipelineBindPoint& bindPoint, const uint32_t& binding, const PipelineLayout& layout, const VkDescriptorSet& set, const uint32_t& dynamic_offset = 0) const {
			vkCmdBindDescriptorSets(m_RendererID, bindPoint, layout, binding, 1, &set, dynamic_offset, nullptr);
		}

		void PushConstants(const VkPipelineLayout& pipeline_layout, const VkShaderStageFlags& shader_stage_flags, const uint32_t& size, const void* data, const uint32_t& offset = 0) const {
			vkCmdPushConstants(m_RendererID, pipeline_layout, shader_stage_flags, offset, size, data);
		}

		void Draw(const uint32_t& vertexCount, const uint32_t& instanceCount = 1, const uint32_t& firstVertex = 0, const uint32_t& firstInstance = 0) const {
			vkCmdDraw(m_RendererID, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void DrawIndexed(const uint32_t& indexCount, const uint32_t& instanceCount = 1, const uint32_t& vertexOffset = 0, const uint32_t& firstIndex = 0, const uint32_t& firstInstance = 0) const {
			vkCmdDrawIndexed(m_RendererID, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		void DrawIndirect(const VkBuffer& buffer, const uint32_t& drawCount = 1, const uint32_t& offset = 0) const {
			vkCmdDrawIndirect(m_RendererID, buffer, offset, drawCount, sizeof(VkDrawIndirectCommand));
		}

		void DrawIndexedIndirect(const VkBuffer& buffer, const uint32_t& drawCount = 1, const uint32_t& offset = 0) const {
			vkCmdDrawIndexedIndirect(m_RendererID, buffer, offset, drawCount, sizeof(VkDrawIndexedIndirectCommand));
		}

		void DrawIndexedIndirect(const VkDrawIndexedIndirectCommand& cmd) const {
			vkCmdDrawIndexed(m_RendererID, cmd.indexCount, cmd.instanceCount, cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
		}

		void Dispatch(const glm::ivec3& groupCount) const {
			vkCmdDispatch(m_RendererID, groupCount.x, groupCount.y, groupCount.z);
		}

		void Dispatch(const glm::ivec2& groupCount) const {
			vkCmdDispatch(m_RendererID, groupCount.x, groupCount.y, 1);
		}

		void Dispatch(const glm::vec2& groupCount) const {
			Dispatch(glm::ivec2(groupCount));
		}

		void DispatchIndirect(const VkBuffer& buffer, const VkDeviceSize& offset = 0) const {
			vkCmdDispatchIndirect(m_RendererID, buffer, offset);
		}

		void SetViewport(const VkViewport& viewport) {
			vkCmdSetViewport(m_RendererID, 0, 1, &viewport);
		}

		VkResult End() const { return vkEndCommandBuffer(m_RendererID); }

		VkResult Reset(const VkCommandBufferResetFlags& flags = 0) const { return vkResetCommandBuffer(m_RendererID, flags); }

		void SetName(const char* name)        { VulkanContext::SetObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)m_RendererID, name);	}
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)m_RendererID, name.c_str()); }
	};

	class CommandPool : public RendererObject<VkCommandPool> {
	public:
		VkResult Create(const VkCommandPoolCreateInfo& createInfo) { return vkCreateCommandPool(VulkanContext::GetDevice(), &createInfo, VulkanContext::GetAllocator(), &m_RendererID); }

		VkResult Create(const uint32_t& queueFamilyIndex, const VkCommandPoolCreateFlags& createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
			//create a command pool for commands submitted to the graphics queue.
			VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };

			commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
			commandPoolInfo.flags = createFlags;

			return Create(commandPoolInfo);
		}

		VkResult Allocate(const VkCommandBufferLevel& level, CommandBuffer& commandBuffer) const { // @TODO: add support for allocating multiple command buffers
			VkCommandBufferAllocateInfo cmdAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };

			//commands will be made from our _commandPool
			cmdAllocInfo.commandPool = m_RendererID;
			//we will allocate 1 command buffer
			cmdAllocInfo.commandBufferCount = 1;
			// command level is Primary
			cmdAllocInfo.level = level;


			return vkAllocateCommandBuffers(VulkanContext::GetDevice(), &cmdAllocInfo, commandBuffer.GetPointer());
		}

		VkResult Reset(const VkCommandPoolResetFlags& flags = 0) { return vkResetCommandPool(VulkanContext::GetDevice(), m_RendererID, flags); }

		void Trim(const VkCommandPoolTrimFlags& flags = 0) { vkTrimCommandPool(VulkanContext::GetDevice(), m_RendererID, flags); }

		void Free(const CommandBuffer& cmd) { vkFreeCommandBuffers(VulkanContext::GetDevice(), m_RendererID, 1, cmd.GetPointer()); }

		void Destroy() { 
			vkDestroyCommandPool(VulkanContext::GetDevice(), m_RendererID, VulkanContext::GetAllocator());
			m_RendererID = VK_NULL_HANDLE;
		}


		void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_RendererID, name); }
		void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)m_RendererID, name.c_str()); }
	};

	namespace UploadContext	{
		
		namespace {
			Fence uploadFence;
			CommandPool commandPool;
			CommandBuffer commandBuffer;
		}

		void Init() {
			commandPool.Create(VulkanContext::graphicsQueue.GetFamily(), 0);
			commandPool.Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, commandBuffer);

			uploadFence.Create(0);
		}

		void Destroy() {
			commandPool.Destroy();
			uploadFence.Destroy();
		}

		void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
			CommandBuffer& cmd = commandBuffer;

			//begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			//execute the function
			function(cmd);

			cmd.End();

			VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

			submit.waitSemaphoreCount = 0;
			submit.pWaitSemaphores = nullptr;
			submit.pWaitDstStageMask = nullptr;
			submit.commandBufferCount = 1;
			submit.pCommandBuffers = cmd.GetPointer();
			submit.signalSemaphoreCount = 0;
			submit.pSignalSemaphores = nullptr;

			//submit command buffer to the queue and execute it.
			// _uploadFence will now block until the graphic commands finish execution
			VulkanContext::graphicsQueue.Submit(submit, uploadFence);

			uploadFence.Wait();
			uploadFence.Reset();

			// reset the command buffers inside the command pool
			commandPool.Reset();
		}
	}

}