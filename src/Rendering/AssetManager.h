#pragma once
#include <wc/vk/Images.h>
#include <stb_image/stb_image.h>
#include <imgui/imgui_impl_vulkan.h>
#include <filesystem>

namespace wc {

	std::string AssetPackName = "default";

	std::string GetAssetPath() { return "resourcepacks/" + AssetPackName; }
	std::string GetCachedAssetPath() { return "cache/" + AssetPackName; }

    class Texture {
    private:
        Image image;
        ImageView view;
        Sampler sampler;
        VkDescriptorSet imageID = VK_NULL_HANDLE;
    public:

        void Load(const std::string& filepath, bool mipMaping = false) {
            int32_t width = 0, height = 0, fnrComponents;
            auto data = stbi_load(filepath.c_str(), &width, &height, &fnrComponents, 4);

            {
                //allocate and create the image
                ImageCreateInfo imageCreateInfo;

                imageCreateInfo.imageType = ImageType::e2D;

                imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

                imageCreateInfo.width = static_cast<uint32_t>(width);
                imageCreateInfo.height = static_cast<uint32_t>(height);

                imageCreateInfo.mipLevels = mipMaping ? GetMipLevelCount(glm::vec2(width, height)) : 1;
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
        }

        void Load(const void* data, int32_t width = 0, int32_t height = 0, bool mipMaping = false) {

            {
                //allocate and create the image
                ImageCreateInfo imageCreateInfo;

                imageCreateInfo.imageType = ImageType::e2D;

                imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

                imageCreateInfo.width = static_cast<uint32_t>(width);
                imageCreateInfo.height = static_cast<uint32_t>(height);

                imageCreateInfo.mipLevels = mipMaping ? GetMipLevelCount(glm::vec2(width, height)) : 1;
                imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

                image.Create(imageCreateInfo);

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
        }

        void Destroy() {
            image.Destroy();
            view.Destroy();
            sampler.Destroy();
        }

        glm::ivec2 GetSize() { return { image.width, image.height }; }

        ImageView GetView() const { return view; }
        Sampler GetSampler() const { return sampler; }
        Image GetImage() const { return image; }

        operator ImTextureID () const { return (ImTextureID)imageID; }
    };

#undef LoadImage
    class AssetManager {
    private:
        std::unordered_map<std::string, uint32_t> m_Cache; // This becomes obsolete after we transition to pre-baked asset packs    
    public:
        std::vector<wc::Texture> m_Textures;

        void Init() {
            wc::Texture texture;
            uint8_t white[] = { 1, 1, 1, 1 };
            texture.Load(&white, 1, 1, false);
            m_Textures.push_back(texture);
        }

        uint32_t LoadTexture(const std::string& file)
        {
            if (m_Cache.find(file) != m_Cache.end()) return m_Cache[file];  // If this texture exist
            if (std::filesystem::exists(file)) {
                wc::Texture texture;
                texture.Load(file, true);
                m_Textures.push_back(texture);

                m_Cache[file] = m_Textures.size() - 1;
                return m_Textures.size() - 1;
            }

            m_Cache[file] = 0;
            WC_ERROR("Cannot find file at location: {0}", file);
            return 0;
        }
        wc::Texture LoadImage(const std::string& file) {
            return m_Textures[LoadTexture(file)];
        }

        void Destroy() {
            for (auto& texture : m_Textures) texture.Destroy();
        }
    };
}