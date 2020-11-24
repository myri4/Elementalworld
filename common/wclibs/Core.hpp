#ifndef CORE_HPP
#define CORE_HPP

#pragma comment(lib, "liblua54.a")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "irrKlang.lib")

#ifdef _WIN32
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

#endif // _WIN32
#endif