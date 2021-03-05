#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <glm/glm.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace wc{
namespace Mouse {
	// @TODO: OS Specific
void SetMousePosition(const int& x, const int& y) {
	SetCursorPos(x, y);
}

void SetMousePosition(const glm::ivec2& pos) {
	SetCursorPos(pos.x, pos.y);
}

glm::vec2 GetMousePos() {
	POINT p;
	GetCursorPos(&p);
	return { p.x,p.y };
}

void ShowMouse(const bool& show) {
	ShowCursor(show);
}
	
enum class MouseButton{NONE, LBUTTON, RBUTTON};

MouseButton isButtonPressed() {
	if (GetAsyncKeyState(VK_LBUTTON)) return MouseButton::LBUTTON;
	if (GetAsyncKeyState(VK_RBUTTON)) return MouseButton::RBUTTON;

	return MouseButton::NONE;
}

}
}
#endif