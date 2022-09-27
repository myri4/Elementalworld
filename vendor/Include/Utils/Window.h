#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include "Log.h"
#include <RmlUi/Core.h>
#include <imgui/imgui_impl_glfw.h>

namespace wc {

	double scrollX = 0.f, scrollY = 0.f;
    int mouseButtons[GLFW_MOUSE_BUTTON_LAST];
    int keyButtons[GLFW_KEY_LAST];
    Rml::Context* context = nullptr;

    namespace Keyboard {

        enum class Key {
                Unknown = -1,              ///< Unhandled key
                A = GLFW_KEY_A,            ///< The A key
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
                CapsLock = GLFW_KEY_CAPS_LOCK,

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
    }

    Rml::Input::KeyIdentifier convertToRmlKey(const Keyboard::Key& key) {
            if (key == Keyboard::Key::A) return Rml::Input::KeyIdentifier::KI_A;
            if (key == Keyboard::Key::B) return Rml::Input::KeyIdentifier::KI_B;
            if (key == Keyboard::Key::C) return Rml::Input::KeyIdentifier::KI_C;
            if (key == Keyboard::Key::D) return Rml::Input::KeyIdentifier::KI_D;
            if (key == Keyboard::Key::E) return Rml::Input::KeyIdentifier::KI_E;
            if (key == Keyboard::Key::F) return Rml::Input::KeyIdentifier::KI_F;
            if (key == Keyboard::Key::G) return Rml::Input::KeyIdentifier::KI_G;
            if (key == Keyboard::Key::H) return Rml::Input::KeyIdentifier::KI_H;
            if (key == Keyboard::Key::I) return Rml::Input::KeyIdentifier::KI_I;
            if (key == Keyboard::Key::J) return Rml::Input::KeyIdentifier::KI_J;
            if (key == Keyboard::Key::K) return Rml::Input::KeyIdentifier::KI_K;
            if (key == Keyboard::Key::L) return Rml::Input::KeyIdentifier::KI_L;
            if (key == Keyboard::Key::M) return Rml::Input::KeyIdentifier::KI_M;
            if (key == Keyboard::Key::N) return Rml::Input::KeyIdentifier::KI_N;
            if (key == Keyboard::Key::O) return Rml::Input::KeyIdentifier::KI_O;
            if (key == Keyboard::Key::P) return Rml::Input::KeyIdentifier::KI_P;
            if (key == Keyboard::Key::Q) return Rml::Input::KeyIdentifier::KI_Q;
            if (key == Keyboard::Key::R) return Rml::Input::KeyIdentifier::KI_R;
            if (key == Keyboard::Key::S) return Rml::Input::KeyIdentifier::KI_S;
            if (key == Keyboard::Key::T) return Rml::Input::KeyIdentifier::KI_T;
            if (key == Keyboard::Key::U) return Rml::Input::KeyIdentifier::KI_U;
            if (key == Keyboard::Key::V) return Rml::Input::KeyIdentifier::KI_V;
            if (key == Keyboard::Key::W) return Rml::Input::KeyIdentifier::KI_W;
            if (key == Keyboard::Key::X) return Rml::Input::KeyIdentifier::KI_X;
            if (key == Keyboard::Key::Y) return Rml::Input::KeyIdentifier::KI_Y;
            if (key == Keyboard::Key::Z) return Rml::Input::KeyIdentifier::KI_Z;

            if (key == Keyboard::Key::Num0) return Rml::Input::KeyIdentifier::KI_0;
            if (key == Keyboard::Key::Num1) return Rml::Input::KeyIdentifier::KI_1;
            if (key == Keyboard::Key::Num2) return Rml::Input::KeyIdentifier::KI_2;
            if (key == Keyboard::Key::Num3) return Rml::Input::KeyIdentifier::KI_3;
            if (key == Keyboard::Key::Num4) return Rml::Input::KeyIdentifier::KI_4;
            if (key == Keyboard::Key::Num5) return Rml::Input::KeyIdentifier::KI_5;
            if (key == Keyboard::Key::Num6) return Rml::Input::KeyIdentifier::KI_6;
            if (key == Keyboard::Key::Num7) return Rml::Input::KeyIdentifier::KI_7;
            if (key == Keyboard::Key::Num8) return Rml::Input::KeyIdentifier::KI_8;
            if (key == Keyboard::Key::Num9) return Rml::Input::KeyIdentifier::KI_9;

            if (key == Keyboard::Key::Space) return Rml::Input::KeyIdentifier::KI_SPACE;

            if (key == Keyboard::Key::Semicolon) return Rml::Input::KeyIdentifier::KI_OEM_1;
            if (key == Keyboard::Key::Add) return Rml::Input::KeyIdentifier::KI_OEM_PLUS;
            if (key == Keyboard::Key::Comma) return Rml::Input::KeyIdentifier::KI_OEM_COMMA;
            if (key == Keyboard::Key::Subtract) return Rml::Input::KeyIdentifier::KI_OEM_MINUS;
            if (key == Keyboard::Key::Period) return Rml::Input::KeyIdentifier::KI_OEM_PERIOD;
            if (key == Keyboard::Key::Slash) return Rml::Input::KeyIdentifier::KI_OEM_2;
           // if (key == Keyboard::Key::Slash) return Rml::Input::KeyIdentifier::KI_OEM_3;
            if (key == Keyboard::Key::LBracket) return Rml::Input::KeyIdentifier::KI_OEM_4;
            if (key == Keyboard::Key::Backslash) return Rml::Input::KeyIdentifier::KI_OEM_5;
            if (key == Keyboard::Key::RBracket) return Rml::Input::KeyIdentifier::KI_OEM_6;
            if (key == Keyboard::Key::Quote) return Rml::Input::KeyIdentifier::KI_OEM_7;


            if (key == Keyboard::Key::Numpad0) return Rml::Input::KeyIdentifier::KI_NUMPAD0;
            if (key == Keyboard::Key::Numpad1) return Rml::Input::KeyIdentifier::KI_NUMPAD1;
            if (key == Keyboard::Key::Numpad2) return Rml::Input::KeyIdentifier::KI_NUMPAD2;
            if (key == Keyboard::Key::Numpad3) return Rml::Input::KeyIdentifier::KI_NUMPAD3;
            if (key == Keyboard::Key::Numpad4) return Rml::Input::KeyIdentifier::KI_NUMPAD4;
            if (key == Keyboard::Key::Numpad5) return Rml::Input::KeyIdentifier::KI_NUMPAD5;
            if (key == Keyboard::Key::Numpad6) return Rml::Input::KeyIdentifier::KI_NUMPAD6;
            if (key == Keyboard::Key::Numpad7) return Rml::Input::KeyIdentifier::KI_NUMPAD7;
            if (key == Keyboard::Key::Numpad8) return Rml::Input::KeyIdentifier::KI_NUMPAD8;
            if (key == Keyboard::Key::Numpad9) return Rml::Input::KeyIdentifier::KI_NUMPAD9;

            if (key == Keyboard::Key::Enter) return Rml::Input::KeyIdentifier::KI_NUMPADENTER;
            if (key == Keyboard::Key::Multiply) return Rml::Input::KeyIdentifier::KI_MULTIPLY;
            if (key == Keyboard::Key::Divide) return Rml::Input::KeyIdentifier::KI_DIVIDE;
            //if (key == Keyboard::Key::Add) return Rml::Input::KeyIdentifier::KI_ADD;

            if (key == Keyboard::Key::Backspace) return Rml::Input::KeyIdentifier::KI_BACK;
            if (key == Keyboard::Key::Tab) return Rml::Input::KeyIdentifier::KI_TAB;
            if (key == Keyboard::Key::Divide) return Rml::Input::KeyIdentifier::KI_DIVIDE;
            if (key == Keyboard::Key::CapsLock) return Rml::Input::KeyIdentifier::KI_CAPITAL;
            if (key == Keyboard::Key::Escape) return Rml::Input::KeyIdentifier::KI_ESCAPE;
            if (key == Keyboard::Key::PageUp) return Rml::Input::KeyIdentifier::KI_PRIOR;
            if (key == Keyboard::Key::PageDown) return Rml::Input::KeyIdentifier::KI_NEXT;

            if (key == Keyboard::Key::Up) return Rml::Input::KeyIdentifier::KI_UP;
            if (key == Keyboard::Key::Down) return Rml::Input::KeyIdentifier::KI_DOWN;
            if (key == Keyboard::Key::Left) return Rml::Input::KeyIdentifier::KI_LEFT;
            if (key == Keyboard::Key::Right) return Rml::Input::KeyIdentifier::KI_RIGHT;
            if (key == Keyboard::Key::Delete) return Rml::Input::KeyIdentifier::KI_DELETE;
            if (key == Keyboard::Key::Insert) return Rml::Input::KeyIdentifier::KI_INSERT;

            if (key == Keyboard::Key::F1 ) return Rml::Input::KeyIdentifier::KI_F1 ;
            if (key == Keyboard::Key::F2 ) return Rml::Input::KeyIdentifier::KI_F2 ;
            if (key == Keyboard::Key::F3 ) return Rml::Input::KeyIdentifier::KI_F3 ;
            if (key == Keyboard::Key::F4 ) return Rml::Input::KeyIdentifier::KI_F4 ;
            if (key == Keyboard::Key::F5 ) return Rml::Input::KeyIdentifier::KI_F5 ;
            if (key == Keyboard::Key::F6 ) return Rml::Input::KeyIdentifier::KI_F6 ;
            if (key == Keyboard::Key::F7 ) return Rml::Input::KeyIdentifier::KI_F7 ;
            if (key == Keyboard::Key::F8 ) return Rml::Input::KeyIdentifier::KI_F8 ;
            if (key == Keyboard::Key::F9 ) return Rml::Input::KeyIdentifier::KI_F9 ;
            if (key == Keyboard::Key::F10) return Rml::Input::KeyIdentifier::KI_F10;
            if (key == Keyboard::Key::F11) return Rml::Input::KeyIdentifier::KI_F11;
            if (key == Keyboard::Key::F12) return Rml::Input::KeyIdentifier::KI_F12;

            if (key == Keyboard::Key::LShift) return Rml::Input::KeyIdentifier::KI_LSHIFT;
            if (key == Keyboard::Key::RShift) return Rml::Input::KeyIdentifier::KI_RSHIFT;

            if (key == Keyboard::Key::LControl) return Rml::Input::KeyIdentifier::KI_LCONTROL;
            if (key == Keyboard::Key::RControl) return Rml::Input::KeyIdentifier::KI_RCONTROL;

            return Rml::Input::KeyIdentifier::KI_UNKNOWN;
        }

    int getModifications(const int& modifications) {
        int mods = 0;
        if (modifications & GLFW_MOD_ALT)       mods |= Rml::Input::KeyModifier::KM_ALT;
        if (modifications & GLFW_MOD_CAPS_LOCK) mods |= Rml::Input::KeyModifier::KM_CAPSLOCK;
        if (modifications & GLFW_MOD_CONTROL)   mods |= Rml::Input::KeyModifier::KM_CTRL;
        if (modifications & GLFW_MOD_NUM_LOCK)  mods |= Rml::Input::KeyModifier::KM_NUMLOCK;
        if (modifications & GLFW_MOD_SHIFT)     mods |= Rml::Input::KeyModifier::KM_SHIFT;
        return mods;
    }

    int getModifications() {
        int mods = 0;
        if (keyButtons[(int)Keyboard::Key::LControl] || keyButtons[(int)Keyboard::Key::RControl]) mods |= Rml::Input::KeyModifier::KM_CTRL;
        if (keyButtons[(int)Keyboard::Key::LShift] || keyButtons[(int)Keyboard::Key::RShift]) mods |= Rml::Input::KeyModifier::KM_SHIFT;
        if (keyButtons[(int)Keyboard::Key::LAlt] || keyButtons[(int)Keyboard::Key::RAlt]) mods |= Rml::Input::KeyModifier::KM_ALT;
        if (keyButtons[(int)Keyboard::Key::CapsLock]) mods |= Rml::Input::KeyModifier::KM_CTRL;
        return mods;
    }

    /*
	class Window {
	public:
        Window() = default;
		~Window() {}

		void Create(const glm::ivec2& size, const char* title, const bool& vsync = false, const bool& fullscreen = false) {
			GLFWmonitor* mode = nullptr;

            if (fullscreen) mode = glfwGetPrimaryMonitor();            

			window = glfwCreateWindow(size.x, size.y, title, mode, nullptr);
			glfwMakeContextCurrent(window);

            // Remove when transition to vulkan
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
			if (!vsync) glfwSwapInterval(0);

            glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) { resized = true; });
			glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
                scrollX = xoffset; scrollY = yoffset;

                context->ProcessMouseWheel(-(float)scrollY, getModifications());
                });
			glfwSetCharCallback(window, [](GLFWwindow* window, uint32_t codepoint) {
                context->ProcessTextInput((char)codepoint);
                });
			glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                keyButtons[key] = action;

                if (action != GLFW_RELEASE) { context->ProcessKeyDown(convertToRmlKey((Keyboard::Key)key), getModifications(mods)); }
                else { context->ProcessKeyUp(convertToRmlKey((Keyboard::Key)key), getModifications(mods)); }
				});

            glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
                context->ProcessMouseMove((int)xpos, (int)ypos, getModifications());
            });
			glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
                mouseButtons[button] = action;

                if (action != GLFW_RELEASE) { context->ProcessMouseButtonDown(button, getModifications(mods)); }
                else { context->ProcessMouseButtonUp(button, getModifications(mods)); }
				});

           // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}

		void Destroy() const {
			glfwDestroyWindow(window);
		}

        float getContentScale() {
            float xscale, yscale;
            glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
            return xscale;
        }

		void display() const {
			resized = false;
            scrollY = 0.f;
            scrollX = 0.f;
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

        void setMaximized(const bool& maximized) {
            if (maximized) glfwMaximizeWindow(window);
            else glfwRestoreWindow(window);
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

        inline operator GLFWwindow* () { return window; }
        inline operator GLFWwindow* () const { return window; }

	private:
		GLFWwindow* window = nullptr;
	};
    */

struct WindowCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    bool startMaximized = false;
    bool Vsync = false;
    std::string appName;
    bool startFullscreen = false;
    bool decorated = true;
};

const char* getClipboard() { return glfwGetClipboardString(nullptr); }
void setClipboard(const std::string& string) { glfwSetClipboardString(nullptr, string.c_str()); }

class Window {
public:
    Window() = default;
    ~Window() {}

    void Create(const glm::ivec2& size, const char* title, const bool& fullscreen = false) {
        GLFWmonitor* mode = nullptr;

        if (fullscreen) mode = glfwGetPrimaryMonitor();

        glfwWindowHint(GLFW_RESIZABLE, false);
        //glfwWindowHint(GLFW_DECORATED, false);
        window = glfwCreateWindow(size.x, size.y, title, mode, nullptr);
        glfwSetWindowUserPointer(window, this);

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            scrollX = xoffset; scrollY = yoffset;
        
            context->ProcessMouseWheel(-(float)scrollY, getModifications());
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
            });
        glfwSetCharCallback(window, [](GLFWwindow* window, uint32_t codepoint) {
            context->ProcessTextInput((char)codepoint);
            ImGui_ImplGlfw_CharCallback(window, codepoint);
            });
        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            keyButtons[key] = action;
        
            if (action != GLFW_RELEASE) { context->ProcessKeyDown(convertToRmlKey((Keyboard::Key)key), getModifications(mods)); }
            else { context->ProcessKeyUp(convertToRmlKey((Keyboard::Key)key), getModifications(mods)); }

            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
            });
        
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            context->ProcessMouseMove((int)xpos, (int)ypos, getModifications());
            ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
            });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            mouseButtons[button] = action;
        
            if (action != GLFW_RELEASE) { context->ProcessMouseButtonDown(button, getModifications(mods)); }
            else { context->ProcessMouseButtonUp(button, getModifications(mods)); }
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
            });

        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void SetCursorPosCallback(const GLFWcursorposfun& callback) const {
        glfwSetCursorPosCallback(window, callback);
    }

    void SetFramebufferSizeCallback(const GLFWframebuffersizefun& callback) {
        glfwSetFramebufferSizeCallback(window, callback);
        //glfwSetWindowSize
    }

    void SetScrollCallback(const GLFWscrollfun& callback) {
        glfwSetScrollCallback(window, callback);
    }

    void SetCharCallback(const GLFWcharfun& callback) {
        glfwSetCharCallback(window, callback);
    }

    void SetMouseButtonCallback(const GLFWmousebuttonfun& callback) {
        glfwSetMouseButtonCallback(window, callback);
    }

    void SetKeyCallback(const GLFWkeyfun& callback) {
        glfwSetKeyCallback(window, callback);
    }

    void Destroy() const {
        glfwDestroyWindow(window);
    }

    float getContentScale() {
        float xscale, yscale;
        glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
        return xscale;
    }

    float getAspectRatio() {
        auto size = GetSize();
        return float((float)size.x / (float)size.y);
    }

    void display() const {
        scrollY = 0.f;
        scrollX = 0.f;
        memset(mouseButtons, GLFW_RELEASE, sizeof(mouseButtons));
        memset(keyButtons, GLFW_RELEASE, sizeof(keyButtons));
        glfwPollEvents();
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

    VkExtent2D GetExtent() const {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        return { (uint32_t)width, (uint32_t)height };
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

    void setCursorPos(const glm::ivec2& pos) {
        glfwSetCursorPos(window, pos.x, pos.y);
    }

    void setMaximized(const bool& maximized) {
        if (maximized) glfwMaximizeWindow(window);
        else glfwRestoreWindow(window);
    }

    void setPosition(const glm::ivec2& pos) {
        glfwSetWindowPos(window, pos.x, pos.y);
    }

    void setTitle(const std::string& title) {
        glfwSetWindowTitle(window, title.c_str());
    }

    void setSize(const glm::ivec2& size) {
        glfwSetWindowSize(window, size.x, size.y);
    }

    void setSizeLimits(const glm::ivec2& minSize, const glm::ivec2& maxSize) {
        glfwSetWindowSizeLimits(window, minSize.x, minSize.y, maxSize.x, maxSize.y);
    }

    void ShowMouse(const bool& show) {
        if (show) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
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

    inline operator GLFWwindow* () { return window; }
    inline operator GLFWwindow* () const { return window; }

private:
    GLFWwindow* window = nullptr;
};
    static Window window;

    namespace Keyboard {
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