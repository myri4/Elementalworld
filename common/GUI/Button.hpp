#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../Utils/Mouse.hpp"

namespace wc{

class Button {
public:
	glm::vec2 position;
	glm::vec2 size;

	bool isMouseOver(const glm::vec2& windowPos) {
		glm::vec2 mousepos = wc::Mouse::GetMousePosToWindow(windowPos);
		if (position.x < mousepos.x && position.y < mousepos.y && position.x + size.x > mousepos.x && position.y + size.y > mousepos.y) return true;
		return false;
	}
};

}

#endif