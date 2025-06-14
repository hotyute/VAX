#define NOMINMAX
#include <windows.h>

#include "input_field_widget.h"
#include "ui_renderer.h"
#include "ui_manager.h" // For UIManager::Instance().GetRenderer() if needed for measure
#include "style.h"    // For Style::DefaultInputField()
#include "../../tools.h"    // For s2ws
#include "font_manager.h"
#include <algorithm>  // For std::remove

InputFieldWidget::InputFieldWidget(const std::string& initialText)
    : Widget(), text(initialText) {
    style = Style::DefaultInputField();
    focusable = true;
    cursorPos = text.length();
    lastBlinkTime = std::chrono::steady_clock::now();
}

void InputFieldWidget::SetText(const std::string& newText) {
    if (text != newText) {
        text = newText;
        cursorPos = std::min((int)text.length(), cursorPos);
        scrollOffset = 0; // Reset scroll on new text, or make smarter
        if (UIManager::Instance().GetRenderer()) {
            EnsureCursorVisible(*UIManager::Instance().GetRenderer());
        }
        MarkDirty();
        if (OnTextChanged) {
            OnTextChanged(this, text);
        }
    }
}

std::string InputFieldWidget::GetDisplayText() const {
    if (isPassword) {
        return std::string(text.length(), '*');
    }
    return text;
}

// input_field_widget.cpp
void InputFieldWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    // Base Widget::Draw handles background/border modulated by inheritedAlpha
    Widget::Draw(renderer, inheritedAlpha);

    Rect screenBounds = GetScreenBounds();
    Rect clientScreenBounds = {
        screenBounds.x + style.padding.left,
        screenBounds.y + style.padding.top,
        screenBounds.width - (style.padding.left + style.padding.right),
        screenBounds.height - (style.padding.top + style.padding.bottom)
    };

    if (clientScreenBounds.width <= 0 || clientScreenBounds.height <= 0) {
        dirty = false;
        return;
    }

    renderer.PushClipRect(clientScreenBounds);

    std::string displayText = GetDisplayText();
    Point textOrigin = { clientScreenBounds.x - scrollOffset, clientScreenBounds.y };

    Color finalFgColor = style.foregroundColor;
    finalFgColor.a *= inheritedAlpha;

    if (finalFgColor.a > 0.001f) { // Only draw text elements if not fully transparent
        if (text.empty() && !placeholderText.empty() && !hasFocus) {
            displayText = placeholderText;
            Color placeholderColor = finalFgColor; // Use modulated foreground
            placeholderColor.a *= 0.5f; // Further dim for placeholder
            if (placeholderColor.a > 0.001f) {
                renderer.DrawText(displayText, textOrigin, style.fontKey, placeholderColor);
            }
        }
        else {
            renderer.DrawText(displayText, textOrigin, style.fontKey, finalFgColor);

            if (hasFocus && showCursor) {
                int cursorPixelX = GetCharPixelOffset(renderer, cursorPos);
                Point p1 = { textOrigin.x + cursorPixelX, textOrigin.y };
                Size textMetrics = renderer.GetTextExtent("M", style.fontKey); // Height for cursor
                Point p2 = { textOrigin.x + cursorPixelX, textOrigin.y + textMetrics.height };
                // Cursor color should also respect inheritedAlpha
                renderer.DrawLine(p1, p2, finalFgColor, 1.0f);
            }
        }
    }

    renderer.PopClipRect();
    dirty = false;
}

Size InputFieldWidget::Measure() {
    Size textSize = { 0, 16 }; // Default height
    if (!style.fontKey.empty()) {
        // Measure with a typical character for height
        textSize = FontManager::Instance().GetTextExtent("Mg", style.fontKey);
    }

    preferredSize = {
        minSize.width > 0 ? minSize.width : 100, // Default preferred width if not set by layout
        textSize.height + style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0)
    };

    // Apply min/max size constraints
    preferredSize.width = std::max(minSize.width, std::min(preferredSize.width, maxSize.width));
    preferredSize.height = std::max(minSize.height, std::min(preferredSize.height, maxSize.height));

    return preferredSize;
}

void InputFieldWidget::HandleEvent(UIEvent& event) {

    if (!visible || !enabled) {
        event.handled = false;
        return;
    }

    // Check for a click to gain focus
    if (event.eventType == UIEvent::Type::Mouse && event.mouse.type == MouseEventType::Press) {
        Rect localBounds = { 0,0, bounds.width, bounds.height };

        if (localBounds.Contains(event.mouse.position)) {
            RequestFocus(); // This is the key call to become the active input field

            // Set cursor position based on click
            Rect clientRelativeBounds = GetClientBounds();
            int clickX_in_client_area = event.mouse.position.x - clientRelativeBounds.x;
            int clickX = clickX_in_client_area + scrollOffset;

            int newCursorPos = 0;
            int currentWidth = 0;
            std::string currentText = GetDisplayText();
            for (size_t i = 0; i < currentText.length(); ++i) {
                Size charSize = UIManager::Instance().GetRenderer()->GetTextExtent(currentText.substr(i, 1), style.fontKey);
                if (clickX < currentWidth + charSize.width / 2) {
                    break;
                }
                currentWidth += charSize.width;
                newCursorPos++;
            }
            cursorPos = newCursorPos;
            if (UIManager::Instance().GetRenderer()) {
                EnsureCursorVisible(*UIManager::Instance().GetRenderer());
            }
            MarkDirty();
            event.handled = true; // The click has been fully handled
            return; // Don't process keyboard events in the same go
        }
    }

    // Handle keyboard events only if we have focus
    if (hasFocus && event.eventType == UIEvent::Type::Keyboard) {
        bool textChanged = false; // Flag to call OnTextChanged once if needed
        if (event.keyboard.type == KeyEventType::Char) {
            // Ensure backspace (ASCII 8) or delete (ASCII 127) don't sneak in here if system sends them as char
            if (event.keyboard.character != 0 && // Ensure it's not a null char
                event.keyboard.character != L'\b' && // ASCII Backspace
                event.keyboard.character != L'\x7F' && // ASCII Delete
                event.keyboard.character != L'\r' &&   // Carriage Return (Enter)
                event.keyboard.character != L'\n' &&   // Line Feed (Enter)
                event.keyboard.character != L'\t') {   // Tab
                InsertChar(event.keyboard.character);
                textChanged = true;
            }
            else {
                //
            }
        }
        else if (event.keyboard.type == KeyEventType::Press) {
            switch (event.keyboard.keyCode) {
            case KeyCode::Backspace:
                DeleteCharBackward();
                textChanged = true;
                break;
            case KeyCode::Delete:
                DeleteCharForward();
                textChanged = true;
                break;
            case KeyCode::LeftArrow:
                MoveCursor(-1);
                break;
            case KeyCode::RightArrow:
                MoveCursor(1);
                break;
            case KeyCode::Home:
                cursorPos = 0;
                if (UIManager::Instance().GetRenderer()) EnsureCursorVisible(*UIManager::Instance().GetRenderer());
                MarkDirty(false);
                break;
            case KeyCode::End:
                cursorPos = text.length();
                if (UIManager::Instance().GetRenderer()) EnsureCursorVisible(*UIManager::Instance().GetRenderer());
                MarkDirty(false);
                break;
            case KeyCode::Enter:
                if (OnEnterPressed) OnEnterPressed(this);
                break;
            default:
                // Potentially handle other non-char keys if needed, or do nothing
                break;
            }
        }

        if (textChanged && OnTextChanged) {
            OnTextChanged(this, text);
        }
        event.handled = true; // Input field consumes keyboard events when focused
    }

    // Call base handler for other things like OnClick callbacks if needed
    if (!event.handled) {
        Widget::HandleEvent(event);
    }
}


void InputFieldWidget::Update(float deltaTime) {
    Widget::Update(deltaTime);
    if (hasFocus) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlinkTime) > blinkInterval) {
            showCursor = !showCursor;
            lastBlinkTime = now;
            MarkDirty(false); // Only this widget needs redraw for cursor
        }
    }
}


void InputFieldWidget::OnFocusGained() {
    Widget::OnFocusGained(); // This sets this->hasFocus = true via base class
    showCursor = true;
    lastBlinkTime = std::chrono::steady_clock::now();
    MarkDirty(false);
}

void InputFieldWidget::OnFocusLost() {
    Widget::OnFocusLost(); // this->hasFocus = false
    showCursor = false;
    MarkDirty(false);
}

void InputFieldWidget::MoveCursor(int delta) {
    int newPos = cursorPos + delta;
    cursorPos = std::max(0, std::min((int)text.length(), newPos));
    if (UIManager::Instance().GetRenderer()) { // If called outside Draw, ensure renderer is available
        EnsureCursorVisible(*UIManager::Instance().GetRenderer());
    }
    showCursor = true; // Make cursor visible immediately on move
    lastBlinkTime = std::chrono::steady_clock::now();
    MarkDirty(false);
}

void InputFieldWidget::InsertChar(wchar_t ch) {
    if (maxLength > 0 && text.length() >= (size_t)maxLength) return;

    char mbc[8];
    int len = wctomb(mbc, ch);
    if (len <= 0 || len >= (int)sizeof(mbc)) return; // Invalid char or too long
    mbc[len] = '\0'; // Null-terminate

    char charToInsert = mbc[0]; // Assuming single-byte for now for simplicity

    if (numbersOnly && !isdigit(charToInsert)) return;
    if (isPassword && !isprint(charToInsert)) return; // Only printable for password

    if (forceCaps) charToInsert = toupper(charToInsert);

    text.insert(cursorPos, 1, charToInsert);
    cursorPos++;
    if (UIManager::Instance().GetRenderer()) {
        EnsureCursorVisible(*UIManager::Instance().GetRenderer());
    }
    MarkDirty();
}

void InputFieldWidget::DeleteCharBackward() {
    if (cursorPos > 0 && !text.empty()) {
        text.erase(cursorPos - 1, 1);
        cursorPos--;

        UIRenderer* renderer = UIManager::Instance().GetRenderer();
        if (renderer) EnsureCursorVisible(*renderer);
        MarkDirty();
    }
    if (OnTextChanged) {
        OnTextChanged(this, text);
    }
}

void InputFieldWidget::DeleteCharForward() {
    if (cursorPos < (int)text.length()) {
        text.erase(cursorPos, 1);
        if (UIManager::Instance().GetRenderer()) {
            EnsureCursorVisible(*UIManager::Instance().GetRenderer());
        }
        MarkDirty();
    }
}

int InputFieldWidget::GetCharPixelOffset(UIRenderer& renderer, int charIndex) {
    if (charIndex <= 0 || style.fontKey.empty()) return 0;
    std::string sub = GetDisplayText().substr(0, charIndex);
    return renderer.GetTextExtent(sub, style.fontKey).width;
}

void InputFieldWidget::EnsureCursorVisible(UIRenderer& renderer) {
    Rect clientRelativeBounds = GetClientBounds(); // This is {padding.left, padding.top, ...}
    if (clientRelativeBounds.width <= 0 || style.fontKey.empty()) return;

    int cursorPixelX = GetCharPixelOffset(renderer, cursorPos);

    if (cursorPixelX < scrollOffset) {
        scrollOffset = cursorPixelX;
    }
    else if (cursorPixelX > scrollOffset + clientRelativeBounds.width) {
        scrollOffset = cursorPixelX - clientRelativeBounds.width;
    }
    scrollOffset = std::max(0, scrollOffset);
    MarkDirty(false);
}