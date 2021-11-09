#pragma once

#include <thread>
#include <memory>

#include <fstream>
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
//#include <net/wc_net.hpp>

//Custom libraries
#include <gl/Buffer.hpp>
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/VertexArray.hpp>
#include <gl/FrameBuffer.hpp>
#include "Skybox.hpp"

#include <Maths/Camera.hpp>

//Lua
#include <sol/sol.hpp>

//Util
#include <Utils/Log.hpp>
#include <Utils/Time.hpp>
#include <Utils/State.hpp>
#include <Utils/Random.hpp>
#include <Utils/Window.hpp>

// GUI
#include <GUI/Renderer2D.hpp>
#include <GUI/AssetManager.hpp>