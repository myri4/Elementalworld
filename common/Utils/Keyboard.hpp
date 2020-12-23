#pragma once

#include "../wclibs/Core.hpp"
#include <Windows.h>

namespace wc{
    namespace Keyboard {

	enum class Key {
        Unknown = -1, ///< Unhandled key
        A = 0,        ///< The A key
        B,            ///< The B key
        C,            ///< The C key
        D,            ///< The D key
        E,            ///< The E key
        F,            ///< The F key
        G,            ///< The G key
        H,            ///< The H key
        I,            ///< The I key
        J,            ///< The J key
        K,            ///< The K key
        L,            ///< The L key
        M,            ///< The M key
        N,            ///< The N key
        O,            ///< The O key
        P,            ///< The P key
        Q,            ///< The Q key
        R,            ///< The R key
        S,            ///< The S key
        T,            ///< The T key
        U,            ///< The U key
        V,            ///< The V key
        W,            ///< The W key
        X,            ///< The X key
        Y,            ///< The Y key
        Z,            ///< The Z key
        Num0,         ///< The 0 key
        Num1,         ///< The 1 key
        Num2,         ///< The 2 key
        Num3,         ///< The 3 key
        Num4,         ///< The 4 key
        Num5,         ///< The 5 key
        Num6,         ///< The 6 key
        Num7,         ///< The 7 key
        Num8,         ///< The 8 key
        Num9,         ///< The 9 key
        Escape,       ///< The Escape key
        LControl,     ///< The left Control key
        LShift,       ///< The left Shift key
        LAlt,         ///< The left Alt key
        LSystem,      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
        RControl,     ///< The right Control key
        RShift,       ///< The right Shift key
        RAlt,         ///< The right Alt key
        RSystem,      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
        Menu,         ///< The Menu key
        LBracket,     ///< The [ key
        RBracket,     ///< The ] key
        Semicolon,    ///< The ; key
        Comma,        ///< The , key
        Period,       ///< The . key
        Quote,        ///< The ' key
        Slash,        ///< The / key
        Backslash,    ///< The \ key
        Tilde,        ///< The ~ key
        Equal,        ///< The = key
        Hyphen,       ///< The - key (hyphen)
        Space,        ///< The Space key
        Enter,        ///< The Enter/Return keys
        Backspace,    ///< The Backspace key
        Tab,          ///< The Tabulation key
        PageUp,       ///< The Page up key
        PageDown,     ///< The Page down key
        End,          ///< The End key
        Home,         ///< The Home key
        Insert,       ///< The Insert key
        Delete,       ///< The Delete key
        Add,          ///< The + key
        Subtract,     ///< The - key (minus, usually from numpad)
        Multiply,     ///< The * key
        Divide,       ///< The / key
        Left,         ///< Left arrow
        Right,        ///< Right arrow
        Up,           ///< Up arrow
        Down,         ///< Down arrow
        Numpad0,      ///< The numpad 0 key
        Numpad1,      ///< The numpad 1 key
        Numpad2,      ///< The numpad 2 key
        Numpad3,      ///< The numpad 3 key
        Numpad4,      ///< The numpad 4 key
        Numpad5,      ///< The numpad 5 key
        Numpad6,      ///< The numpad 6 key
        Numpad7,      ///< The numpad 7 key
        Numpad8,      ///< The numpad 8 key
        Numpad9,      ///< The numpad 9 key
        F1 ,           ///< The F1 key
        F2 ,           ///< The F2 key
        F3 ,           ///< The F3 key
        F4 ,           ///< The F4 key
        F5 ,           ///< The F5 key
        F6 ,           ///< The F6 key
        F7 ,           ///< The F7 key
        F8 ,           ///< The F8 key
        F9 ,           ///< The F9 key
        F10,          ///< The F10 key
        F11,          ///< The F11 key
        F12,          ///< The F12 key
        F13,          ///< The F13 key
        F14,          ///< The F14 key
        F15,          ///< The F15 key
        Pause,        ///< The Pause key

        KeyCount,     ///< Keep last -- the total number of keyboard keys

        // Deprecated values:

        Dash = Hyphen,       ///<  Use Hyphen instead
        BackSpace = Backspace,    ///<  Use Backspace instead
        BackSlash = Backslash,    ///<  Use Backslash instead
        SemiColon = Semicolon,    ///<  Use Semicolon instead
        Return = Enter         ///<  Use Enter instead
	};

    bool isButtonPressed(Key key) {
#ifdef _WIN32
        if (key == Key::A) if (GetAsyncKeyState('A')) return true;
        if (key == Key::B) if (GetAsyncKeyState('B')) return true;
        if (key == Key::C) if (GetAsyncKeyState('C')) return true;
        if (key == Key::D) if (GetAsyncKeyState('D')) return true;
        if (key == Key::E) if (GetAsyncKeyState('E')) return true;
        if (key == Key::F) if (GetAsyncKeyState('F')) return true;
        if (key == Key::G) if (GetAsyncKeyState('G')) return true;
        if (key == Key::H) if (GetAsyncKeyState('H')) return true;
        if (key == Key::I) if (GetAsyncKeyState('I')) return true;
        if (key == Key::J) if (GetAsyncKeyState('J')) return true;
        if (key == Key::K) if (GetAsyncKeyState('K')) return true;
        if (key == Key::L) if (GetAsyncKeyState('L')) return true;
        if (key == Key::M) if (GetAsyncKeyState('M')) return true;
        if (key == Key::N) if (GetAsyncKeyState('N')) return true;
        if (key == Key::O) if (GetAsyncKeyState('O')) return true;
        if (key == Key::P) if (GetAsyncKeyState('P')) return true;
        if (key == Key::Q) if (GetAsyncKeyState('Q')) return true;
        if (key == Key::R) if (GetAsyncKeyState('R')) return true;
        if (key == Key::S) if (GetAsyncKeyState('S')) return true;
        if (key == Key::T) if (GetAsyncKeyState('T')) return true;
        if (key == Key::U) if (GetAsyncKeyState('U')) return true;
        if (key == Key::V) if (GetAsyncKeyState('V')) return true;
        if (key == Key::W) if (GetAsyncKeyState('W')) return true;
        if (key == Key::X) if (GetAsyncKeyState('X')) return true;
        if (key == Key::Y) if (GetAsyncKeyState('Y')) return true;
        if (key == Key::Z) if (GetAsyncKeyState('Z')) return true;
        
        if (key == Key::Num0) if (GetAsyncKeyState('0')) return true;
        if (key == Key::Num1) if (GetAsyncKeyState('1')) return true;
        if (key == Key::Num2) if (GetAsyncKeyState('2')) return true;
        if (key == Key::Num3) if (GetAsyncKeyState('3')) return true;
        if (key == Key::Num4) if (GetAsyncKeyState('4')) return true;
        if (key == Key::Num5) if (GetAsyncKeyState('5')) return true;
        if (key == Key::Num6) if (GetAsyncKeyState('6')) return true;
        if (key == Key::Num7) if (GetAsyncKeyState('7')) return true;
        if (key == Key::Num8) if (GetAsyncKeyState('8')) return true;
        if (key == Key::Num9) if (GetAsyncKeyState('9')) return true;

        if (key == Key::Escape) if (GetAsyncKeyState(VK_ESCAPE))     return true;
        if (key == Key::Menu) if (GetAsyncKeyState(VK_MENU))         return true;
        if (key == Key::Space)  if (GetAsyncKeyState(VK_SPACE))      return true;
        if (key == Key::SemiColon)  if (GetAsyncKeyState(';'))       return true;


        if (key == Key::LControl) if (GetAsyncKeyState(VK_LCONTROL)) return true;
        if (key == Key::LShift) if (GetAsyncKeyState(VK_LSHIFT))     return true;
        if (key == Key::LBracket) if (GetAsyncKeyState('['))         return true;

        if (key == Key::RControl) if (GetAsyncKeyState(VK_RCONTROL)) return true;
        if (key == Key::RShift) if (GetAsyncKeyState(VK_RSHIFT))     return true;
        if (key == Key::RBracket) if (GetAsyncKeyState(']'))         return true;

                   
        if (key == Key::Numpad0) if (GetAsyncKeyState(VK_NUMPAD0)) return true;
        if (key == Key::Numpad1) if (GetAsyncKeyState(VK_NUMPAD1)) return true;
        if (key == Key::Numpad2) if (GetAsyncKeyState(VK_NUMPAD2)) return true;
        if (key == Key::Numpad3) if (GetAsyncKeyState(VK_NUMPAD3)) return true;
        if (key == Key::Numpad4) if (GetAsyncKeyState(VK_NUMPAD4)) return true;
        if (key == Key::Numpad5) if (GetAsyncKeyState(VK_NUMPAD5)) return true;
        if (key == Key::Numpad6) if (GetAsyncKeyState(VK_NUMPAD6)) return true;
        if (key == Key::Numpad7) if (GetAsyncKeyState(VK_NUMPAD7)) return true;
        if (key == Key::Numpad8) if (GetAsyncKeyState(VK_NUMPAD8)) return true;
        if (key == Key::Numpad9) if (GetAsyncKeyState(VK_NUMPAD9)) return true;
           
        if (key == Key::F1 ) if (GetAsyncKeyState(VK_F1))  return true;
        if (key == Key::F2 ) if (GetAsyncKeyState(VK_F2))  return true;
        if (key == Key::F3 ) if (GetAsyncKeyState(VK_F3))  return true;
        if (key == Key::F4 ) if (GetAsyncKeyState(VK_F4))  return true;
        if (key == Key::F5 ) if (GetAsyncKeyState(VK_F5))  return true;
        if (key == Key::F6 ) if (GetAsyncKeyState(VK_F6))  return true;
        if (key == Key::F7 ) if (GetAsyncKeyState(VK_F7))  return true;
        if (key == Key::F8 ) if (GetAsyncKeyState(VK_F8))  return true;
        if (key == Key::F9 ) if (GetAsyncKeyState(VK_F9))  return true;
        if (key == Key::F10) if (GetAsyncKeyState(VK_F10)) return true;
        if (key == Key::F11) if (GetAsyncKeyState(VK_F11)) return true;
        if (key == Key::F12) if (GetAsyncKeyState(VK_F12)) return true;
        if (key == Key::F13) if (GetAsyncKeyState(VK_F13)) return true;
        if (key == Key::F14) if (GetAsyncKeyState(VK_F14)) return true;
        if (key == Key::F15) if (GetAsyncKeyState(VK_F15)) return true;
           
        if (key == Key::Up) if (GetAsyncKeyState(VK_UP)) return true;
        if (key == Key::Down) if (GetAsyncKeyState(VK_DOWN)) return true;

        if (key == Key::Left) if (GetAsyncKeyState(VK_LEFT)) return true;
        if (key == Key::Right) if (GetAsyncKeyState(VK_RIGHT)) return true;


#endif // _WIN32

        return false;
    }
   }
}