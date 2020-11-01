#pragma once

#include <wclibs/Core.hpp>
#include <glm/glm.hpp>

namespace wc{
namespace Mouse {
void SetMousePosition(const int& x, const int& y) {

#ifdef _WIN32
	SetCursorPos(x, y);
#endif

}

void SetMousePosition(const glm::ivec2& pos) {

#ifdef _WIN32
	SetCursorPos(pos.x, pos.y);
#endif

}

glm::vec2 GetMousePos() {

#ifdef _WIN32
	POINT p;
	GetCursorPos(&p);
	return { p.x,p.y };
#else

	return { 0, 0 };
#endif // _WIN32
}

void ShowMouse(const bool& show) {

#ifdef _WIN32
	ShowCursor(show);
#endif

}
	enum class MouseButton{NONE, LBUTTON, RBUTTON};
	MouseButton isButtonPressed() {

#ifdef _WIN32
		if (GetAsyncKeyState(VK_LBUTTON)) return MouseButton::LBUTTON;
		if (GetAsyncKeyState(VK_RBUTTON)) return MouseButton::RBUTTON;
#endif // _WIN32


		return MouseButton::NONE;
	}
}
}