#pragma once

#include <glad/glad.h>

namespace gl {
	class CommandBuffer {
		//VkCommandBuffer commandBuffer;
	public:

		CommandBuffer() = default;

		//void Allocate(const VkCommandBufferAllocateInfo& cmdAllocInfo) {
		//	vk::Engine::VK_CHECK(vkAllocateCommandBuffers(vk::Engine::device, &cmdAllocInfo, &commandBuffer));
		//}
		//
		//void Reset(const VkCommandBufferResetFlags& flags = 0) {
		//	vk::Engine::VK_CHECK(vkResetCommandBuffer(commandBuffer, flags));
		//}
		//
		//void Begin(const VkCommandBufferBeginInfo& cmdBeginInfo) const {
		//	vk::Engine::VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));
		//}
		//
		//void BeginRenderPass(const VkRenderPassBeginInfo& rpInfo, const VkSubpassContents& contents) const {
		//	vkCmdBeginRenderPass(commandBuffer, &rpInfo, contents);
		//}
		//
		//void EndRenderPass() const {
		//	vkCmdEndRenderPass(commandBuffer);
		//}
		//
		//void End() const {
		//	vk::Engine::VK_CHECK(vkEndCommandBuffer(commandBuffer));
		//}

		//void Draw(const uint32_t& vertexCount, const uint32_t& instanceCount = 1, const uint32_t& firstVertex = 0, const uint32_t& firstInstance = 0) const {
		//	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
		//}
		//
		//void DrawIndexed(const uint32_t& indexCount) const {
		//	glDrawElementsInstancedBaseVertexBaseInstance();
		//}
		//
		//inline operator VkCommandBuffer& () { return commandBuffer; }
		//inline operator const VkCommandBuffer& () const { return commandBuffer; }
	};
}