#ifndef PCH_HPP
#define PCH_HPP

//#include <iostream>
//#include <future>
#include <ppl.h>
#include <thread>
#include <memory>

#include <fstream>
#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>
#include <sstream>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Internet connection
#include <net/wc_net.hpp>

//Custom libraries
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/IndexBuffer.hpp>
#include <gl/FrameBuffer.hpp>
#include <gl/Material.hpp>  
#include <gl/Light.hpp>
#include <gl/VertexArray.hpp>
#include <gl/VertexBuffer.hpp>
//#include "Skybox.hpp"

#include <Maths/Camera.hpp>

//Lua
#include <sol/sol.hpp>

//Util
#include <Utils/State.hpp>
#include <Utils/Keyboard.hpp>
#include <Utils/Mouse.hpp>
#include <Utils/Log.hpp>
#include <Utils/Window.hpp>
#include <Utils/Time.hpp>
#include <Utils/Random.hpp>
#include <Utils/Bits.hpp>

// GUI
#include <GUI/AssetManager.hpp>
#include <GUI/Renderer2D.hpp>
#include "Vertex.hpp"
#endif