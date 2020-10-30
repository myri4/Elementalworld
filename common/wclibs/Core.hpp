#pragma once

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