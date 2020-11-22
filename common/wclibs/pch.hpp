#ifndef PCH_HPP
#define PCH_HPP

//#include <iostream>
//#include <algorithm>
//#include <ctime>
//#include <math.h>
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

#include <SFML/Config.hpp>
#include <SFML/GpuPreference.hpp>

//OpenGL
#include <glad/glad.h>
//#include <GLFW/glfw3.h>

//SFML Stuff
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>

//Custom libraries
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/Vertex.hpp>
#include <gl/Skybox.hpp>
#include <gl/IndexBuffer.hpp>
#include <gl/Text.hpp>
#include <gl/FrameBuffer.hpp>
#include <gl/Material.hpp>  
#include <gl/Light.hpp>
#include <Maths/Camera.hpp>

//Core file
#include <wclibs/Core.hpp>

//Lua
#include <lua/lua.hpp>
#include <sol/sol.hpp>

//Sound
#include <irrKlang/irrKlang.h>

//Util
#include <Utils/State.hpp>
#include <Utils/Keyboard.hpp>
#include <Utils/Mouse.hpp>
#include <Utils/Log.hpp>

#endif