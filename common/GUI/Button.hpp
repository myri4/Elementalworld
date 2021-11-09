#pragma once

#include "../Utils/Window.hpp"

namespace wc {

class Button {
public:
	glm::ivec2 position;
	glm::ivec2 size;

	bool isMouseOver() {
		glm::ivec2 mousepos = wc::Mouse::GetMousePosToWindow();
		if (position.x < mousepos.x && position.y < mousepos.y && position.x + size.x > mousepos.x && position.y + size.y > mousepos.y) return true;
		return false;
	}
};

struct DragButton : public Button {
	bool attached = false;
	glm::ivec2 offset;

	inline void attach(const glm::ivec2& Position, const bool& attachment = true) {
		attached = attachment;
		offset = position - Position;
	}

	inline void updatePosition(const glm::ivec2& Position) {
		if (attached) position = Position + offset;
	}
};

struct Slider {
	Button sliderButton;
	glm::vec2 start;
	glm::vec2 end;
	glm::vec2 current;

	float getFactor() {
		return glm::length(current - start) / glm::length(start - end);
	}

	glm::vec2 getButtonPosition(const glm::vec2& size) {
		return current - size * 0.5f;
	}

	void upadatePosition() {
		if (current.x > end.x) current.x = end.x;
		if (current.y > end.y) current.y = end.y;

		if (current.x < start.x) current.x = start.x;
		if (current.y < start.y) current.y = start.y;
	}
};
}