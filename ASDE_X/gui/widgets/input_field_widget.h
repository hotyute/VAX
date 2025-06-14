#pragma once
#include "widget.h"
#include <string>
#include <chrono> // For cursor blink timing
#include "ui_common.h"

class InputFieldWidget : public Widget {
public:
    std::string text;
    std::string placeholderText;
    int cursorPos = 0; // Character position
    int scrollOffset = 0; // For horizontal scrolling of text if it exceeds width

    bool isPassword = false;
    bool numbersOnly = false;
    bool forceCaps = false;
    int maxLength = 0; // 0 for unlimited

    std::function<void(InputFieldWidget*, const std::string&)> OnTextChanged;
    std::function<void(InputFieldWidget*)> OnEnterPressed;

    InputFieldWidget(const std::string& initialText = "");

    void SetText(const std::string& newText);
    const std::string& GetText() const { return text; }

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;
    void OnFocusGained() override;
    void OnFocusLost() override;
    void Update(float deltaTime) override;


private:
    std::string GetDisplayText() const;
    void MoveCursor(int delta);
    void InsertChar(wchar_t ch);
    void DeleteCharBackward();
    void DeleteCharForward();
    void EnsureCursorVisible(UIRenderer& renderer);
    int GetCharPixelOffset(UIRenderer& renderer, int charIndex); // Gets pixel offset of char at index

    bool showCursor = false;
    std::chrono::steady_clock::time_point lastBlinkTime;
    const std::chrono::milliseconds blinkInterval = std::chrono::milliseconds(500);
};