#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include "Log.h"
#include <imgui/imgui_impl_glfw.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) 	indices.computeFamily = i;
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transferFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

namespace wc {

	double scrollX = 0.f, scrollY = 0.f;
    int mouseButtons[GLFW_MOUSE_BUTTON_LAST];
    int keyButtons[GLFW_KEY_LAST];

    namespace Keyboard {

        enum class Key {
                Unknown = -1,              ///< Unhandled key
                A = GLFW_KEY_A,            ///< The A key
                B = GLFW_KEY_B,            ///< The B key
                C = GLFW_KEY_C,            ///< The C key
                D = GLFW_KEY_D,            ///< The D key
                E = GLFW_KEY_E,            ///< The E key
                F = GLFW_KEY_F,            ///< The F key
                G = GLFW_KEY_G,            ///< The G key
                H = GLFW_KEY_H,            ///< The H key
                I = GLFW_KEY_I,            ///< The I key
                J = GLFW_KEY_J,            ///< The J key
                K = GLFW_KEY_K,            ///< The K key
                L = GLFW_KEY_L,            ///< The L key
                M = GLFW_KEY_M,            ///< The M key
                N = GLFW_KEY_N,            ///< The N key
                O = GLFW_KEY_O,            ///< The O key
                P = GLFW_KEY_P,            ///< The P key
                Q = GLFW_KEY_Q,            ///< The Q key
                R = GLFW_KEY_R,            ///< The R key
                S = GLFW_KEY_S,            ///< The S key
                T = GLFW_KEY_T,            ///< The T key
                U = GLFW_KEY_U,            ///< The U key
                V = GLFW_KEY_V,            ///< The V key
                W = GLFW_KEY_W,            ///< The W key
                X = GLFW_KEY_X,            ///< The X key
                Y = GLFW_KEY_Y,            ///< The Y key
                Z = GLFW_KEY_Z,            ///< The Z key

                Num0 = GLFW_KEY_0,         ///< The 0 key
                Num1 = GLFW_KEY_1,         ///< The 1 key
                Num2 = GLFW_KEY_2,         ///< The 2 key
                Num3 = GLFW_KEY_3,         ///< The 3 key
                Num4 = GLFW_KEY_4,         ///< The 4 key
                Num5 = GLFW_KEY_5,         ///< The 5 key
                Num6 = GLFW_KEY_6,         ///< The 6 key
                Num7 = GLFW_KEY_7,         ///< The 7 key
                Num8 = GLFW_KEY_8,         ///< The 8 key
                Num9 = GLFW_KEY_9,         ///< The 9 key

                Escape = GLFW_KEY_ESCAPE,       ///< The Escape key

                LControl = GLFW_KEY_LEFT_CONTROL,     ///< The left Control key
                LShift = GLFW_KEY_LEFT_SHIFT,       ///< The left Shift key       
                LAlt = GLFW_KEY_LEFT_ALT,         ///< The left Alt key
                LSystem = GLFW_KEY_LEFT_SUPER,      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
                LBracket = GLFW_KEY_LEFT_BRACKET,     ///< The [ key

                RControl = GLFW_KEY_RIGHT_CONTROL,     ///< The right Control key
                RShift = GLFW_KEY_RIGHT_SHIFT,       ///< The right Shift key
                RAlt = GLFW_KEY_RIGHT_ALT,         ///< The right Alt key        
                RSystem = GLFW_KEY_RIGHT_SUPER,      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
                RBracket = GLFW_KEY_RIGHT_BRACKET,     ///< The ] key

                Menu = GLFW_KEY_MENU,         ///< The Menu key
                Semicolon = GLFW_KEY_SEMICOLON,    ///< The ; key        
                Comma = GLFW_KEY_COMMA,        ///< The , key
                Period = GLFW_KEY_PERIOD,       ///< The . key
                Quote = GLFW_KEY_APOSTROPHE,        ///< The ' key
                Slash = GLFW_KEY_SLASH,        ///< The / key
                Backslash = GLFW_KEY_BACKSLASH,    ///< The \ key
                //Tilde = GLFW_KEY_,        ///< The ~ key
                Equal = GLFW_KEY_EQUAL,        ///< The = key
                //Hyphen = GLFW_KEY_MINUS,       ///< The - key (hyphen)
                CapsLock = GLFW_KEY_CAPS_LOCK,

                Space = GLFW_KEY_SPACE,        ///< The Space key
                Enter = GLFW_KEY_ENTER,       ///< The Enter/Return keys

                Backspace = GLFW_KEY_BACKSPACE,    ///< The Backspace key
                Tab = GLFW_KEY_TAB,                ///< The Tabulation key
                PageUp = GLFW_KEY_PAGE_UP,         ///< The Page up key
                PageDown = GLFW_KEY_PAGE_DOWN,     ///< The Page down key
                End = GLFW_KEY_END,          ///< The End key
                Home = GLFW_KEY_HOME,         ///< The Home key
                Insert = GLFW_KEY_INSERT,       ///< The Insert key
                Delete = GLFW_KEY_DELETE,       ///< The Delete key
                Add = GLFW_KEY_KP_ADD,          ///< The + key
                Subtract = GLFW_KEY_MINUS,     ///< The - key (minus, usually from numpad)
                Multiply = GLFW_KEY_KP_MULTIPLY,     ///< The * key
                Divide = GLFW_KEY_KP_DIVIDE,       ///< The / key
                Left = GLFW_KEY_LEFT,         ///< Left arrow
                Right = GLFW_KEY_RIGHT,        ///< Right arrow
                Up = GLFW_KEY_UP,           ///< Up arrow
                Down = GLFW_KEY_DOWN,         ///< Down arrow
                Numpad0 = GLFW_KEY_KP_0,      ///< The numpad 0 key
                Numpad1 = GLFW_KEY_KP_1,      ///< The numpad 1 key
                Numpad2 = GLFW_KEY_KP_2,      ///< The numpad 2 key
                Numpad3 = GLFW_KEY_KP_3,      ///< The numpad 3 key
                Numpad4 = GLFW_KEY_KP_4,      ///< The numpad 4 key
                Numpad5 = GLFW_KEY_KP_5,      ///< The numpad 5 key
                Numpad6 = GLFW_KEY_KP_6,      ///< The numpad 6 key
                Numpad7 = GLFW_KEY_KP_7,      ///< The numpad 7 key
                Numpad8 = GLFW_KEY_KP_8,      ///< The numpad 8 key
                Numpad9 = GLFW_KEY_KP_9,      ///< The numpad 9 key
                F1    = GLFW_KEY_F1,           ///< The F1 key
                F2    = GLFW_KEY_F2,           ///< The F2 key
                F3    = GLFW_KEY_F3,           ///< The F3 key
                F4    = GLFW_KEY_F4,           ///< The F4 key
                F5    = GLFW_KEY_F5,           ///< The F5 key
                F6    = GLFW_KEY_F6,           ///< The F6 key
                F7    = GLFW_KEY_F7,           ///< The F7 key
                F8    = GLFW_KEY_F8,           ///< The F8 key
                F9    = GLFW_KEY_F9,           ///< The F9 key
                F10   = GLFW_KEY_F10,          ///< The F10 key
                F11   = GLFW_KEY_F11,          ///< The F11 key
                F12   = GLFW_KEY_F12,          ///< The F12 key
                F13   = GLFW_KEY_F13,          ///< The F13 key
                F14   = GLFW_KEY_F14,          ///< The F14 key
                F15   = GLFW_KEY_F15,          ///< The F15 key
                Pause = GLFW_KEY_PAUSE,        ///< The Pause key

                KeyCount,     ///< Keep last -- the total number of keyboard keys

                // Deprecated values:

               // Dash = Hyphen,       ///<  Use Hyphen instead
                Return = Enter         ///<  Use Enter instead
            };

        int getKey(const Key& key) { return keyButtons[(uint32_t)key]; }
    }

struct WindowCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    bool startMaximized = false;
    bool Vsync = false;
    std::string appName;
    bool startFullscreen = false;
    bool decorated = true;
};

const char* getClipboard() { return glfwGetClipboardString(nullptr); }
void setClipboard(const std::string& string) { glfwSetClipboardString(nullptr, string.c_str()); }

class Window {
public:
    Window() = default;
    ~Window() {}

    void Create(const WindowCreateInfo& info) {
        GLFWmonitor* mode = nullptr;

        if (info.startFullscreen) mode = glfwGetPrimaryMonitor();

        glfwWindowHint(GLFW_RESIZABLE, false);
        window = glfwCreateWindow(info.width, info.height, info.appName.c_str(), mode, nullptr);
        glfwSetWindowUserPointer(window, this);

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            scrollX = xoffset; scrollY = yoffset;
        
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
            });
        glfwSetCharCallback(window, [](GLFWwindow* window, uint32_t codepoint) {
            ImGui_ImplGlfw_CharCallback(window, codepoint);
            });
        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            keyButtons[key] = action;

            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
            });
        
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
            });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            mouseButtons[button] = action;
        
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
            });        
    }

    void CreateSwapchain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const uint32_t& width, const uint32_t& height) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
        { // choose swap surface format
            for (const auto& availableFormat : swapChainSupport.formats)
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    surfaceFormat = availableFormat;
        }
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        { // choose swap present mode
            for (const auto& availablePresentMode : swapChainSupport.presentModes)
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                    presentMode = availablePresentMode;
        }
        VkExtent2D extent;

        if (swapChainSupport.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) extent = swapChainSupport.capabilities.currentExtent;
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);

            extent = actualExtent;
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

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
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

    void SetCursorPosCallback(const GLFWcursorposfun& callback) const {
        glfwSetCursorPosCallback(window, callback);
    }

    void SetFramebufferSizeCallback(const GLFWframebuffersizefun& callback) {
        glfwSetFramebufferSizeCallback(window, callback);
    }

    void SetScrollCallback(const GLFWscrollfun& callback) {
        glfwSetScrollCallback(window, callback);
    }

    void SetCharCallback(const GLFWcharfun& callback) {
        glfwSetCharCallback(window, callback);
    }

    void SetMouseButtonCallback(const GLFWmousebuttonfun& callback) {
        glfwSetMouseButtonCallback(window, callback);
    }

    void SetKeyCallback(const GLFWkeyfun& callback) {
        glfwSetKeyCallback(window, callback);
    }

    void Destroy(VkInstance instance) const {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        glfwDestroyWindow(window);
    }

    float getContentScale() {
        float xscale, yscale;
        glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
        return xscale;
    }

    float getAspectRatio() {
        auto size = GetSize();
        return float((float)size.x / (float)size.y);
    }

    void poolEvents() const {
        scrollY = 0.f;
        scrollX = 0.f;
        memset(mouseButtons, GLFW_RELEASE, sizeof(mouseButtons));
        memset(keyButtons, GLFW_RELEASE, sizeof(keyButtons));
        glfwPollEvents();
    }

    glm::ivec2 GetPos() const {
        int xpos, ypos;
        glfwGetWindowPos(window, &xpos, &ypos);
        return { xpos, ypos };
    }

    glm::ivec2 GetSize() const {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        return { width, height };
    }

    VkExtent2D GetExtent() const {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        return { (uint32_t)width, (uint32_t)height };
    }

    void close() const {
        glfwSetWindowShouldClose(window, true);
    }

    bool isOpen() const {
        return !glfwWindowShouldClose(window);
    }

    bool hasFocus() const {
        return glfwGetWindowAttrib(window, GLFW_FOCUSED);
    }

    void setCursorPos(const glm::ivec2& pos) {
        glfwSetCursorPos(window, pos.x, pos.y);
    }

    void setMaximized(const bool& maximized) {
        if (maximized) glfwMaximizeWindow(window);
        else glfwRestoreWindow(window);
    }

    void setPosition(const glm::ivec2& pos) {
        glfwSetWindowPos(window, pos.x, pos.y);
    }

    void setTitle(const std::string& title) {
        glfwSetWindowTitle(window, title.c_str());
    }

    void setSize(const glm::ivec2& size) {
        glfwSetWindowSize(window, size.x, size.y);
    }

    void setSizeLimits(const glm::ivec2& minSize, const glm::ivec2& maxSize) {
        glfwSetWindowSizeLimits(window, minSize.x, minSize.y, maxSize.x, maxSize.y);
    }

    void ShowMouse(const bool& show) {
        if (show) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    int getKey(const int& key) {
        return glfwGetKey(window, key);
    }

    int getMouse(const int& key) {
        return glfwGetMouseButton(window, key);
    }

    glm::ivec2 getCursorPos() {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return glm::ivec2(x, y);
    }

    void Present(const uint32_t& swapchainImageIndex, const VkSemaphore& renderSemaphore, const VkQueue& presentQueue) {
        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

        presentInfo.pSwapchains = &swapchain;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swapchainImageIndex;

        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    inline operator GLFWwindow* () { return window; }
    inline operator GLFWwindow* () const { return window; }

    VkSwapchainKHR swapchain; // from other articles

    // image format expected by the windowing system
    VkFormat swapchainImageFormat;

    //array of images from the swapchain
    std::vector<VkImage> swapchainImages;

    //array of image-views from the swapchain
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkFramebuffer> framebuffers;

    VkSurfaceKHR surface; // Vulkan window surface	

private:
    GLFWwindow* window = nullptr;
};
    static Window window;

    namespace Keyboard {
        int isKeyPressed(const Key& key)
        {
            return window.getKey((int32_t)key);
        }
    }

	namespace Mouse {
		void SetMousePosition(const glm::ivec2& pos) {
            window.setCursorPos(pos);
		}

		void SetMousePosition(const int& x, const int& y) {
			SetMousePosition({ x,y });
		}

		glm::ivec2 GetMousePos() {			
            return window.getCursorPos() + window.GetPos();
		}

		glm::ivec2 GetMousePosToWindow() {
			return window.getCursorPos();
		}

		void ShowMouse(const bool& show) {
			window.ShowMouse(show);
		}

        int getMouse(const int& key) {
            return mouseButtons[key];
        }

        int isMouseButtonPressed(const int& key) {
            return window.getMouse(key);
        }
	}
}