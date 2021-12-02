#pragma once
#include <GUI/Button.hpp>
#include <GUI/Textbox.hpp>
#include <wc/pch.hpp>

namespace wc {

	enum class MenuMode { GAME, INVENTORY, MAINMENU, SETTINGS, ESCMENU };
	MenuMode mode = MenuMode::MAINMENU; // @TODO: Remove it from here and make a file that needs to be include everywhere

	class MainMenu {
		TextButton singlePlayer;
		TextButton multiPlayer;
		TextButton settings;
	public:
		void OnCreate(const Font& font, const float& scale) {
			singlePlayer.text = "Singleplayer";
			singlePlayer.CenterText(font, scale);

			singlePlayer.position = { 0,0 };
			singlePlayer.size = { 500,100 };

			multiPlayer.text = "Multiplayer";
			multiPlayer.CenterText(font, scale);

			multiPlayer.position = { 0,100 };
			multiPlayer.size = { 500,100 };

			settings.text = "Settings";
			settings.CenterText(font, scale);

			settings.position = { 0,200 };
			settings.size = { 500,100 };
		}

		void OnInput() {
			if (singlePlayer.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::GAME;
			//else if (settings.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::SETTINGS;
			//else if (multiPlayer.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::SETTINGS;
		}

		void OnUpdate(const Font& font, const float& scale) {
			singlePlayer.Render(font, scale);
			multiPlayer.Render(font, scale);
			settings.Render(font, scale);
		}
	};

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
			if (resume.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::GAME;
			else if (mainMenu.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::MAINMENU;
			else if (quitGame.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) window.close();
		}

		void OnUpdate(const Font& font, const float& scale) {
			resume.Render(font, scale);
			mainMenu.Render(font, scale);
			quitGame.Render(font, scale);
		}
	};



}