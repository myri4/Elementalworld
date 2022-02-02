#pragma once
#include "../Utils/Window.hpp"

namespace wc {
class Textbox {
public:
	std::string text;
	bool isSelected = false;

	void update() {
		if (isSelected) {
			if (keyEntered) text += static_cast<char>(currKeyEntered);
			else if (buttonPressed) {
				if (Keyboard::getKey(Keyboard::Key::Backspace) != GLFW_RELEASE)
					if (text.length() > 0)
					{
						std::string t = text;
						uint32_t length = text.length() - 1;
						text = "";
						for (uint32_t i = 0; i < length; i++) text += t[i];
					}

					if (Keyboard::isKeyPressed(Keyboard::Key::V) != GLFW_RELEASE && Keyboard::isKeyPressed(Keyboard::Key::LControl) != GLFW_RELEASE) text += window.getClipboard();
			}
		}
	}

};
}