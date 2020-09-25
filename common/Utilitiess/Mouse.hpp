#pragma once

#include <wclibs/Core.hpp>
#include <glm/glm.hpp>

namespace ew{
void SetMousePosition(int x, int y) {
#ifdef _WIN32
	SetCursorPos(x, y);
#endif
}

void SetMousePosition(float x, float y) {
#ifdef _WIN32
	SetCursorPos(x, y);
#endif
}

void SetMousePosition(glm::vec2 pos) {
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
void ShowMouse(bool show) {
#ifdef _WIN32
	ShowCursor(show);
#endif
}
}