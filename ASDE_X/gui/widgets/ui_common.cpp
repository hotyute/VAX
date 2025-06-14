#include "ui_common.h"
#include <windows.h> // For VK_ constants

KeyCode VKToKeyCode(WPARAM vk_code, LPARAM lParam) {
    // For distinguishing left/right modifier keys
    // The high bit of the
    // LParam's extended-key flag indicates whether the key is an extended key.
    // For example, the right-hand ALT and CTRL keys on an enhanced keyboard.
    // See: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
    bool isExtendedKey = (lParam & 0x01000000) != 0;

    switch (vk_code) {
        // Control Keys
    case VK_RETURN:   return KeyCode::Enter;
    case VK_ESCAPE:   return KeyCode::Escape;
    case VK_BACK:     return KeyCode::Backspace;
    case VK_TAB:      return KeyCode::Tab;
    case VK_SPACE:    return KeyCode::Space;
    case VK_CAPITAL:  return KeyCode::CapsLock;
    case VK_SCROLL:   return KeyCode::ScrollLock;
    case VK_NUMLOCK:  return KeyCode::NumLock;
    case VK_PRINT:    // VK_SNAPSHOT for PrintScreen
    case VK_SNAPSHOT: return KeyCode::PrintScreen;
    case VK_PAUSE:    return KeyCode::PauseBreak;
    case VK_INSERT:   return KeyCode::Insert;
    case VK_DELETE:   return KeyCode::Delete;
    case VK_HOME:     return KeyCode::Home;
    case VK_END:      return KeyCode::End;
    case VK_PRIOR:    return KeyCode::PageUp;   // Page Up
    case VK_NEXT:     return KeyCode::PageDown; // Page Down

        // Arrow Keys
    case VK_LEFT:     return KeyCode::LeftArrow;
    case VK_RIGHT:    return KeyCode::RightArrow;
    case VK_UP:       return KeyCode::UpArrow;
    case VK_DOWN:     return KeyCode::DownArrow;

        // Modifier Keys
    case VK_SHIFT:    // This is general. Use LSHIFT/RSHIFT for specifics.
        // To distinguish L/R shift, you might need to check GetAsyncKeyState or lParam on WM_KEYDOWN/UP
        // For simplicity, mapping general SHIFT to KeyCode::Shift here.
        // If you get separate LSHIFT/RSHIFT events, handle them.
        // Windows often sends VK_SHIFT for both, then uses lParam or GetKeyState for specifics.
        // If isExtendedKey can be reliably used here for Shift, it might help.
        // However, GetAsyncKeyState(VK_LSHIFT) & 0x8000 is more reliable.
        return KeyCode::Shift;
    case VK_LSHIFT:   return KeyCode::LShift;
    case VK_RSHIFT:   return KeyCode::RShift;
    case VK_CONTROL:  // General Control
        return isExtendedKey ? KeyCode::RControl : KeyCode::LControl; // lParam helps distinguish
    case VK_LCONTROL: return KeyCode::LControl;
    case VK_RCONTROL: return KeyCode::RControl;
    case VK_MENU:     // General Alt
        return isExtendedKey ? KeyCode::RAlt : KeyCode::LAlt; // lParam helps distinguish
    case VK_LMENU:    return KeyCode::LAlt;
    case VK_RMENU:    return KeyCode::RAlt;
    case VK_LWIN:     return KeyCode::LWin;
    case VK_RWIN:     return KeyCode::RWin;

        // Alphanumeric Keys
    case 'A': return KeyCode::A; case 'B': return KeyCode::B; case 'C': return KeyCode::C;
    case 'D': return KeyCode::D; case 'E': return KeyCode::E; case 'F': return KeyCode::F;
    case 'G': return KeyCode::G; case 'H': return KeyCode::H; case 'I': return KeyCode::I;
    case 'J': return KeyCode::J; case 'K': return KeyCode::K; case 'L': return KeyCode::L;
    case 'M': return KeyCode::M; case 'N': return KeyCode::N; case 'O': return KeyCode::O;
    case 'P': return KeyCode::P; case 'Q': return KeyCode::Q; case 'R': return KeyCode::R;
    case 'S': return KeyCode::S; case 'T': return KeyCode::T; case 'U': return KeyCode::U;
    case 'V': return KeyCode::V; case 'W': return KeyCode::W; case 'X': return KeyCode::X;
    case 'Y': return KeyCode::Y; case 'Z': return KeyCode::Z;
    case '0': return KeyCode::Num0; case '1': return KeyCode::Num1; case '2': return KeyCode::Num2;
    case '3': return KeyCode::Num3; case '4': return KeyCode::Num4; case '5': return KeyCode::Num5;
    case '6': return KeyCode::Num6; case '7': return KeyCode::Num7; case '8': return KeyCode::Num8;
    case '9': return KeyCode::Num9;

        // Numpad Keys
    case VK_NUMPAD0:  return KeyCode::Numpad0; case VK_NUMPAD1:  return KeyCode::Numpad1;
    case VK_NUMPAD2:  return KeyCode::Numpad2; case VK_NUMPAD3:  return KeyCode::Numpad3;
    case VK_NUMPAD4:  return KeyCode::Numpad4; case VK_NUMPAD5:  return KeyCode::Numpad5;
    case VK_NUMPAD6:  return KeyCode::Numpad6; case VK_NUMPAD7:  return KeyCode::Numpad7;
    case VK_NUMPAD8:  return KeyCode::Numpad8; case VK_NUMPAD9:  return KeyCode::Numpad9;
    case VK_MULTIPLY: return KeyCode::NumpadMultiply;
    case VK_ADD:      return KeyCode::NumpadAdd;
    case VK_SEPARATOR:return KeyCode::NumpadSeparator; // Usually comma or period on numpad
    case VK_SUBTRACT: return KeyCode::NumpadSubtract;
    case VK_DECIMAL:  return KeyCode::NumpadDecimal;
    case VK_DIVIDE:   return KeyCode::NumpadDivide;
        // Numpad Enter is often the same as VK_RETURN. If it has a distinct VK_ code on some systems, add it.
        // Some systems might send VK_RETURN with numlock on for numpad enter.

        // Function Keys
    case VK_F1: return KeyCode::F1; case VK_F2: return KeyCode::F2; case VK_F3: return KeyCode::F3;
    case VK_F4: return KeyCode::F4; case VK_F5: return KeyCode::F5; case VK_F6: return KeyCode::F6;
    case VK_F7: return KeyCode::F7; case VK_F8: return KeyCode::F8; case VK_F9: return KeyCode::F9;
    case VK_F10: return KeyCode::F10; case VK_F11: return KeyCode::F11; case VK_F12: return KeyCode::F12;
    case VK_F13: return KeyCode::F13; case VK_F14: return KeyCode::F14; case VK_F15: return KeyCode::F15;
    case VK_F16: return KeyCode::F16; case VK_F17: return KeyCode::F17; case VK_F18: return KeyCode::F18;
    case VK_F19: return KeyCode::F19; case VK_F20: return KeyCode::F20; case VK_F21: return KeyCode::F21;
    case VK_F22: return KeyCode::F22; case VK_F23: return KeyCode::F23; case VK_F24: return KeyCode::F24;

        // Punctuation & Symbol Keys
    case VK_OEM_1:      return KeyCode::Oem1;      // ';:' for US
    case VK_OEM_PLUS:   return KeyCode::OemPlus;
    case VK_OEM_COMMA:  return KeyCode::OemComma;
    case VK_OEM_MINUS:  return KeyCode::OemMinus;
    case VK_OEM_PERIOD: return KeyCode::OemPeriod;
    case VK_OEM_2:      return KeyCode::Oem2;      // '/?' for US
    case VK_OEM_3:      return KeyCode::Oem3;      // '`~' for US
    case VK_OEM_4:      return KeyCode::Oem4;      // '[' for US
    case VK_OEM_5:      return KeyCode::Oem5;      // '\|' for US
    case VK_OEM_6:      return KeyCode::Oem6;      // ']' for US
    case VK_OEM_7:      return KeyCode::Oem7;      // ''"' for US
    case VK_OEM_8:      return KeyCode::Oem8;
    case VK_OEM_102:    return KeyCode::Oem102;    // '<>' or '\|'

        // Media Keys
    case VK_VOLUME_MUTE:        return KeyCode::VolumeMute;
    case VK_VOLUME_DOWN:        return KeyCode::VolumeDown;
    case VK_VOLUME_UP:          return KeyCode::VolumeUp;
    case VK_MEDIA_NEXT_TRACK:   return KeyCode::MediaNextTrack;
    case VK_MEDIA_PREV_TRACK:   return KeyCode::MediaPrevTrack;
    case VK_MEDIA_STOP:         return KeyCode::MediaStop;
    case VK_MEDIA_PLAY_PAUSE:   return KeyCode::MediaPlayPause;
    case VK_LAUNCH_MAIL:        return KeyCode::LaunchMail;
    case VK_LAUNCH_MEDIA_SELECT:return KeyCode::LaunchMediaSelect;
    case VK_LAUNCH_APP1:        return KeyCode::LaunchApp1;
    case VK_LAUNCH_APP2:        return KeyCode::LaunchApp2;

        // Browser Keys
    case VK_BROWSER_BACK:       return KeyCode::BrowserBack;
    case VK_BROWSER_FORWARD:    return KeyCode::BrowserForward;
    case VK_BROWSER_REFRESH:    return KeyCode::BrowserRefresh;
    case VK_BROWSER_STOP:       return KeyCode::BrowserStop;
    case VK_BROWSER_SEARCH:     return KeyCode::BrowserSearch;
    case VK_BROWSER_FAVORITES:  return KeyCode::BrowserFavorites;
    case VK_BROWSER_HOME:       return KeyCode::BrowserHome;

    default: return KeyCode::Unknown;
    }
}