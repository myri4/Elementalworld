#pragma once


#include <iostream>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <math.h>
#include <future>
#include <thread>
#include <memory>

#include <fstream>
#include <string>
#include <vector>
#include <ostream>
#include <map>
#include <sstream>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include <stb_image/stb_image.h>

#include <SFML/Config.hpp>
#include <SFML/GpuPreference.hpp>

//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//window
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Window/WindowStyle.hpp>

//Graphics
#include <SFML/Graphics/RenderWindow.hpp>

//Custom libraries
#include <gl/Camera.hpp>
#include <gl/Shaders.hpp>
#include <gl/Texture.hpp>
#include <gl/VertexBuffer.hpp>
#include <gl/Skybox.hpp>
#include <gl/IndexBuffer.hpp>
#include <gl/Text.hpp>
#include <gl/FrameBuffer.hpp>
#include <gl/Vertex.hpp>
#include <gl/Material.hpp>  
#include <gl/Light.hpp>

//Core file
#include <wclibs/Core.hpp>

//Lua
#include <Utils/Lua.hpp>

//Sound
#include <irrKlang/irrKlang.h>

//Utilitiess
#include <Utils/State.hpp>
#include <Utils/Keyboard.hpp>
#include <Utils/Mouse.hpp>
#include <Utils/Log.hpp>