#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <glm/glm.hpp>

#include <Windows.h>

namespace wc{
namespace Mouse {
	// @TODO: OS Specific
void SetMousePosition(const int& x, const int& y) {
	SetCursorPos(x, y);
}

void SetMousePosition(const glm::ivec2& pos) {
	SetCursorPos(pos.x, pos.y);
}

glm::ivec2 GetMousePos() {
	POINT p;
	GetCursorPos(&p);
	return { p.x, p.y };
}

glm::ivec2 GetMousePosToWindow(const glm::ivec2& windowPos) {
	glm::ivec2 pos = GetMousePos();
	return { pos.x - windowPos.x, pos.y - windowPos.y };
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