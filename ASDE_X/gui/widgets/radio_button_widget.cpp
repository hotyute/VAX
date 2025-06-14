#define NOMINMAX
#include <windows.h>

#include "radio_button_widget.h"
#include "radio_group_widget.h"
#include "ui_renderer.h"
#include "font_manager.h"
#include "style.h"
#include "../../tools.h"

RadioButtonWidget::RadioButtonWidget(const std::string& labelText) : Widget(), label(labelText) {
    style = Style::DefaultLabel();
    style.padding = { 2, 2, 2, 2 };
    focusable = true;
}

void RadioButtonWidget::SetChecked(bool checkedState) {
    if (isChecked != checkedState) {
        isChecked = checkedState;
        MarkDirty(false);
        if (isChecked && group) {
            group->OnRadioButtonSelected(this);
        }
    }
}

// radio_button_widget.cpp
void RadioButtonWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    // Base Widget::Draw handles its own background/border modulation
    Widget::Draw(renderer, inheritedAlpha);

    Rect screenBounds = GetScreenBounds();
    Rect clientScreenBounds = { /* ... calculate clientScreenBounds ... */ };
    clientScreenBounds.x = screenBounds.x + style.padding.left;
    clientScreenBounds.y = screenBounds.y + style.padding.top;
    clientScreenBounds.width = screenBounds.width - (style.padding.left + style.padding.right);
    clientScreenBounds.height = screenBounds.height - (style.padding.top + style.padding.bottom);


    int radioSize = std::min(clientScreenBounds.width, clientScreenBounds.height) - 4;
    radioSize = std::min(radioSize, 16);
    radioSize = std::max(radioSize, 8);
    Point radioCenter = { clientScreenBounds.x + radioSize / 2, clientScreenBounds.y + clientScreenBounds.height / 2 };

    Color finalGlyphColor = style.foregroundColor;
    finalGlyphColor.a *= inheritedAlpha;

    if (finalGlyphColor.a > 0.001f) {
        Rect outerCircle = { radioCenter.x - radioSize / 2, radioCenter.y - radioSize / 2, radioSize, radioSize };
        renderer.DrawRect(outerCircle, finalGlyphColor, false, 1.0f);

        if (isChecked) {
            Rect innerCircle = {
                radioCenter.x - radioSize / 4, radioCenter.y - radioSize / 4,
                radioSize / 2, radioSize / 2
            };
            renderer.DrawRect(innerCircle, finalGlyphColor, true);
        }
    }

    if (!label.empty()) {
        Size textSize = renderer.GetTextExtent(label, style.fontKey);
        Point textPos = {
            clientScreenBounds.x + radioSize + 5,
            clientScreenBounds.y + (clientScreenBounds.height - textSize.height) / 2
        };

        Color finalTextColor = style.foregroundColor; // Assuming label text uses foregroundColor
        finalTextColor.a *= inheritedAlpha;
        if (finalTextColor.a > 0.001f) {
            renderer.DrawText(label, textPos, style.fontKey, finalTextColor);
        }
    }
    dirty = false;
}

Size RadioButtonWidget::Measure() {
    int radioCircleDisplayWidth = 16 + 5;
    Size textSize = { 0,0 };

    if (!label.empty() && !style.fontKey.empty()) {
        textSize = FontManager::Instance().GetTextExtent(label, style.fontKey);
    }
    else if (!label.empty()) {
        textSize = { static_cast<int>(label.length()) * 8, 16 };
    }

    int contentHeight = std::max(16, textSize.height);

    preferredSize = {
        radioCircleDisplayWidth + textSize.width + style.padding.left + style.padding.right,
        contentHeight + style.padding.top + style.padding.bottom
    };

    preferredSize.width = std::max(minSize.width, preferredSize.width);
    preferredSize.height = std::max(minSize.height, preferredSize.height);
    preferredSize.width = std::min(preferredSize.width, maxSize.width);
    preferredSize.height = std::min(preferredSize.height, maxSize.height);

    return preferredSize;
}

void RadioButtonWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) return;

    // Let base class handle OnClick if it's set up
    Widget::HandleEvent(event);
    if (event.handled) return;

    // Use local coordinates for hit testing.
    Rect localBounds = { 0, 0, bounds.width, bounds.height };

    if (event.eventType == UIEvent::Type::Mouse && event.mouse.type == MouseEventType::Press && event.mouse.button == MouseButton::Left) {
        if (localBounds.Contains(event.mouse.position)) {
            if (!isChecked) {
                SetChecked(true);
            }
            event.handled = true;
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        if (event.keyboard.type == KeyEventType::Press && (event.keyboard.keyCode == KeyCode::Enter || event.keyboard.keyCode == KeyCode::Space)) {
            if (!isChecked) SetChecked(true);
            event.handled = true;
        }
    }
}