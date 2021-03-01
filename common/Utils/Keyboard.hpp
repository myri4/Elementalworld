#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../wc/Core.hpp"
#include <Windows.h>

namespace wc {
    namespace Keyboard {

        enum class Key : int8_t {
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
            F1,           ///< The F1 key
            F2,           ///< The F2 key
            F3,           ///< The F3 key
            F4,           ///< The F4 key
            F5,           ///< The F5 key
            F6,           ///< The F6 key
            F7,           ///< The F7 key
            F8,           ///< The F8 key
            F9,           ///< The F9 key
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

        bool isKeyPressed(Key key)
        {
            int vkey = 0;
            switch (key)
            {
            default:              vkey = 0;             break;
            case Key::A:          vkey = 'A';           break;
            case Key::B:          vkey = 'B';           break;
            case Key::C:          vkey = 'C';           break;
            case Key::D:          vkey = 'D';           break;
            case Key::E:          vkey = 'E';           break;
            case Key::F:          vkey = 'F';           break;
            case Key::G:          vkey = 'G';           break;
            case Key::H:          vkey = 'H';           break;
            case Key::I:          vkey = 'I';           break;
            case Key::J:          vkey = 'J';           break;
            case Key::K:          vkey = 'K';           break;
            case Key::L:          vkey = 'L';           break;
            case Key::M:          vkey = 'M';           break;
            case Key::N:          vkey = 'N';           break;
            case Key::O:          vkey = 'O';           break;
            case Key::P:          vkey = 'P';           break;
            case Key::Q:          vkey = 'Q';           break;
            case Key::R:          vkey = 'R';           break;
            case Key::S:          vkey = 'S';           break;
            case Key::T:          vkey = 'T';           break;
            case Key::U:          vkey = 'U';           break;
            case Key::V:          vkey = 'V';           break;
            case Key::W:          vkey = 'W';           break;
            case Key::X:          vkey = 'X';           break;
            case Key::Y:          vkey = 'Y';           break;
            case Key::Z:          vkey = 'Z';           break;
            case Key::Num0:       vkey = '0';           break;
            case Key::Num1:       vkey = '1';           break;
            case Key::Num2:       vkey = '2';           break;
            case Key::Num3:       vkey = '3';           break;
            case Key::Num4:       vkey = '4';           break;
            case Key::Num5:       vkey = '5';           break;
            case Key::Num6:       vkey = '6';           break;
            case Key::Num7:       vkey = '7';           break;
            case Key::Num8:       vkey = '8';           break;
            case Key::Num9:       vkey = '9';           break;
            case Key::Escape:     vkey = VK_ESCAPE;     break;
            case Key::LControl:   vkey = VK_LCONTROL;   break;
            case Key::LShift:     vkey = VK_LSHIFT;     break;
            case Key::LAlt:       vkey = VK_LMENU;      break;
            case Key::LSystem:    vkey = VK_LWIN;       break;
            case Key::RControl:   vkey = VK_RCONTROL;   break;
            case Key::RShift:     vkey = VK_RSHIFT;     break;
            case Key::RAlt:       vkey = VK_RMENU;      break;
            case Key::RSystem:    vkey = VK_RWIN;       break;
            case Key::Menu:       vkey = VK_APPS;       break;
            case Key::LBracket:   vkey = VK_OEM_4;      break;
            case Key::RBracket:   vkey = VK_OEM_6;      break;
            case Key::Semicolon:  vkey = VK_OEM_1;      break;
            case Key::Comma:      vkey = VK_OEM_COMMA;  break;
            case Key::Period:     vkey = VK_OEM_PERIOD; break;
            case Key::Quote:      vkey = VK_OEM_7;      break;
            case Key::Slash:      vkey = VK_OEM_2;      break;
            case Key::Backslash:  vkey = VK_OEM_5;      break;
            case Key::Tilde:      vkey = VK_OEM_3;      break;
            case Key::Equal:      vkey = VK_OEM_PLUS;   break;
            case Key::Hyphen:     vkey = VK_OEM_MINUS;  break;
            case Key::Space:      vkey = VK_SPACE;      break;
            case Key::Enter:      vkey = VK_RETURN;     break;
            case Key::Backspace:  vkey = VK_BACK;       break;
            case Key::Tab:        vkey = VK_TAB;        break;
            case Key::PageUp:     vkey = VK_PRIOR;      break;
            case Key::PageDown:   vkey = VK_NEXT;       break;
            case Key::End:        vkey = VK_END;        break;
            case Key::Home:       vkey = VK_HOME;       break;
            case Key::Insert:     vkey = VK_INSERT;     break;
            case Key::Delete:     vkey = VK_DELETE;     break;
            case Key::Add:        vkey = VK_ADD;        break;
            case Key::Subtract:   vkey = VK_SUBTRACT;   break;
            case Key::Multiply:   vkey = VK_MULTIPLY;   break;
            case Key::Divide:     vkey = VK_DIVIDE;     break;
            case Key::Left:       vkey = VK_LEFT;       break;
            case Key::Right:      vkey = VK_RIGHT;      break;
            case Key::Up:         vkey = VK_UP;         break;
            case Key::Down:       vkey = VK_DOWN;       break;
            case Key::Numpad0:    vkey = VK_NUMPAD0;    break;
            case Key::Numpad1:    vkey = VK_NUMPAD1;    break;
            case Key::Numpad2:    vkey = VK_NUMPAD2;    break;
            case Key::Numpad3:    vkey = VK_NUMPAD3;    break;
            case Key::Numpad4:    vkey = VK_NUMPAD4;    break;
            case Key::Numpad5:    vkey = VK_NUMPAD5;    break;
            case Key::Numpad6:    vkey = VK_NUMPAD6;    break;
            case Key::Numpad7:    vkey = VK_NUMPAD7;    break;
            case Key::Numpad8:    vkey = VK_NUMPAD8;    break;
            case Key::Numpad9:    vkey = VK_NUMPAD9;    break;
            case Key::F1:         vkey = VK_F1;         break;
            case Key::F2:         vkey = VK_F2;         break;
            case Key::F3:         vkey = VK_F3;         break;
            case Key::F4:         vkey = VK_F4;         break;
            case Key::F5:         vkey = VK_F5;         break;
            case Key::F6:         vkey = VK_F6;         break;
            case Key::F7:         vkey = VK_F7;         break;
            case Key::F8:         vkey = VK_F8;         break;
            case Key::F9:         vkey = VK_F9;         break;
            case Key::F10:        vkey = VK_F10;        break;
            case Key::F11:        vkey = VK_F11;        break;
            case Key::F12:        vkey = VK_F12;        break;
            case Key::F13:        vkey = VK_F13;        break;
            case Key::F14:        vkey = VK_F14;        break;
            case Key::F15:        vkey = VK_F15;        break;
            case Key::Pause:      vkey = VK_PAUSE;      break;
            }
            return (GetAsyncKeyState(vkey) & 0x8000) != 0;
        }

    }
}
#endif