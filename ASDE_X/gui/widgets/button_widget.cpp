#define NOMINMAX
#include <windows.h>

#include "button_widget.h"
#include "ui_manager.h"
#include "ui_renderer.h"
#include "style.h" // For Style::DefaultButton()
#include "font_manager.h"
#include "../../tools.h"

ButtonWidget::ButtonWidget(const std::string& text) : Widget(), text(text) {
    style = Style::DefaultButton();
    focusable = true;
}

void ButtonWidget::SetText(const std::string& newText) {
    if (text != newText) {
        text = newText;
        MarkDirty(true); // Mark dirty to trigger remeasure/redraw
    }
}

// button_widget.cpp
void ButtonWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    Rect screenBounds = GetScreenBounds();

    // Prepare a temporary style for drawing this button instance
    Style currentDrawingStyle = this->style;

    // Modulate base colors by inheritedAlpha
    currentDrawingStyle.backgroundColor.a *= inheritedAlpha;
    currentDrawingStyle.borderColor.a *= inheritedAlpha;
    // currentDrawingStyle.foregroundColor.a *= inheritedAlpha; // Text color will be handled separately

    // Modify background color based on state (after applying inheritedAlpha)
    if (isPressed) {
        currentDrawingStyle.backgroundColor.r *= 0.8f;
        currentDrawingStyle.backgroundColor.g *= 0.8f;
        currentDrawingStyle.backgroundColor.b *= 0.8f;
    }
    else if (hasFocus) {
        currentDrawingStyle.backgroundColor.r *= 1.2f;
        currentDrawingStyle.backgroundColor.g *= 1.2f;
        currentDrawingStyle.backgroundColor.b *= 1.2f;
    }
    // Clamp colors
    currentDrawingStyle.backgroundColor.r = std::max(0.0f, std::min(1.0f, currentDrawingStyle.backgroundColor.r));
    currentDrawingStyle.backgroundColor.g = std::max(0.0f, std::min(1.0f, currentDrawingStyle.backgroundColor.g));
    currentDrawingStyle.backgroundColor.b = std::max(0.0f, std::min(1.0f, currentDrawingStyle.backgroundColor.b));


    // Draw background and border using the modified style
    if (currentDrawingStyle.backgroundColor.a > 0.001f) {
        renderer.DrawRect(screenBounds, currentDrawingStyle.backgroundColor, true);
    }
    if (currentDrawingStyle.hasBorder && currentDrawingStyle.borderWidth > 0.0f && currentDrawingStyle.borderColor.a > 0.001f) {
        renderer.DrawRect(screenBounds, currentDrawingStyle.borderColor, false, currentDrawingStyle.borderWidth);
    }
    // Note: We are NOT calling Widget::Draw(renderer, inheritedAlpha); here because we want to
    // apply the pressed/focus state *after* the inheritedAlpha is applied to the base style.

    if (!text.empty() && !style.fontKey.empty()) {
        Size textSize = renderer.GetTextExtent(text, style.fontKey);
        int clientWidth = screenBounds.width - (style.padding.left + style.padding.right);
        int clientHeight = screenBounds.height - (style.padding.top + style.padding.bottom);
        Point textPos = {
            screenBounds.x + style.padding.left + (clientWidth - textSize.width) / 2,
            screenBounds.y + style.padding.top + (clientHeight - textSize.height) / 2
        };

        Color finalTextColor = style.foregroundColor; // Start with original text color
        finalTextColor.a *= inheritedAlpha;         // Modulate its alpha

        if (finalTextColor.a > 0.001f) {
            renderer.DrawText(text, textPos, style.fontKey, finalTextColor);
        }
    }
    dirty = false;
}

Size ButtonWidget::Measure() {
    Size textSize = { 0,0 };
    if (!text.empty() && !style.fontKey.empty()) {
        textSize = FontManager::Instance().GetTextExtent(text, style.fontKey);
    }
    else if (!text.empty()) {
        textSize = { static_cast<int>(text.length()) * 8, 16 };
    }

    preferredSize = {
        textSize.width + style.padding.left + style.padding.right,
        textSize.height + style.padding.top + style.padding.bottom
    };

    // Add border to size calculation
    if (style.hasBorder) {
        preferredSize.width += 2 * style.borderWidth;
        preferredSize.height += 2 * style.borderWidth;
    }

    preferredSize.width = std::max(preferredSize.width, minSize.width > 0 ? minSize.width : 30);
    preferredSize.height = std::max(preferredSize.height, minSize.height > 0 ? minSize.height : 20);

    preferredSize.width = std::min(preferredSize.width, maxSize.width);
    preferredSize.height = std::min(preferredSize.height, maxSize.height);

    return preferredSize;
}

void ButtonWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) return;

    // Event coordinates are local to this widget
    Rect localBounds = { 0,0, bounds.width, bounds.height };

    if (event.eventType == UIEvent::Type::Mouse) {
        if (event.mouse.type == MouseEventType::Press && event.mouse.button == MouseButton::Left) {
            if (localBounds.Contains(event.mouse.position)) {
                isPressed = true;
                MarkDirty(false);
                event.handled = true;
            }
        }
        else if (event.mouse.type == MouseEventType::Release && event.mouse.button == MouseButton::Left) {
            bool wasPressed = isPressed;
            isPressed = false;
            MarkDirty(false);
            if (wasPressed && localBounds.Contains(event.mouse.position)) {
                if (OnClick) OnClick(this);
                event.handled = true;
            }
        }
        else if (event.mouse.type == MouseEventType::Leave) {
            if (isPressed) {
                isPressed = false;
                MarkDirty(false);
            }
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        if (event.keyboard.type == KeyEventType::Press && (event.keyboard.keyCode == KeyCode::Enter || event.keyboard.keyCode == KeyCode::Space)) {
            if (OnClick) OnClick(this);
            event.handled = true;
        }
    }

    if (!event.handled) {
        Widget::HandleEvent(event);
    }
}