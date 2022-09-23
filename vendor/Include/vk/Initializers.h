#pragma once

#include "VulkanContext.h"

namespace vk{
    namespace init {
        inline VkImageMemoryBarrier imageMemoryBarrier()
        {
            VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return imageMemoryBarrier;
        }

        inline VkBufferMemoryBarrier bufferMemoryBarrier()
        {
            VkBufferMemoryBarrier bufferMemoryBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
            bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return bufferMemoryBarrier;
        }
    }        

        VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, const uint32_t mipLevels = 1)
        {
            VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

            info.imageType = VK_IMAGE_TYPE_2D;

            info.format = format;
            info.extent = extent;

            info.mipLevels = mipLevels;
            info.arrayLayers = 1;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage = usageFlags;

            return info;
        }

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
        {
            VkPipelineDepthStencilStateCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

            info.depthTestEnable = bDepthTest;
            info.depthWriteEnable = bDepthWrite;
            info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
            info.depthBoundsTestEnable = false;
            info.minDepthBounds = 0.0f; // Optional
            info.maxDepthBounds = 1.0f; // Optional
            info.stencilTestEnable = false;

            return info;
        }
}