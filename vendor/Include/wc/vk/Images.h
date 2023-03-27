#pragma once

#include "VulkanContext.h"
#include "Buffer.h"
#include <stb_image/stb_image.h>
#include <imgui/imgui_impl_vulkan.h>
#include <wc/Utils/DeletionQueue.h>
#include <glm/glm.hpp>

namespace wc {

    glm::ivec2 GetMipSize(int level, glm::ivec2 size)
    {
        while (level != 0)
        {
            size.x /= 2;
            size.y /= 2;
            level--;
        }

        return size;
    }

    int GetMipLevelCount(glm::vec2 size)
    {
        return (int)glm::floor(glm::log2(glm::min(size.x, size.y)));
    }    

    class Image : public RendererObject<VkImage> {
        VmaAllocation allocation = VK_NULL_HANDLE;
    public:
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t width = 0, height = 0;
        uint32_t mipLevels = 1;
        uint32_t layers = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;

        VkResult Create(const VkImageCreateInfo& dimg_info, VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY) {
            VmaAllocationCreateInfo dimg_allocinfo = {};
            dimg_allocinfo.usage = usage;
            width = dimg_info.extent.width;
            height = dimg_info.extent.height;
            mipLevels = dimg_info.mipLevels;
            layers = dimg_info.arrayLayers;
            format = dimg_info.format;
            layout = dimg_info.initialLayout;

            return vmaCreateImage(VulkanContext::GetMemoryAllocator(), &dimg_info, &dimg_allocinfo, &m_RendererID, &allocation, nullptr);
        }

        void Destroy() {
            vmaDestroyImage(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation);
            m_RendererID = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }

        glm::ivec2 GetMipSize(int level) const 
        {
            glm::ivec2 size = { width, height };
            while (level != 0)
            {
                size.x /= 2;
                size.y /= 2;
                level--;
            }

            return size;
        }

        glm::ivec2 GetSize() const { return { width, height }; }
        float GetAspectRatio() const { return (float)width / (float)height; }

        int GetMipLevelCount()
        {
            glm::vec2 textureSize = { width, height };
            return (int)glm::floor(glm::log2(glm::min(textureSize.x, textureSize.y)));
        }

        void insertMemoryBarrier(
            const VkCommandBuffer& cmdbuffer,
            const VkAccessFlags& srcAccessMask,
            const VkAccessFlags& dstAccessMask,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkPipelineStageFlags& srcStageMask,
            const VkPipelineStageFlags& dstStageMask,
            const VkImageSubresourceRange& subresourceRange)
        {
            VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.srcAccessMask = srcAccessMask;
            imageMemoryBarrier.dstAccessMask = dstAccessMask;
            imageMemoryBarrier.oldLayout = oldImageLayout;
            imageMemoryBarrier.newLayout = newImageLayout;
            imageMemoryBarrier.image = m_RendererID;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
        }

        void insertMemoryBarrier(
            const VkCommandBuffer& cmdbuffer,
            const VkImageAspectFlags& aspectMask,
            const VkAccessFlags& srcAccessMask,
            const VkAccessFlags& dstAccessMask,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkPipelineStageFlags& srcStageMask,
            const VkPipelineStageFlags& dstStageMask)
        {
            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = aspectMask;
            subresourceRange.levelCount = mipLevels;
            subresourceRange.layerCount = layers;

            insertMemoryBarrier(cmdbuffer, srcAccessMask,
                dstAccessMask,
                oldImageLayout,
                newImageLayout,
                srcStageMask,
                dstStageMask, subresourceRange);
        }

        void setLayout(
            const VkCommandBuffer& cmdbuffer,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkImageSubresourceRange& subresourceRange,
            const VkPipelineStageFlags& srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            const VkPipelineStageFlags& dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
        {
            // Create an image barrier object
            VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = oldImageLayout;
            imageMemoryBarrier.newLayout = newImageLayout;
            imageMemoryBarrier.image = m_RendererID;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            // Source layouts (old)
            // Source access mask controls actions that have to be finished on the old layout
            // before it will be transitioned to the new layout
            switch (oldImageLayout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                imageMemoryBarrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source 
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
            }

            // Target layouts (new)
            // Destination access mask controls the dependency for the new image layout
            switch (newImageLayout)
            {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (imageMemoryBarrier.srcAccessMask == 0)
                {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
            }

            // Put barrier inside setup command buffer
            vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
        }

        // Fixed sub resource on first mip level and layer
        void setLayout(
            const VkCommandBuffer& cmdbuffer,
            const VkImageAspectFlags& aspectMask,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkPipelineStageFlags& srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            const VkPipelineStageFlags& dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
        {
            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = aspectMask;
            subresourceRange.levelCount = mipLevels;
            subresourceRange.layerCount = layers;
            setLayout(cmdbuffer, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
        }

        void* Map() {
            void* data = nullptr;
            vmaMapMemory(VulkanContext::GetMemoryAllocator(), allocation, &data);
            return data;
        }

        void Unmap() {
            vmaUnmapMemory(VulkanContext::GetMemoryAllocator(), allocation);
        }

        VkSubresourceLayout SubresourceLayout(const VkImageAspectFlagBits& aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) {
            VkImageSubresource subResource{};
            subResource.aspectMask = aspectMask;
            VkSubresourceLayout subResourceLayout;

            vkGetImageSubresourceLayout(VulkanContext::GetDevice(), m_RendererID, &subResource, &subResourceLayout);
            return subResourceLayout;
        }

        /**
        * @brief Returns true if the attachment has a depth component
        */
        bool hasDepth() const
        {
            std::vector<VkFormat> formats =
            {
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            };
            return std::find(formats.begin(), formats.end(), format) != std::end(formats);
        }

        /**
        * @brief Returns true if the attachment has a stencil component
        */
        bool hasStencil() const
        {
            std::vector<VkFormat> formats =
            {
                VK_FORMAT_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            };
            return std::find(formats.begin(), formats.end(), format) != std::end(formats);
        }

        /**
        * @brief Returns true if the attachment is a depth and/or stencil attachment
        */
        bool isDepthStencil() const
        {
            return(hasDepth() || hasStencil());
        }

        void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)m_RendererID, name); }
        void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)m_RendererID, name.c_str()); }
    };

    struct ImageView : public RendererObject<VkImageView> {
        VkResult Create(const VkImageViewCreateInfo& createInfo) { return vkCreateImageView(VulkanContext::GetDevice(), &createInfo, nullptr, &m_RendererID); }

        VkResult Create(const VkFormat& format, const VkImage& image, const VkImageAspectFlags& aspectFlags, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t levelCount = 1, uint32_t layerCount = 1)
        {
            //build a image-view for the depth image to use for rendering
            VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

            info.viewType = viewType;
            info.image = image;
            info.format = format;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = levelCount;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = layerCount;
            info.subresourceRange.aspectMask = aspectFlags;

            return Create(info);
        }

        VkResult Create(const Image& image)
        {
            //build a image-view for the depth image to use for rendering
            VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

            info.viewType = image.layers > 0 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            info.image = image;
            info.format = image.format;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = image.mipLevels;
            info.subresourceRange.baseArrayLayer = 0;
            info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            info.subresourceRange.layerCount = image.layers;
            info.subresourceRange.aspectMask = image.hasDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

            return Create(info);
        }

        void Destroy() {
            vkDestroyImageView(VulkanContext::GetDevice(), m_RendererID, nullptr);
            m_RendererID = VK_NULL_HANDLE;
        }

        void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_RendererID, name); }
        void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_RendererID, name.c_str()); }
    };

    enum class Filter {
        NEAREST = 0,
        LINEAR = 1,
    };

    enum class SamplerMipmapMode {
        NEAREST = 0,
        LINEAR = 1,
    };

    enum SamplerAddressMode {
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        CLAMP_TO_BORDER = 3
    };

    struct SamplerCreateInfo {
        Filter                magFilter = Filter::NEAREST;
        Filter                minFilter = Filter::NEAREST;
        SamplerMipmapMode     mipmapMode = SamplerMipmapMode::NEAREST;
        SamplerAddressMode    addressModeU = SamplerAddressMode::REPEAT;
        SamplerAddressMode    addressModeV = SamplerAddressMode::REPEAT;
        SamplerAddressMode    addressModeW = SamplerAddressMode::REPEAT;
        float                 mipLodBias = 0.f;
        bool                  anisotropyEnable = false;
        float                 maxAnisotropy = 1.f;
        float                 minLod = 0.f;
        float                 maxLod = 1.f;
    };

    struct Sampler : public RendererObject<VkSampler> {

        Sampler() = default;
        Sampler(const VkSampler& sampler) { m_RendererID = sampler; }

        VkResult Create(const VkSamplerCreateInfo& create_info) {
            return vkCreateSampler(VulkanContext::GetDevice(), &create_info, nullptr, &m_RendererID);
        }

        VkResult Create(const SamplerCreateInfo& info) {
            VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

            createInfo.magFilter = (VkFilter)info.magFilter;
            createInfo.minFilter = (VkFilter)info.minFilter;
            createInfo.mipmapMode = (VkSamplerMipmapMode)info.mipmapMode;
            createInfo.addressModeU = (VkSamplerAddressMode)info.addressModeU;
            createInfo.addressModeV = (VkSamplerAddressMode)info.addressModeV;
            createInfo.addressModeW = (VkSamplerAddressMode)info.addressModeW;
            createInfo.mipLodBias = info.mipLodBias;
            createInfo.anisotropyEnable = info.anisotropyEnable;
            createInfo.maxAnisotropy = info.maxAnisotropy;
            createInfo.compareEnable = false;
            createInfo.compareOp = VK_COMPARE_OP_NEVER;
            createInfo.minLod = info.minLod;
            createInfo.maxLod = info.maxLod;
            createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            createInfo.unnormalizedCoordinates = false;
            return Create(createInfo);
        }

        void Destroy() {
            vkDestroySampler(VulkanContext::GetDevice(), m_RendererID, nullptr);
            m_RendererID = VK_NULL_HANDLE;
        }

        void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t)m_RendererID, name); }
        void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t)m_RendererID, name.c_str()); }
    };

    class DepthBuffer : public RendererObject<VkImage> {
    private:
        VmaAllocation allocation = VK_NULL_HANDLE;
        ImageView depthImageView;
    public:
        VkResult Create(const VkExtent2D& viewportExtent) {
            //depth image size will match the window
            VkExtent3D depthImageExtent = {
                viewportExtent.width,
                viewportExtent.height,
                1
            };

            //the depth image will be an image with the format we selected and Depth Attachment usage flag
            VkImageCreateInfo dimg_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

            dimg_info.imageType = VK_IMAGE_TYPE_2D;

            dimg_info.format = VK_FORMAT_D32_SFLOAT;
            dimg_info.extent = depthImageExtent;

            dimg_info.mipLevels = 1;
            dimg_info.arrayLayers = 1;
            dimg_info.samples = VK_SAMPLE_COUNT_1_BIT;
            dimg_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            dimg_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;


            //for the depth image, we want to allocate it from GPU local memory
            VmaAllocationCreateInfo dimg_allocinfo = {};
            dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            //allocate and create the image
            vmaCreateImage(VulkanContext::GetMemoryAllocator(), &dimg_info, &dimg_allocinfo, &m_RendererID, &allocation, nullptr);


            VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)m_RendererID, "Depth buffer");

            return depthImageView.Create(VK_FORMAT_D32_SFLOAT, m_RendererID, VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        void Destroy() {
            depthImageView.Destroy();
            vmaDestroyImage(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation);
            m_RendererID = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }

        VkFormat GetFormat() const { return VK_FORMAT_D32_SFLOAT; }
        const VkImageView& GetImageView() const { return depthImageView; }
    };

    DeletionQueue TextureDeletionQueue;

    class Texture {
    private:
        Image image;
        ImageView view;
        Sampler sampler;
        VkDescriptorSet imageID = VK_NULL_HANDLE;
    public:

        void Create(const VkImageCreateInfo& imageInfo, const SamplerCreateInfo& samplerInfo) {
            image.Create(imageInfo);

            VkImageViewCreateInfo imageView = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageView.format = imageInfo.format;
            imageView.subresourceRange = {};
            imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageView.subresourceRange.layerCount = 1;
            imageView.subresourceRange.levelCount = 1;
            imageView.image = image;
            view.Create(imageView);
            sampler.Create(samplerInfo);
        }

        void Create(const Image& Image, const Sampler& Sampler, const ImageView& imageView) {
            image = Image;
            sampler = Sampler;
            view = imageView;
            imageID = ImGui_ImplVulkan_AddTexture(sampler, view, image.layout);
        }

        void Load(const std::string& filepath, bool mipMaping = false, bool autoFree = true) {
            int32_t width = 0, height = 0, fnrComponents;
            auto data = stbi_load(filepath.c_str(), &width, &height, &fnrComponents, 4);

            {
                //allocate and create the image
                VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;

                imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

                imageCreateInfo.extent.width = static_cast<uint32_t>(width);
                imageCreateInfo.extent.height = static_cast<uint32_t>(height);
                imageCreateInfo.extent.depth = 1;

                imageCreateInfo.mipLevels = mipMaping ? GetMipLevelCount(glm::vec2(width, height)) : 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

                image.Create(imageCreateInfo);
                image.SetName(filepath);

                view.Create(imageCreateInfo.format, image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, image.mipLevels);
            }

            if (data) {
                SamplerCreateInfo samplerInfo;
                samplerInfo.addressModeU = SamplerAddressMode::REPEAT;
                samplerInfo.addressModeV = SamplerAddressMode::REPEAT;
                samplerInfo.addressModeW = SamplerAddressMode::REPEAT;
                samplerInfo.maxLod = (float)image.mipLevels;

                if (VulkanContext::GetSupportedFeatures().samplerAnisotropy && mipMaping) {
                	samplerInfo.anisotropyEnable = true;
                	samplerInfo.maxAnisotropy = VulkanContext::GetProperties().limits.maxSamplerAnisotropy;
                }

                if (width <= 128 || height <= 128) {
                    samplerInfo.magFilter = Filter::NEAREST;
                    samplerInfo.minFilter = Filter::NEAREST;
                    samplerInfo.mipmapMode = SamplerMipmapMode::NEAREST;
                }
                else {
                    samplerInfo.magFilter = Filter::LINEAR;
                    samplerInfo.minFilter = Filter::LINEAR;
                    samplerInfo.mipmapMode = SamplerMipmapMode::LINEAR;
                }

                sampler.Create(samplerInfo);
                {               
                    VkDeviceSize imageSize = width * height * 4;

                    VkExtent3D imageExtent;
                    imageExtent.width = static_cast<uint32_t>(width);
                    imageExtent.height = static_cast<uint32_t>(height);
                    imageExtent.depth = 1;

                    StagingBuffer stagingBuffer;
                    stagingBuffer.Create(imageSize);
                    stagingBuffer.SetData(data, imageSize);

                    VkImageSubresourceRange subresourceRange = {};
                    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    subresourceRange.levelCount = 1;
                    subresourceRange.layerCount = 1;

                    //transition image to transfer-receiver	
                    UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
                        // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                        image.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                        VkBufferImageCopy copyRegion = {};
                        copyRegion.bufferOffset = 0;
                        copyRegion.bufferRowLength = 0;
                        copyRegion.bufferImageHeight = 0;

                        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        copyRegion.imageSubresource.mipLevel = 0;
                        copyRegion.imageSubresource.baseArrayLayer = 0;
                        copyRegion.imageSubresource.layerCount = 1;
                        copyRegion.imageExtent = imageExtent;

                        //copy the buffer into the image
                        vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                        if (mipMaping) {
                            image.insertMemoryBarrier(
                                cmd,
                                VK_ACCESS_TRANSFER_WRITE_BIT,
                                VK_ACCESS_TRANSFER_READ_BIT,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                subresourceRange);
                   
                            // Copy down mips from n-1 to n
                            for (uint32_t i = 1; i < image.mipLevels; i++)
                            {
                                VkImageBlit imageBlit{};

                                // Source
                                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                imageBlit.srcSubresource.layerCount = 1;
                                imageBlit.srcSubresource.mipLevel = i - 1;
                                imageBlit.srcSubresource.baseArrayLayer = 0;
                                imageBlit.srcOffsets[1].x = int32_t(image.width >> (i - 1));
                                imageBlit.srcOffsets[1].y = int32_t(image.height >> (i - 1));
                                imageBlit.srcOffsets[1].z = 1;

                                // Destination
                                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                imageBlit.dstSubresource.layerCount = 1;
                                imageBlit.dstSubresource.mipLevel = i;
                                imageBlit.dstSubresource.baseArrayLayer = 0;
                                imageBlit.dstOffsets[1].x = int32_t(image.width >> i);
                                imageBlit.dstOffsets[1].y = int32_t(image.height >> i);
                                imageBlit.dstOffsets[1].z = 1;

                                VkImageSubresourceRange mipSubRange = {};
                                mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                mipSubRange.baseMipLevel = i;
                                mipSubRange.baseArrayLayer = 0;
                                mipSubRange.levelCount = 1;
                                mipSubRange.layerCount = 1;

                                // Prepare current mip level as image blit destination
                                image.insertMemoryBarrier(
                                    cmd, 0,
                                    VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    mipSubRange);

                                // Blit from previous level
                                vkCmdBlitImage(
                                    cmd,
                                    image,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    image,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    1,
                                    &imageBlit,
                                    VK_FILTER_LINEAR);

                                // Prepare current mip level as image blit source for next level
                                image.insertMemoryBarrier(
                                    cmd,
                                    VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_READ_BIT,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    mipSubRange);
                            }

                            // After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
                            subresourceRange.levelCount = image.mipLevels;
                            image.insertMemoryBarrier(
                                cmd,
                                VK_ACCESS_TRANSFER_READ_BIT,
                                VK_ACCESS_SHADER_READ_BIT,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                subresourceRange);
                        }
                        else
                            image.setLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    });

                    stagingBuffer.Destroy();                    
                }

                imageID = ImGui_ImplVulkan_AddTexture(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            else
                WC_ERROR("Could not find file at location {0}", filepath.c_str());

            stbi_image_free(data);
            if (autoFree) TextureDeletionQueue.push_function([=]() { Destroy(); });
        }

        void Load(const void* data, int32_t width = 0, int32_t height = 0, bool mipMaping = false, bool autoFree = true) {
            int32_t fnrComponents = 4;

            {
                //allocate and create the image
                VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;

                imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

                imageCreateInfo.extent.width = static_cast<uint32_t>(width);
                imageCreateInfo.extent.height = static_cast<uint32_t>(height);
                imageCreateInfo.extent.depth = 1;

                imageCreateInfo.mipLevels = mipMaping ? GetMipLevelCount(glm::vec2(width, height)) : 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

                image.Create(imageCreateInfo);
                //image.SetName(filepath);

                view.Create(imageCreateInfo.format, image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, image.mipLevels);
            }

            SamplerCreateInfo samplerInfo;
            samplerInfo.addressModeU = SamplerAddressMode::REPEAT;
            samplerInfo.addressModeV = SamplerAddressMode::REPEAT;
            samplerInfo.addressModeW = SamplerAddressMode::REPEAT;
            samplerInfo.maxLod = (float)image.mipLevels;

            if (VulkanContext::GetSupportedFeatures().samplerAnisotropy && mipMaping) {
                samplerInfo.anisotropyEnable = true;
                samplerInfo.maxAnisotropy = VulkanContext::GetProperties().limits.maxSamplerAnisotropy;
            }

            if (width <= 128 || height <= 128) {
                samplerInfo.magFilter = Filter::NEAREST;
                samplerInfo.minFilter = Filter::NEAREST;
                samplerInfo.mipmapMode = SamplerMipmapMode::NEAREST;
            }
            else {
                samplerInfo.magFilter = Filter::LINEAR;
                samplerInfo.minFilter = Filter::LINEAR;
                samplerInfo.mipmapMode = SamplerMipmapMode::LINEAR;
            }

            sampler.Create(samplerInfo);
            {
                VkDeviceSize imageSize = width * height * 4;

                VkExtent3D imageExtent;
                imageExtent.width = static_cast<uint32_t>(width);
                imageExtent.height = static_cast<uint32_t>(height);
                imageExtent.depth = 1;

                StagingBuffer stagingBuffer;
                stagingBuffer.Create(imageSize);
                stagingBuffer.SetData(data, imageSize);

                VkImageSubresourceRange subresourceRange = {};
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.levelCount = 1;
                subresourceRange.layerCount = 1;

                //transition image to transfer-receiver	
                UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
                    // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                    image.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                    VkBufferImageCopy copyRegion = {};
                    copyRegion.bufferOffset = 0;
                    copyRegion.bufferRowLength = 0;
                    copyRegion.bufferImageHeight = 0;

                    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    copyRegion.imageSubresource.mipLevel = 0;
                    copyRegion.imageSubresource.baseArrayLayer = 0;
                    copyRegion.imageSubresource.layerCount = 1;
                    copyRegion.imageExtent = imageExtent;

                    //copy the buffer into the image
                    vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                    if (mipMaping) {
                            image.insertMemoryBarrier(
                                cmd,
                                VK_ACCESS_TRANSFER_WRITE_BIT,
                                VK_ACCESS_TRANSFER_READ_BIT,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                subresourceRange);

                            // Copy down mips from n-1 to n
                            for (uint32_t i = 1; i < image.mipLevels; i++)
                            {
                                VkImageBlit imageBlit{};

                                // Source
                                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                imageBlit.srcSubresource.layerCount = 1;
                                imageBlit.srcSubresource.mipLevel = i - 1;
                                imageBlit.srcSubresource.baseArrayLayer = 0;
                                imageBlit.srcOffsets[1].x = int32_t(image.width >> (i - 1));
                                imageBlit.srcOffsets[1].y = int32_t(image.height >> (i - 1));
                                imageBlit.srcOffsets[1].z = 1;

                                // Destination
                                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                imageBlit.dstSubresource.layerCount = 1;
                                imageBlit.dstSubresource.mipLevel = i;
                                imageBlit.dstSubresource.baseArrayLayer = 0;
                                imageBlit.dstOffsets[1].x = int32_t(image.width >> i);
                                imageBlit.dstOffsets[1].y = int32_t(image.height >> i);
                                imageBlit.dstOffsets[1].z = 1;

                                VkImageSubresourceRange mipSubRange = {};
                                mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                mipSubRange.baseMipLevel = i;
                                mipSubRange.baseArrayLayer = 0;
                                mipSubRange.levelCount = 1;
                                mipSubRange.layerCount = 1;

                                // Prepare current mip level as image blit destination
                                image.insertMemoryBarrier(
                                    cmd, 0,
                                    VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    mipSubRange);

                                // Blit from previous level
                                vkCmdBlitImage(
                                    cmd,
                                    image,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    image,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    1,
                                    &imageBlit,
                                    VK_FILTER_LINEAR);

                                // Prepare current mip level as image blit source for next level
                                image.insertMemoryBarrier(
                                    cmd,
                                    VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_READ_BIT,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    mipSubRange);
                            }

                            // After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
                            subresourceRange.levelCount = image.mipLevels;
                            image.insertMemoryBarrier(
                                cmd,
                                VK_ACCESS_TRANSFER_READ_BIT,
                                VK_ACCESS_SHADER_READ_BIT,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                subresourceRange);
                        }
                    else
                        image.setLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    });

                stagingBuffer.Destroy();
            }

            imageID = ImGui_ImplVulkan_AddTexture(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            

            if (autoFree) TextureDeletionQueue.push_function([=]() { Destroy(); });
        }

        void Destroy() {
            image.Destroy();
            view.Destroy();
            sampler.Destroy();
        }        

        glm::ivec2 GetSize() { return { image.width, image.height }; }

        ImageView GetView() { return view; }
        Sampler GetSampler() { return sampler; }
        Image GetImage() { return image; }

        operator ImTextureID () { return (ImTextureID)imageID; }
    };


    VkDescriptorImageInfo GetDescriptorData(VkSampler sampler, VkImageView view, VkImageLayout imageLayout) {
        VkDescriptorImageInfo imageInfo;
        imageInfo.sampler = sampler;
        imageInfo.imageView = view;
        imageInfo.imageLayout = imageLayout;
        return imageInfo;
    }

    VkDescriptorImageInfo GetDescriptorData(VkSampler sampler, VkImageView view, const Image& image) { return GetDescriptorData(sampler, view, image.layout); }

}