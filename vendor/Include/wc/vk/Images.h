#pragma once

#include "VulkanContext.h"
#include "Buffer.h"
#include <stb_image/stb_image.h>
#include <imgui/imgui_impl_vulkan.h>

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

    int GetMipLevelCount(const glm::vec2& size)
    {
        return (int)glm::floor(glm::log2(glm::min(size.x, size.y)));
    }

    struct ImageView : public RendererObject<VkImageView> {
        VkResult Create(const VkImageViewCreateInfo& createInfo) { return vkCreateImageView(VulkanContext::GetDevice(), &createInfo, nullptr, &m_RendererID); }

        VkResult Create(const VkFormat& format, const VkImage& image, const VkImageAspectFlags& aspectFlags, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D, const uint32_t& levelCount = 1, const uint32_t& layerCount = 1)
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

        void Destroy() { vkDestroyImageView(VulkanContext::GetDevice(), m_RendererID, nullptr); }

        void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_RendererID, name); }
        void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_RendererID, name.c_str()); }
    };

    class Image : public RendererObject<VkImage> {
        VmaAllocation allocation;
    public:
        VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        uint32_t width = 0, height = 0;
        uint32_t mipLevels = 1;

        VkResult Create(const VkImageCreateInfo& dimg_info) {
            VmaAllocationCreateInfo dimg_allocinfo = {};
            dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            return vmaCreateImage(VulkanContext::GetMemoryAllocator(), &dimg_info, &dimg_allocinfo, &m_RendererID, &allocation, nullptr);
        }

        void Destroy() {
            vmaDestroyImage(VulkanContext::GetMemoryAllocator(), m_RendererID, allocation);
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
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.layerCount = 1;
            setLayout(cmdbuffer, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
        }

        void insertImageMemoryBarrier(
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

        void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)m_RendererID, name); }
        void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)m_RendererID, name.c_str()); }
    };

    struct Sampler : public RendererObject<VkSampler> {

        VkResult Create(const VkSamplerCreateInfo& create_info) {
            return vkCreateSampler(VulkanContext::GetDevice(), &create_info, nullptr, &m_RendererID);
        }

        void Destroy() {
            vkDestroySampler(VulkanContext::GetDevice(), m_RendererID, nullptr);
        }

        void SetName(const char* name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t)m_RendererID, name); }
        void SetName(const std::string& name) { VulkanContext::SetObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t)m_RendererID, name.c_str()); }
    };

    class Texture {
        Image image;
        ImageView view;
        Sampler sampler;
    public:

        void Create(const glm::ivec2& size, const VkFormat& image_format = VK_FORMAT_R8G8B8A8_SRGB, const uint32_t& image_channels = 4, const bool& mipmapping = false, const uint32_t& flags = 0) {
            VkDeviceSize imageSize = size.x * size.y * image_channels;                     

            image.width = size.x;
            image.height = size.y;
            if (mipmapping) image.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;

            //allocate and create the image
            VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

            info.imageType = VK_IMAGE_TYPE_2D;

            info.format = image_format;

            info.extent.width = static_cast<uint32_t>(size.x);
            info.extent.height = static_cast<uint32_t>(size.y);
            info.extent.depth = 1;

            info.mipLevels = image.mipLevels;
            info.arrayLayers = 1;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | flags;

            image.Create(info);

            view.Create(image_format, image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, image.mipLevels);
        }

        void Create(const Image& img, const ImageView& imgView) {
            image = img;
            view = imgView;
        }

        void SetData(const glm::ivec2& size, const void* data) {
            VkDeviceSize imageSize = size.x * size.y * 4;

            VkExtent3D imageExtent;
            imageExtent.width = static_cast<uint32_t>(size.x);
            imageExtent.height = static_cast<uint32_t>(size.y);
            imageExtent.depth = 1;

            StagingBuffer stagingBuffer;
            stagingBuffer.Create(imageSize);
            stagingBuffer.SetData(data, imageSize);

            //transition image to transfer-receiver	
            UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
                //barrier the image into the transfer-receive layout
                image.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                VkBufferImageCopy copyRegion = {};
                copyRegion.bufferOffset = 0;
                copyRegion.bufferRowLength = 0;
                copyRegion.bufferImageHeight = 0;

                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.imageSubresource.mipLevel = 0;
                copyRegion.imageSubresource.baseArrayLayer = 0;
                copyRegion.imageSubresource.layerCount = image.mipLevels;
                copyRegion.imageExtent = imageExtent;

                //copy the buffer into the image
                vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                // @TODO: Fix pipelina stage
                image.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                });

            stagingBuffer.Destroy();
        }

        void SetSamplerInfo(const VkSamplerCreateInfo& samplerInfo) {
            sampler.Create(samplerInfo);
        }

        void ResetSamplerInfo() {
            sampler.Destroy();
        }

        VkDescriptorImageInfo GetDescriptorData() const {
            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = sampler;
            imageInfo.imageView = view;
            imageInfo.imageLayout = image.layout;
            return imageInfo;
        }

        void Destroy() {
            image.Destroy();
            view.Destroy();
            sampler.Destroy();
        }

        wc::Image& GetImage() { return image; }
        wc::ImageView& GetImageView() { return view; }
        wc::Sampler& GetSampler() { return sampler; }
    };

    bool loadTexture(const std::string& filepath, Texture& texture, const bool& mipmaps = false) {
        int texWidth, texHeight, texChannels;
        uint8_t* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, 0);

        texture.Create({ texWidth, texHeight }, VK_FORMAT_R8G8B8A8_SRGB, 4, mipmaps);
        texture.SetData({ texWidth, texHeight }, pixels);
        return pixels != nullptr;
    }

    class RenderableTexture : public Texture {
    public:
        uint32_t handle = 0;
    };

    class TextureArray {
        Image image;
        ImageView view;
        Sampler sampler;

        glm::ivec3 size = {32, 32, 40}; // width, height, maxTextures
        uint32_t channels = 4;
    public:

        void Create(const glm::ivec3& Size, const VkFormat& image_format = VK_FORMAT_R8G8B8A8_SRGB, const uint32_t& image_channels = 4) {
            size = Size;
            channels = image_channels;
            VkDeviceSize imageSize = size.x * size.y * size.z * image_channels;

            VkExtent3D imageExtent;
            imageExtent.width = static_cast<uint32_t>(size.x);
            imageExtent.height = static_cast<uint32_t>(size.y);
            imageExtent.depth = 1;

            //allocate and create the image

            VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

            createInfo.imageType = VK_IMAGE_TYPE_2D;

            createInfo.format = image_format;
            createInfo.extent = imageExtent;

            createInfo.mipLevels = 1;
            createInfo.arrayLayers = size.z;
            createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            image.Create(createInfo);

            VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            viewInfo.image = image;
            viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            viewInfo.format = image_format;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = size.z;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            view.Create(viewInfo);
        }

        void SetTextureData(const void* data, const uint32_t& textureID) {
            uint32_t imageSize = size.x * size.y * 4;

            VkExtent3D imageExtent;
            imageExtent.width = static_cast<uint32_t>(size.x);
            imageExtent.height = static_cast<uint32_t>(size.y);
            imageExtent.depth = 1;

            StagingBuffer stagingBuffer;
            stagingBuffer.Create(imageSize);
            stagingBuffer.SetData(data, imageSize);

            //transition image to transfer-receiver	
            UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
                //barrier the image into the transfer-receive layout

                VkImageSubresourceRange subresourceRange = {};
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.baseMipLevel = 0;
                subresourceRange.levelCount = 1;
                subresourceRange.layerCount = size.z;

                //setImageLayout(cmd, 
                //               image, mask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                //    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                image.setLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                VkBufferImageCopy copyRegion = {};
                copyRegion.bufferOffset = 0;
                copyRegion.bufferRowLength = 0;
                copyRegion.bufferImageHeight = 0;

                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.imageSubresource.mipLevel = 0;
                copyRegion.imageSubresource.baseArrayLayer = textureID;
                copyRegion.imageSubresource.layerCount = 1;
                copyRegion.imageExtent = imageExtent;

                //copy the buffer into the image
                vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                // @TODO: Fix pipelina stage
                image.setLayout(
                    cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                });

            stagingBuffer.Destroy();
        }

        void GenerateMipMap() {

        }

        void SetSamplerInfo(const VkSamplerCreateInfo& samplerInfo) {
            sampler.Create(samplerInfo);
        }

        void ResetSamplerInfo() {
            sampler.Destroy();
        }

        VkDescriptorImageInfo GetDescriptorData() {
            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = sampler;
            imageInfo.imageView = view;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            return imageInfo;
        }

        void Destroy() {
            image.Destroy();
            view.Destroy();
            sampler.Destroy();
        }
    };

    class DepthBuffer : public RendererObject<VkImage> {
    private:
        VmaAllocation allocation;
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
        }

        VkFormat GetFormat() const { return VK_FORMAT_D32_SFLOAT; }
        const VkImageView& GetImageView() const { return depthImageView; }
    };

    class ImGuiTexture {
    private:
        Texture texture;
        VkDescriptorSet imageID;// = ImGui_ImplVulkan_AddTexture(sampler, );
    public:
        void Load(const std::string& filepath) {
            int32_t width = 0, height = 0, fnrComponents;
            auto data = stbi_load(filepath.c_str(), &width, &height, &fnrComponents, 0);

            if (data) {
                VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
                sampler.magFilter = VK_FILTER_LINEAR;
                sampler.minFilter = VK_FILTER_LINEAR;
                sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;


                texture.Create(glm::ivec2(width,  height));
                texture.SetData(glm::ivec2(width, height), data);
                texture.SetSamplerInfo(sampler);
                imageID = ImGui_ImplVulkan_AddTexture(texture.GetSampler(), texture.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            else
                WC_ERROR("Could not find file at location {0}", filepath.c_str());

            delete data;
        }


        operator ImTextureID () { return (ImTextureID)imageID; }

        void Destroy() {
            texture.Destroy();
        }
    };
}