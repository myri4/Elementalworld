#pragma once

#include <thread>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <array>
#include <iomanip>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Internet connection
#include <wc/net/wc_net.h>

// Utilities
#include <wc/Utils/Log.h>
#include <wc/Utils/Time.h>
#include <wc/Utils/Window.h>
#include <wc/Utils/List.h>

// Graphics
#include <wc/vk/Buffer.h>
#include <wc/vk/Pipeline.h>
#include <wc/vk/Images.h>
#include <wc/vk/Renderpass.h>
#include <wc/vk/Synchronization.h>

#include <wc/Math/Camera.h>

// GUI
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>

// Audio
#include <wc/Audio/AudioEngine.h>