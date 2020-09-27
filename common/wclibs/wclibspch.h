#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <map>
#include <sstream>
#include <array>
#include <memory>
#include <future>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image/stb_image.h>

#include <SFML/Config.hpp>
#include <SFML/GpuPreference.hpp>

//main
//#include <SFML/Main.hpp>

//window
#include <SFML/Window/Clipboard.hpp>
//#include <SFML/Window/Context.hpp>
#include <SFML/Window/ContextSettings.hpp>
//#include <SFML/Window/Cursor.hpp>
#include <SFML/Window/Event.hpp>
//#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
//#include <SFML/Window/Mouse.hpp>
//#include <SFML/Window/Sensor.hpp>
//#include <SFML/Window/Touch.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Window/WindowStyle.hpp>

//Graphics
#include <SFML/Graphics/RenderWindow.hpp>

//Custom libraries
//#include <gl/glErrors.h>
#include <gl/Camera.h>
#include <gl/Shaders.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <gl/Skybox.h>
#include <gl/IndexBuffer.h>
#include <gl/Text.h>
#include <gl/FrameBuffer.h>

//Core file
#include "Core.hpp"

//Lua
#include <Utilitiess/Lua.hpp>
#include <GLFW/glfw3.h>