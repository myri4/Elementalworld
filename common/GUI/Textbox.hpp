#pragma once
#include "../Utils/Window.hpp"
#include "../Utils/Keyboard.hpp"

namespace wc {
class Textbox {
public:
	std::string text;
	const char* clipBoard;
	bool isSelected = false;

	void update() {
		if (isSelected) {
			if (keyPressed) text += static_cast<char>(currentKeyPressed);
			else if (buttonPressed) {
				if (currKey == GLFW_KEY_BACKSPACE && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
					if (text.length() > 0)
					{
						std::string t = text;
						uint32_t length = text.length() - 1;
						text = "";
						for (uint32_t i = 0; i < length; i++) text += t[i];
					}

				if (Keyboard::isKeyPressed(Keyboard::Key::V) && Keyboard::isKeyPressed(Keyboard::Key::LControl)) text += clipBoard;
			}
		}
	}

};
}