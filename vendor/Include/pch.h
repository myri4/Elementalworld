#pragma once

#include <thread>
#include <memory>

#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>
#include <sstream>
#include <array>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Internet connection
#include <net/wc_net.h>

//Custom libraries
#include <gl/Buffer.h>
#include <gl/Shaders.h>
#include <gl/Texture.h>
#include <gl/FrameBuffer.h>
#include <gl/Fence.h>

#include <Maths/Camera.h>

//Lua
#include <sol/sol.hpp>

//Util
#include <Utils/Log.h>
#include <Utils/Time.h>
#include <Utils/State.h>
#include <Utils/Random.h>
#include <Utils/Window.h>
#include <Utils/List.h>

// GUI
<<<<<<< Updated upstream
#include <GUI/AssetManager.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <GUI/GUI.h>
=======
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>
>>>>>>> Stashed changes
