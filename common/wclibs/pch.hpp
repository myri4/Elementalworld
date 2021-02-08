#ifndef PCH_HPP
#define PCH_HPP

//#include <iostream>
//#include <future>
//#include <thread>
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

//Custom libraries
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/Vertex.hpp>
#include <gl/IndexBuffer.hpp>
#include <gl/FrameBuffer.hpp>
#include <gl/Material.hpp>  
#include <gl/Light.hpp>
//#include "Skybox.hpp"

#include <Maths/Camera.hpp>

//Core file
#include <wclibs/Core.hpp>

//Lua
#include <sol/sol.hpp>

//Sound
//#include <irrKlang/irrKlang.h>

//Util
#include <Utils/State.hpp>
#include <Utils/Keyboard.hpp>
#include <Utils/Mouse.hpp>
#include <Utils/Log.hpp>
#include <Utils/Window.hpp>
#include <Utils/Time.hpp>

// GUI
#include <GUI/AssetManager.hpp>
#include <GUI/Renderer2D.hpp>
#endif