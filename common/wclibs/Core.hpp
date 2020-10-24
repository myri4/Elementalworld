#pragma once
#include <memory>
#include <glad/glad.h>

#ifdef  _WIN32
#include <Windows.h>
#endif
#pragma comment(lib, "liblua54.a")
#pragma comment(lib, "freetype.lib")


//TODO: LINUX MOUSE & KEYBOARD IMPL
#ifdef  __LINUX__

#endif


#ifdef  _WIN32
//Visual studio specific
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "irrKlang.lib")

#ifdef _DEBUG
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
//#pragma comment(lib, "sfml-network-d.lib")
//#pragma comment(lib, "sfml-audio-d.lib")

#else
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
//#pragma comment(lib, "sfml-network.lib")
//#pragma comment(lib, "sfml-audio.lib")
#endif //  _RELEASE

#endif //  _WIN32


//Custom definitions
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

//OpenGL Memory Buffer Variables
static const size_t chunkSize = 32;

static const size_t MaxFaceCount = 100;
static const size_t MaxVertexCount = MaxFaceCount * 4;
static const size_t MaxIndexCount = MaxFaceCount * 6;

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
