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

//Internet connection
#include <net/wc_net.h>

//Util
#include <Utils/Log.h>
#include <Utils/Time.h>
#include <Utils/State.h>
#include <Utils/Random.h>
#include <Utils/Window.h>
#include <Utils/List.h>

//Custom libraries
#include <vk/Buffer.h>
#include <vk/Pipeline.h>
#include <vk/Images.h>
#include <vk/Renderpass.h>
#include <vk/Synchronization.h>

#include <Maths/Camera.h>

//Lua
#include <sol/sol.hpp>

// GUI
#include <GUI/AssetManager.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <GUI/GUI.h>