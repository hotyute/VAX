#pragma once

#include <string>
#include <vector>
#include <windows.h> // For HFONT, COLORREF (or define your own Color struct)

// Forward declarations
class Widget;

struct Point {
    int x = 0;
    int y = 0;
};

struct Size {
    int width = 0;
    int height = 0;
};

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool Contains(const Point& p) const {
        return p.x >= x && p.x < (x + width) &&
            p.y >= y && p.y < (y + height);
    }
};

struct Color {
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    Color() = default;
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};

struct Padding {
    int top = 0, right = 0, bottom = 0, left = 0;
};

struct Margin {
    int top = 0, right = 0, bottom = 0, left = 0;
};

enum class MouseButton {
    Left, Middle, Right, None
};

enum class KeyEventType {
    Press, Release, Char
};

enum class MouseEventType {
    Move, Press, Release, Scroll, Enter, Leave
};

// --- EXPANDED KeyCode ENUM ---
enum class KeyCode {
    Unknown,

    // Control Keys
    Enter, Escape, Backspace, Tab, Space,
    CapsLock, ScrollLock, NumLock,
    PrintScreen, PauseBreak, Insert, Delete, Home, End, PageUp, PageDown,

    // Arrow Keys
    LeftArrow, RightArrow, UpArrow, DownArrow,

    // Modifier Keys
    LShift, RShift, Shift,         // Shift can be used if side doesn't matter
    LControl, RControl, Control,   // Control can be used if side doesn't matter
    LAlt, RAlt, Alt,               // Alt (Menu) can be used if side doesn't matter
    LWin, RWin,                    // Windows/Super/Command keys

    // Alphanumeric Keys
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, // Top row numbers

    // Numpad Keys
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    NumpadMultiply, NumpadAdd, NumpadSeparator, NumpadSubtract, NumpadDecimal, NumpadDivide,
    NumpadEnter, // Often distinct from main Enter

    // Function Keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,

    // Punctuation & Symbol Keys (US Standard Layout - may vary by region)
    Oem1,          // ';:' for US
    OemPlus,       // '+' any country
    OemComma,      // ',' any country
    OemMinus,      // '-' any country
    OemPeriod,     // '.' any country
    Oem2,          // '/?' for US
    Oem3,          // '`~' for US
    Oem4,          // '[' for US
    Oem5,          // '\|' for US
    Oem6,          // ']' for US
    Oem7,          // ''"' for US
    Oem8,          // Miscellaneous, varies
    Oem102,        // '<>' or '\|' on RT 102-key keyboard

    // Media Keys (Examples, less common for typical UI but can be useful)
    VolumeMute, VolumeDown, VolumeUp,
    MediaNextTrack, MediaPrevTrack, MediaStop, MediaPlayPause,
    LaunchMail, LaunchMediaSelect, LaunchApp1, LaunchApp2,

    // Browser Keys (Examples)
    BrowserBack, BrowserForward, BrowserRefresh, BrowserStop,
    BrowserSearch, BrowserFavorites, BrowserHome
};


struct MouseEvent {
    MouseEventType type;
    Point position;
    Point screen_position;
    MouseButton button = MouseButton::None;
    int scrollDelta = 0;
    // Modifiers:
    bool ctrlPressed = false;
    bool shiftPressed = false;
    bool altPressed = false;
};

struct KeyboardEvent {
    KeyEventType type;
    KeyCode keyCode = KeyCode::Unknown;
    wchar_t character = 0; // For Char events
    // Modifiers:
    bool ctrlPressed = false;
    bool shiftPressed = false;
    bool altPressed = false;
};

struct UIEvent {
    enum class Type { Mouse, Keyboard, Focus, Generic } eventType = Type::Generic;
    MouseEvent mouse;
    KeyboardEvent keyboard;
    Widget* targetWidget = nullptr;
    bool handled = false;

    UIEvent(const MouseEvent& me) : eventType(Type::Mouse), mouse(me) {}
    UIEvent(const KeyboardEvent& ke) : eventType(Type::Keyboard), keyboard(ke) {}
    UIEvent(Type t = Type::Generic) : eventType(t) {}
};

// Helper to convert Win32 VK to KeyCode
KeyCode VKToKeyCode(WPARAM vk_code, LPARAM lParam = 0); // lParam can help distinguish L/R Shift/Ctrl/Alt