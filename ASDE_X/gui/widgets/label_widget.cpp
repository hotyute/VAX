#define NOMINMAX
#include <windows.h>

#include "label_widget.h"
#include "ui_renderer.h"
#include "style.h" // For Style::DefaultLabel()
#include "font_manager.h" // Include FontManager
#include "../../tools.h"

LabelWidget::LabelWidget(const std::string& text) : Widget(), text(text) {
    style = Style::DefaultLabel();
}

void LabelWidget::SetText(const std::string& newText) {
    if (text != newText) {
        text = newText;
        MarkDirty(); // Text change might change size
        if (parent) parent->MarkDirty(); // Notify parent to re-layout
    }
}

// label_widget.cpp
void LabelWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    // Base Widget::Draw handles background/border modulated by inheritedAlpha
    Widget::Draw(renderer, inheritedAlpha);

    if (!text.empty() && !style.fontKey.empty()) {
        Rect screenBounds = GetScreenBounds();
        Point textPos = {
            screenBounds.x + style.padding.left,
            screenBounds.y + style.padding.top
        };

        // For centered text:
        // Size textSize = renderer.GetTextExtent(text, style.fontKey);
        // textPos.x = screenBounds.x + style.padding.left + ( (screenBounds.width - (style.padding.left + style.padding.right)) - textSize.width) / 2;
        // textPos.y = screenBounds.y + style.padding.top + ( (screenBounds.height - (style.padding.top + style.padding.bottom)) - textSize.height) / 2;


        Color finalTextColor = style.foregroundColor;
        finalTextColor.a *= inheritedAlpha; // Modulate text alpha

        if (finalTextColor.a > 0.001f) { // Only draw if not fully transparent
            renderer.DrawText(text, textPos, style.fontKey, finalTextColor);
        }
    }

    dirty = false;
}

Size LabelWidget::Measure() {
    if (text.empty() || style.fontKey.empty()) {
        preferredSize = { style.padding.left + style.padding.right, style.padding.top + style.padding.bottom };
    }
    else {
        // Use FontManager to get text dimensions
        Size textSize = FontManager::Instance().GetTextExtent(text, style.fontKey);

        preferredSize = {
            textSize.width + style.padding.left + style.padding.right,
            textSize.height + style.padding.top + style.padding.bottom
        };
    }
    // Apply min/max size constraints if any
    preferredSize.width = std::max(minSize.width, std::min(preferredSize.width, maxSize.width));
    preferredSize.height = std::max(minSize.height, std::min(preferredSize.height, maxSize.height));
    return preferredSize;
}