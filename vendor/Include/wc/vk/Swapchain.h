#pragma once

#include <wc/Utils/Log.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace wc {

	class Swapchain {
	public:

        void Create(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkInstance& instance, VkExtent2D Extent) {
            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };

            SwapChainSupportDetails swapChainSupport;
            {
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupport.capabilities);

                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

                if (formatCount != 0) {
                    swapChainSupport.formats.resize(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapChainSupport.formats.data());
                }

                uint32_t presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

                if (presentModeCount != 0) {
                    swapChainSupport.presentModes.resize(presentModeCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapChainSupport.presentModes.data());
                }
            }


            VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
            {
                for (const auto& availableFormat : swapChainSupport.formats)
                    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                        surfaceFormat = availableFormat;
                        break;
                    }
            }
            VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
            {
                for (const auto& availablePresentMode : swapChainSupport.presentModes)
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                        presentMode = availablePresentMode;
                        break;
                    }
            }
            VkExtent2D extent;
            {
                auto& capabilities = swapChainSupport.capabilities;
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
                    extent = capabilities.currentExtent;
                else {
                    VkExtent2D actualExtent = Extent;

                    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                    extent = actualExtent;
                }
            }

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
                imageCount = swapChainSupport.capabilities.maxImageCount;

            VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            //QueueFamilyIndices indices = findQueueFamilies(physicalDevice/*, window.surface*/);
            //uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value()/*, indices.presentFamily.value()*/ };

            //if (indices.graphicsFamily != indices.presentFamily) {
            //	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            //	createInfo.queueFamilyIndexCount = 2;
            //	createInfo.pQueueFamilyIndices = queueFamilyIndices;
            //}
            //else 
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;


            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
                WC_ERROR("failed to create swap chain!");

            vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
            swapchainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

            swapchainImageFormat = surfaceFormat.format;

            swapchainImageViews.resize(swapchainImages.size());

            for (size_t i = 0; i < swapchainImages.size(); i++) {
                VkImageViewCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.image = swapchainImages[i];
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format = swapchainImageFormat;
                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseMipLevel = 0;
                createInfo.subresourceRange.levelCount = 1;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
                    WC_ERROR("Failed to create image views!");
            }
        }

        void Destroy() {

        }

        uint32_t AquireNextImageID() {

        }

	private:
        VkSwapchainKHR swapchain; // from other articles

        // image format expected by the windowing system
        VkFormat swapchainImageFormat;

        //array of images from the swapchain
        std::vector<VkImage> swapchainImages;

        //array of image-views from the swapchain
        std::vector<VkImageView> swapchainImageViews;

        VkSurfaceKHR surface; // Vulkan window surface	
	};

}