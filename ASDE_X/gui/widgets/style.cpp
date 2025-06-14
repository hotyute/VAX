// --- START OF FILE style.cpp (Updated with font_keys.h) ---
#include "style.h"
#include "font_keys.h" // Include your new header

// No longer define const std::string FONT_KEY_... here

Style Style::DefaultButton() {
    Style s;
    s.fontKey = FONT_KEY_TOP_BUTTON; // Use the constant
    s.padding = { 4, 8, 4, 8 };
    s.foregroundColor = Color(0.805f, 0.805f, 0.805f);
    s.backgroundColor = Color(0.227f, 0.227f, 0.227f);
    s.borderColor = Color(0.325f, 0.325f, 0.325f);
    return s;
}

Style Style::DefaultLabel() {
    Style s;
    s.fontKey = FONT_KEY_UI_LABEL; // Use the constant
    s.foregroundColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    s.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    s.hasBorder = false;
    return s;
}

Style Style::DefaultInputField() {
    Style s;
    s.fontKey = FONT_KEY_TOP_BUTTON; // Or a specific input font like FONT_KEY_UI_LABEL
    s.backgroundColor = { 1.0f, 1.0f, 1.0f, 0.8f };
    s.foregroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    s.borderColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    s.padding = { 3, 3, 3, 3 };
    return s;
}

Style Style::DefaultWindow() {
    Style s;
    s.fontKey = FONT_KEY_TITLE; // Use the constant
    s.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.6f };
    s.borderColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    s.borderWidth = 1.0f;
    return s;
}

Style Style::DefaultDisplayBox() { // For TextBoxWidget
    Style s = Style::DefaultInputField();
    s.fontKey = FONT_KEY_UI_LABEL; // Use the constant, or a dedicated one
    return s;
}
// --- END OF FILE style.cpp ---