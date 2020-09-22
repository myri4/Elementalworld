#pragma once

#ifdef  _WIN32
#pragma comment(lib, "liblua54.a")
#pragma comment(lib, "freetype.lib")

#ifdef _DEBUG
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
//#pragma comment(lib, "sfml-network-d.lib")
//#pragma comment(lib, "sfml-audio-d.lib")
#endif // _DEBUG

#ifdef WC_RELEASE
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
//#pragma comment(lib, "sfml-network.lib")
//#pragma comment(lib, "sfml-audio.lib")
#endif //  _RELEASE

#endif //  _WIN32

#ifdef _DEBUG

#define dMsg(...) std::cout<<__VA_ARGS__
#else
#define dMsg(...)
#endif // DEBUG

#include <memory>

//Custom definitions
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

//OpenGL Memory Buffer Variables
static const size_t MaxFaceCount = 1000;
static const size_t MaxVertexCount = MaxFaceCount * 4;
static const size_t MaxIndexCount = MaxFaceCount * 6;
static const size_t MaxTextures = 32;

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;



typedef signed char        int8;
typedef short              int16;
typedef int                int32;
typedef long long          int64;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;