#pragma once
#include <GUI/Button.hpp>
#include <wc/pch.hpp>
#include <Utils/Window.hpp>

namespace wc {

	enum MenuMode { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU };
	uint32_t mode = MenuMode::MAINMENU; // @TODO: Remove it from here and make a file that needs to be include everywhere

	class EscMenu {
		TextButton resume;
		TextButton mainMenu;
		TextButton quitGame;
	public:
		void OnCreate(const Font& font, const float& scale) {
			resume.text = "Resume";
			resume.CenterText(font, scale);

			resume.position = { 0,0 };
			resume.size = { 500,100 };

			mainMenu.text = "Main menu";
			mainMenu.CenterText(font, scale);

			mainMenu.position = { 0,100 };
			mainMenu.size = { 500,100 };

			quitGame.text = "Quit game";
			quitGame.CenterText(font, scale);

			quitGame.position = { 0,200 };
			quitGame.size = { 500,100 };
		}

		void OnInput() {
			if (resume.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) {
				int16_t xt, yt;
				glm::ivec2 windSize = window.GetSize();
				xt = windSize.x / 2;
				yt = windSize.y / 2;
				Mouse::SetMousePosition(xt, yt);
				mode = MenuMode::GAME;
			}
			else if (mainMenu.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::MAINMENU;
			else if (quitGame.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) window.close();
		}

		void OnUpdate(const Font& font, const float& scale) {
			resume.Render(font, scale);
			mainMenu.Render(font, scale);
			quitGame.Render(font, scale);
		}
	};

	//ipTextbox.text = "25.32.4.119";
	//playerName.text = "321";	
}