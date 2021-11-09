#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include "Log.hpp"

namespace wc {

	bool resized = false;

	double scrollX = 0.f, scrollY = 0.f;
	bool mouseScrolled = false;
	uint32_t currKeyEntered = 0; 
	bool keyEntered = false;
	bool buttonPressed = false;
    int mouseButtons[GLFW_MOUSE_BUTTON_LAST];
    int keyButtons[GLFW_KEY_LAST];

	class Window {
	public:
		Window() {}
		~Window() {}

		void Create(const char* luaScript, const char* title) {
			sol::state windowScript;
			windowScript.script_file(luaScript);

			GLFWmonitor* mode = nullptr;
			if (windowScript["fullscreen"]) mode = glfwGetPrimaryMonitor();

			window = glfwCreateWindow(windowScript["screenWidth"], windowScript["screenHeight"], title, mode, nullptr);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, windowScript["antialiasingLevel"]);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

			bool vsync = windowScript["vsync"];
			glfwMakeContextCurrent(window);
			if (!vsync) glfwSwapInterval(0);
			glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {glViewport(0, 0, width, height); resized = true; });
			glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) { scrollX = xoffset; scrollY = yoffset; mouseScrolled = true; });
			glfwSetCharCallback(window, [](GLFWwindow* window, uint32_t codepoint) { currKeyEntered = codepoint; keyEntered = true; });
			glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                keyButtons[key] = action;
				buttonPressed = true;
				});

			glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
                mouseButtons[button] = action;
				});

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) WC_ERROR("Failed to initialize GLAD");
		}

		void Destroy() const {
			glfwDestroyWindow(window);
		}

		void display() const {
			resized = false;
            keyEntered = false;
			buttonPressed = false;
			mouseScrolled = false;
            memset(mouseButtons, GLFW_RELEASE, sizeof(mouseButtons));
            memset(keyButtons, GLFW_RELEASE, sizeof(keyButtons));
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		const char* getClipboard() {
			return glfwGetClipboardString(window);
		}

		void setClipboard(const char* string) {
			glfwSetClipboardString(window, string);
		}

		glm::ivec2 GetPos() const {
			int xpos, ypos;
			glfwGetWindowPos(window, &xpos, &ypos);
			return { xpos, ypos };
		}

		glm::ivec2 GetSize() const {
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			return { width, height };
		}

		void close() const {
			glfwSetWindowShouldClose(window, true);
		}

		bool isOpen() const {
			return !glfwWindowShouldClose(window);
		}

		bool hasFocus() const {
			return glfwGetWindowAttrib(window, GLFW_FOCUSED);
		}

		void setActive() const {
			glfwMakeContextCurrent(window);
		}

		void setCursorPos(const glm::ivec2& pos) {
			glfwSetCursorPos(window, pos.x, pos.y);
		}

		void ShowMouse(const bool& show) {
			if (show) 
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);			
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}

        int getKey(const int& key) {
            return glfwGetKey(window, key);
        }

        int getMouse(const int& key) {
            return glfwGetMouseButton(window, key);
        }

		glm::ivec2 getCursorPos() {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			return glm::ivec2(x, y);
		}
	private:
		GLFWwindow* window = nullptr;
	}window;

    namespace Keyboard {

        enum class Key {
                Unknown = -1, ///< Unhandled key
                A = GLFW_KEY_A,        ///< The A key
                B = GLFW_KEY_B,            ///< The B key
                C = GLFW_KEY_C,            ///< The C key
                D = GLFW_KEY_D,            ///< The D key
                E = GLFW_KEY_E,            ///< The E key
                F = GLFW_KEY_F,            ///< The F key
                G = GLFW_KEY_G,            ///< The G key
                H = GLFW_KEY_H,            ///< The H key
                I = GLFW_KEY_I,            ///< The I key
                J = GLFW_KEY_J,            ///< The J key
                K = GLFW_KEY_K,            ///< The K key
                L = GLFW_KEY_L,            ///< The L key
                M = GLFW_KEY_M,            ///< The M key
                N = GLFW_KEY_N,            ///< The N key
                O = GLFW_KEY_O,            ///< The O key
                P = GLFW_KEY_P,            ///< The P key
                Q = GLFW_KEY_Q,            ///< The Q key
                R = GLFW_KEY_R,            ///< The R key
                S = GLFW_KEY_S,            ///< The S key
                T = GLFW_KEY_T,            ///< The T key
                U = GLFW_KEY_U,            ///< The U key
                V = GLFW_KEY_V,            ///< The V key
                W = GLFW_KEY_W,            ///< The W key
                X = GLFW_KEY_X,            ///< The X key
                Y = GLFW_KEY_Y,            ///< The Y key
                Z = GLFW_KEY_Z,            ///< The Z key

                Num0 = GLFW_KEY_0,         ///< The 0 key
                Num1 = GLFW_KEY_1,         ///< The 1 key
                Num2 = GLFW_KEY_2,         ///< The 2 key
                Num3 = GLFW_KEY_3,         ///< The 3 key
                Num4 = GLFW_KEY_4,         ///< The 4 key
                Num5 = GLFW_KEY_5,         ///< The 5 key
                Num6 = GLFW_KEY_6,         ///< The 6 key
                Num7 = GLFW_KEY_7,         ///< The 7 key
                Num8 = GLFW_KEY_8,         ///< The 8 key
                Num9 = GLFW_KEY_9,         ///< The 9 key

                Escape = GLFW_KEY_ESCAPE,       ///< The Escape key

                LControl = GLFW_KEY_LEFT_CONTROL,     ///< The left Control key
                LShift = GLFW_KEY_LEFT_SHIFT,       ///< The left Shift key       
                LAlt = GLFW_KEY_LEFT_ALT,         ///< The left Alt key
                LSystem = GLFW_KEY_LEFT_SUPER,      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
                LBracket = GLFW_KEY_LEFT_BRACKET,     ///< The [ key

                RControl = GLFW_KEY_RIGHT_CONTROL,     ///< The right Control key
                RShift = GLFW_KEY_RIGHT_SHIFT,       ///< The right Shift key
                RAlt = GLFW_KEY_RIGHT_ALT,         ///< The right Alt key        
                RSystem = GLFW_KEY_RIGHT_SUPER,      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
                RBracket = GLFW_KEY_RIGHT_BRACKET,     ///< The ] key

                Menu = GLFW_KEY_MENU,         ///< The Menu key
                Semicolon = GLFW_KEY_SEMICOLON,    ///< The ; key        
                Comma = GLFW_KEY_COMMA,        ///< The , key
                Period = GLFW_KEY_PERIOD,       ///< The . key
                Quote = GLFW_KEY_APOSTROPHE,        ///< The ' key
                Slash = GLFW_KEY_SLASH,        ///< The / key
                Backslash = GLFW_KEY_BACKSLASH,    ///< The \ key
                //Tilde = GLFW_KEY_,        ///< The ~ key
                Equal = GLFW_KEY_EQUAL,        ///< The = key
                //Hyphen = GLFW_KEY_MINUS,       ///< The - key (hyphen)

                Space = GLFW_KEY_SPACE,        ///< The Space key
                Enter = GLFW_KEY_ENTER,       ///< The Enter/Return keys

                Backspace = GLFW_KEY_BACKSPACE,    ///< The Backspace key
                Tab = GLFW_KEY_TAB,                ///< The Tabulation key
                PageUp = GLFW_KEY_PAGE_UP,         ///< The Page up key
                PageDown = GLFW_KEY_PAGE_DOWN,     ///< The Page down key
                End = GLFW_KEY_END,          ///< The End key
                Home = GLFW_KEY_HOME,         ///< The Home key
                Insert = GLFW_KEY_INSERT,       ///< The Insert key
                Delete = GLFW_KEY_DELETE,       ///< The Delete key
                Add = GLFW_KEY_KP_ADD,          ///< The + key
                Subtract = GLFW_KEY_MINUS,     ///< The - key (minus, usually from numpad)
                Multiply = GLFW_KEY_KP_MULTIPLY,     ///< The * key
                Divide = GLFW_KEY_KP_DIVIDE,       ///< The / key
                Left = GLFW_KEY_LEFT,         ///< Left arrow
                Right = GLFW_KEY_RIGHT,        ///< Right arrow
                Up = GLFW_KEY_UP,           ///< Up arrow
                Down = GLFW_KEY_DOWN,         ///< Down arrow
                Numpad0 = GLFW_KEY_KP_0,      ///< The numpad 0 key
                Numpad1 = GLFW_KEY_KP_1,      ///< The numpad 1 key
                Numpad2 = GLFW_KEY_KP_2,      ///< The numpad 2 key
                Numpad3 = GLFW_KEY_KP_3,      ///< The numpad 3 key
                Numpad4 = GLFW_KEY_KP_4,      ///< The numpad 4 key
                Numpad5 = GLFW_KEY_KP_5,      ///< The numpad 5 key
                Numpad6 = GLFW_KEY_KP_6,      ///< The numpad 6 key
                Numpad7 = GLFW_KEY_KP_7,      ///< The numpad 7 key
                Numpad8 = GLFW_KEY_KP_8,      ///< The numpad 8 key
                Numpad9 = GLFW_KEY_KP_9,      ///< The numpad 9 key
                F1    = GLFW_KEY_F1,           ///< The F1 key
                F2    = GLFW_KEY_F2,           ///< The F2 key
                F3    = GLFW_KEY_F3,           ///< The F3 key
                F4    = GLFW_KEY_F4,           ///< The F4 key
                F5    = GLFW_KEY_F5,           ///< The F5 key
                F6    = GLFW_KEY_F6,           ///< The F6 key
                F7    = GLFW_KEY_F7,           ///< The F7 key
                F8    = GLFW_KEY_F8,           ///< The F8 key
                F9    = GLFW_KEY_F9,           ///< The F9 key
                F10   = GLFW_KEY_F10,          ///< The F10 key
                F11   = GLFW_KEY_F11,          ///< The F11 key
                F12   = GLFW_KEY_F12,          ///< The F12 key
                F13   = GLFW_KEY_F13,          ///< The F13 key
                F14   = GLFW_KEY_F14,          ///< The F14 key
                F15   = GLFW_KEY_F15,          ///< The F15 key
                Pause = GLFW_KEY_PAUSE,        ///< The Pause key

                KeyCount,     ///< Keep last -- the total number of keyboard keys

                // Deprecated values:

               // Dash = Hyphen,       ///<  Use Hyphen instead
                Return = Enter         ///<  Use Enter instead
            };

        int getKey(const Key& key)
        {
            return keyButtons[(uint32_t)key];
        }

        int isKeyPressed(const Key& key)
        {
            return window.getKey((int32_t)key);
        }
    }

	namespace Mouse {
		void SetMousePosition(const glm::ivec2& pos) {
            window.setCursorPos(pos);
		}

		void SetMousePosition(const int& x, const int& y) {
			SetMousePosition({ x,y });
		}

		glm::ivec2 GetMousePos() {			
            return window.getCursorPos() + window.GetPos();
		}

		glm::ivec2 GetMousePosToWindow() {
			return window.getCursorPos();
		}

		void ShowMouse(const bool& show) {
			window.ShowMouse(show);
		}

        int getMouse(const int& key) {
            return mouseButtons[key];
        }

        int isMouseButtonPressed(const int& key) {
            return window.getMouse(key);
        }
	}
}