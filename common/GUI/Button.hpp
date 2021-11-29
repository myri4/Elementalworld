#pragma once

#include "../Utils/Window.hpp"
#include "Renderer2D.hpp"

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

class TextButton : public Button {
public:
	std::string text;
	glm::ivec2 textSize = glm::ivec2(0.f);
	
	void CenterText(const Font& font, const float& scale) {
		for (auto& c : text) {
			Character ch = font.Characters[c];

			float xpos = textSize.x + ch.Bearing.x * scale;
			float ypos = textSize.y - ch.Bearing.y * scale;

			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			textSize.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
	}

	void Render(const Font& font, const float& scale) {
		Renderer2D::DrawQuad(position, size);
		Renderer2D::DrawText(text, font, position + (size - textSize) / 2, scale);
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