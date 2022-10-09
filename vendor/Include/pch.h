#pragma once

#include <thread>

#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <array>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Internet connection
#include <net/wc_net.h>

//Util
#include <wc/Utils/Log.h>
#include <wc/Utils/Time.h>
#include <wc/Utils/Random.h>
#include <wc/Utils/Window.h>
#include <wc/Utils/List.h>

//Custom libraries
#include <wc/vk/Buffer.h>
#include <wc/vk/Pipeline.h>
#include <wc/vk/Images.h>
#include <wc/vk/Renderpass.h>
#include <wc/vk/Synchronization.h>

#include <wc/Maths/Camera.h>

//Lua
#include <sol/sol.hpp>

// GUI
#include <wc/GUI/AssetManager.h>
#include <wc/GUI/GUI.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>