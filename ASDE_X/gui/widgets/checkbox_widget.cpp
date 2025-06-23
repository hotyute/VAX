#define NOMINMAX
#include <windows.h>

#include "checkbox_widget.h"
#include "ui_renderer.h"
#include "font_manager.h" 
#include "style.h"        
#include "../../tools.h"  // For s2ws if needed by GetTextExtent or rendering directly
#include "font_keys.h"    // For default font keys

CheckBoxWidget::CheckBoxWidget(const std::string& labelText) : Widget(), label(labelText) {
    style = Style::DefaultLabel(); // Use label style as a base for font and color
    style.padding = {2, 2, 2, 2};
    style.fontKey = FONT_KEY_UI_LABEL; // Ensure a default font key is set
    focusable = true;
}

void CheckBoxWidget::SetChecked(bool checkedState, bool fireEvent) {
    if (isChecked != checkedState) {
        isChecked = checkedState;
        MarkDirty(false);
        if (OnCheckChanged && fireEvent) {
            OnCheckChanged(this, isChecked);
        }
    }
}

void CheckBoxWidget::Draw(UIRenderer& renderer, float inheritedAlpha) {
    if (!visible) return;
    Widget::Draw(renderer, inheritedAlpha); // Base for background/own border (if any)

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

    int boxSize = std::min(clientScreenBounds.height, 16); // Base size on available height, max 16
    boxSize = std::max(boxSize, 8);  // Min size

    // Center the box vertically within the client area's height
    Point boxTopLeft = {clientScreenBounds.x, clientScreenBounds.y + (clientScreenBounds.height - boxSize) / 2};
    Rect boxRect = {boxTopLeft.x, boxTopLeft.y, boxSize, boxSize};

    Color finalGlyphColor = style.foregroundColor; // Use the style's foreground color
    finalGlyphColor.a *= inheritedAlpha; // Modulate by inherited alpha
    
    if (finalGlyphColor.a > 0.001f) {
        renderer.DrawRect(boxRect, finalGlyphColor, false, 1.0f); // Draw the box outline

        if (isChecked) {
            // Draw a checkmark (e.g., two lines)
            Point p1 = {boxRect.x + 2, boxRect.y + boxSize / 2};
            Point p2 = {boxRect.x + boxSize / 2, boxRect.y + boxSize - 2};
            Point p3 = {boxRect.x + boxSize - 2, boxRect.y + 2};
            renderer.DrawLine(p1, p2, finalGlyphColor, 1.5f);
            renderer.DrawLine(p2, p3, finalGlyphColor, 1.5f);
        }
    }

    if (!label.empty() && !style.fontKey.empty()) {
        Size textSize = renderer.GetTextExtent(label, style.fontKey);
        Point textPos = {
            boxRect.x + boxSize + 5, // Space after checkbox
            clientScreenBounds.y + (clientScreenBounds.height - textSize.height) / 2
        };
        Color finalTextColor = style.foregroundColor; // Label uses the same foreground color
        finalTextColor.a *= inheritedAlpha;
        if (finalTextColor.a > 0.001f) {
             renderer.DrawText(label, textPos, style.fontKey, finalTextColor);
        }
    }
    dirty = false;
}

Size CheckBoxWidget::Measure() {
    int boxDisplayWidth = 16 + 5; // box size + spacing for label
    Size textSize = {0,0};

    if (!label.empty() && !style.fontKey.empty()) {
        textSize = FontManager::Instance().GetTextExtent(label, style.fontKey);
    } else if (!label.empty()) { // Fallback if fontKey is somehow not set
        textSize = {(int)label.length() * 8, 16};
    }

    int contentHeight = std::max(16, textSize.height); // Min height for the checkbox itself

    preferredSize = {
        boxDisplayWidth + textSize.width + style.padding.left + style.padding.right,
        contentHeight + style.padding.top + style.padding.bottom
    };
    preferredSize.width = std::max(minSize.width, preferredSize.width);
    preferredSize.height = std::max(minSize.height, preferredSize.height);
    preferredSize.width = std::min(preferredSize.width, maxSize.width);
    preferredSize.height = std::min(preferredSize.height, maxSize.height);
    return preferredSize;
}

void CheckBoxWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) return;

    Rect localBounds = {0,0, bounds.width, bounds.height};
    if (event.eventType == UIEvent::Type::Mouse && event.mouse.type == MouseEventType::Press && event.mouse.button == MouseButton::Left) {
        if (localBounds.Contains(event.mouse.position)) {
            SetChecked(!isChecked); // This will fire OnCheckChanged if state changes
            event.handled = true;
        }
    } else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        if (event.keyboard.type == KeyEventType::Press && (event.keyboard.keyCode == KeyCode::Enter || event.keyboard.keyCode == KeyCode::Space)) {
            SetChecked(!isChecked);
            event.handled = true;
        }
    }
    if (!event.handled) {
        Widget::HandleEvent(event); // For base OnClick etc.
    }
}