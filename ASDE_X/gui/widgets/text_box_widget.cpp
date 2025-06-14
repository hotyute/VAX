// --- START OF FILE text_box_widget.cpp ---

#define NOMINMAX
#include <windows.h>

#include "text_box_widget.h"
#include "ui_renderer.h"
#include "ui_manager.h" // For renderer access
#include "style.h"
#include "font_manager.h" // For FontManager
#include "../../tools.h" // For s2ws and wordWrap (if you adapt it)
#include <algorithm>
#include <sstream> // For word wrap


// (WordWrapTextForUI function from previous response should be here or in a shared tools file)
void WordWrapTextForUI(
    const std::string& textToWrap,
    int maxWidthPixels,
    const std::string& fontKey,
    UIRenderer* renderer,
    std::vector<std::string>& outLines) {

    outLines.clear();
    if (textToWrap.empty() || maxWidthPixels <= 0 || fontKey.empty() || !renderer) {
        if (!textToWrap.empty()) outLines.push_back(textToWrap);
        return;
    }

    std::string currentLine;
    std::string currentWord;
    std::stringstream ss(textToWrap);

    while (ss >> currentWord) {
        if (!currentWord.empty() && renderer->GetTextExtent(currentWord, fontKey).width > maxWidthPixels) {
            if (!currentLine.empty()) {
                outLines.push_back(currentLine);
                currentLine.clear();
            }
            std::string subWord = currentWord;
            while (!subWord.empty()) {
                size_t fitChars = 0;
                for (size_t k = 0; k < subWord.length(); ++k) {
                    if (renderer->GetTextExtent(subWord.substr(0, k + 1), fontKey).width > maxWidthPixels) {
                        break;
                    }
                    fitChars = k + 1;
                }
                if (fitChars == 0 && !subWord.empty()) fitChars = 1;
                outLines.push_back(subWord.substr(0, fitChars));
                subWord.erase(0, fitChars);
            }
            currentWord.clear();
            continue;
        }

        std::string testLine = currentLine;
        if (!testLine.empty()) {
            testLine += " ";
        }
        testLine += currentWord;

        if (renderer->GetTextExtent(testLine, fontKey).width <= maxWidthPixels) {
            if (!currentLine.empty()) {
                currentLine += " ";
            }
            currentLine += currentWord;
        }
        else {
            if (!currentLine.empty()) {
                outLines.push_back(currentLine);
            }
            currentLine = currentWord;
        }
    }

    if (!currentLine.empty()) {
        outLines.push_back(currentLine);
    }

    if (outLines.empty() && !textToWrap.empty()) {
        outLines.push_back(textToWrap);
    }
}


TextBoxWidget::TextBoxWidget() : Widget() {
    style = Style::DefaultDisplayBox();
    focusable = true; // Allow focus for scrolling with keys
    CalculateLineHeight(UIManager::Instance().GetRenderer());
}

int TextBoxWidget::GetLineHeight() const { // Definition
    return calculatedLineHeight;
}

void TextBoxWidget::CalculateLineHeight(UIRenderer* renderer) {
    UIRenderer* r = renderer ? renderer : UIManager::Instance().GetRenderer();

    if (!style.fontKey.empty() && r) {
        calculatedLineHeight = r->GetTextExtent("Mg", style.fontKey).height;
    }
    else if (!style.fontKey.empty()) {
        Size s = FontManager::Instance().GetTextExtent("Mg", style.fontKey);
        calculatedLineHeight = s.height;
    }
    else {
        calculatedLineHeight = 16;
    }
    calculatedLineHeight = std::max(1, calculatedLineHeight) + lineSpacing;
}

void TextBoxWidget::ReWrapAllLines(UIRenderer* renderer, int availableClientAreaWidth) {
    if (availableClientAreaWidth <= 0) {
        for (auto& logicalLine : logicalLines) {
            logicalLine.wrappedSegments.assign(1, logicalLine.text);
            logicalLine.needsWrapping = false;
        }
        UpdateDisplayLines();
        return;
    }

    if (!renderer && !(renderer = UIManager::Instance().GetRenderer())) {
        for (auto& ll : logicalLines) ll.needsWrapping = true;
        return;
    }

    if (style.fontKey.empty()) return;

    CalculateLineHeight(renderer);

    for (auto& logicalLine : logicalLines) {
        logicalLine.wrappedSegments.clear();
        WordWrapTextForUI(logicalLine.text, availableClientAreaWidth, style.fontKey, renderer, logicalLine.wrappedSegments);

        if (logicalLine.wrappedSegments.empty()) {
            logicalLine.wrappedSegments.push_back(""); // Ensure at least one segment, even if empty
        }
        logicalLine.needsWrapping = false;
    }
    UpdateDisplayLines();
}

void TextBoxWidget::UpdateDisplayLines() {
    displayLines.clear();
    contentHeight = 0;
    for (const auto& logicalLine : logicalLines) {
        for (size_t segIdx = 0; segIdx < logicalLine.wrappedSegments.size(); ++segIdx) {
            displayLines.push_back({ &logicalLine, (int)segIdx });
            contentHeight += GetLineHeight();
        }
    }
    EnsureScrollBounds();
}


void TextBoxWidget::AddLine(const std::string& text, const Color& color) {
    logicalLines.emplace_back(text, color);
    if (maxHistory > 0 && logicalLines.size() > (size_t)maxHistory) {
        if (pruneTop) logicalLines.pop_front();
        else logicalLines.pop_back();
    }
    logicalLines.back().needsWrapping = true;
    MarkDirty();
}

void TextBoxWidget::AddLineFront(const std::string& text, const Color& color) {
    logicalLines.emplace_front(text, color);
    if (maxHistory > 0 && logicalLines.size() > (size_t)maxHistory) {
        if (!pruneTop) logicalLines.pop_back();
        else logicalLines.pop_front();
    }
    logicalLines.front().needsWrapping = true;
    MarkDirty();
}

void TextBoxWidget::Clear() {
    logicalLines.clear();
    displayLines.clear();
    scrollOffsetY = 0;
    contentHeight = 0;
    MarkDirty();
}

void TextBoxWidget::SetMaxHistory(int max) {
    maxHistory = max;
    while (maxHistory > 0 && logicalLines.size() > (size_t)maxHistory) {
        if (pruneTop) logicalLines.pop_front(); else logicalLines.pop_back();
    }
    UpdateDisplayLines();
    MarkDirty();
}

void TextBoxWidget::SetReadOnly(bool ro) {
    readOnly = ro;
    focusable = !readOnly;
}

void TextBoxWidget::ScrollDelta(int deltaPixels) {
    scrollOffsetY += deltaPixels;
    EnsureScrollBounds();
    MarkDirty(false);
}

void TextBoxWidget::EnsureScrollBounds() {
    Rect screenBounds = GetScreenBounds();
    int clientHeight = screenBounds.height - (style.padding.top + style.padding.bottom);
    if (contentHeight <= clientHeight) {
        scrollOffsetY = 0;
    }
    else {
        scrollOffsetY = std::max(0, scrollOffsetY);
        scrollOffsetY = std::min(scrollOffsetY, contentHeight - clientHeight);
    }
}

// text_box_widget.cpp
void TextBoxWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    // Base Widget::Draw handles background/border modulated by inheritedAlpha
    Widget::Draw(renderer, inheritedAlpha);

    Rect screenBounds = GetScreenBounds();
    Rect clientScreenBounds = { /* ... calculate clientScreenBounds ... */ };
    clientScreenBounds.x = screenBounds.x + style.padding.left;
    clientScreenBounds.y = screenBounds.y + style.padding.top;
    clientScreenBounds.width = screenBounds.width - (style.padding.left + style.padding.right);
    clientScreenBounds.height = screenBounds.height - (style.padding.top + style.padding.bottom);


    if (clientScreenBounds.width <= 0 || clientScreenBounds.height <= 0) {
        dirty = false;
        return;
    }

    bool needsReWrap = false;
    for (const auto& ll : logicalLines) if (ll.needsWrapping) { needsReWrap = true; break; }
    if (needsReWrap) {
        ReWrapAllLines(&renderer, clientScreenBounds.width);
    }

    renderer.PushClipRect(clientScreenBounds);

    int currentY = clientScreenBounds.y - scrollOffsetY;
    for (const auto& dispLinePair : displayLines) {
        const StyledTextLine* logicalLine = dispLinePair.first;
        int segmentIndex = dispLinePair.second;
        int lineHeight = GetLineHeight();

        if (currentY + lineHeight < clientScreenBounds.y) { currentY += lineHeight; continue; }
        if (currentY > clientScreenBounds.y + clientScreenBounds.height) break;

        if (segmentIndex < logicalLine->wrappedSegments.size()) {
            Color finalLineColor = logicalLine->color;
            finalLineColor.a *= inheritedAlpha; // Modulate color of this specific line

            if (finalLineColor.a > 0.001f) {
                renderer.DrawText(logicalLine->wrappedSegments[segmentIndex],
                    { clientScreenBounds.x, currentY },
                    style.fontKey, // Font key comes from the TextBoxWidget's style
                    finalLineColor);
            }
        }
        currentY += lineHeight;
    }

    renderer.PopClipRect();
    dirty = false;
}

Size TextBoxWidget::Measure() {
    if (calculatedLineHeight <= lineSpacing && UIManager::Instance().GetRenderer()) {
        CalculateLineHeight(UIManager::Instance().GetRenderer());
    }

    int desiredContentHeight = GetLineHeight() * (numBlocksToDisplay > 0 ? numBlocksToDisplay : 5);

    preferredSize = {
        std::max(minSize.width, 150),
        std::max(minSize.height, desiredContentHeight + style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0))
    };

    preferredSize.width = std::max(minSize.width, std::min(preferredSize.width, maxSize.width));
    preferredSize.height = std::max(minSize.height, std::min(preferredSize.height, maxSize.height));

    return preferredSize;
}

void TextBoxWidget::Arrange(const Rect& finalRect) {
    Widget::Arrange(finalRect);

    Rect clientRelativeBounds = GetClientBounds();
    if (clientRelativeBounds.width > 0) {
        if (auto* renderer = UIManager::Instance().GetRenderer()) {
            ReWrapAllLines(renderer, clientRelativeBounds.width);
        }
        else {
            for (auto& ll : logicalLines) ll.needsWrapping = true;
        }
    }
    else {
        for (auto& logicalLine : logicalLines) {
            logicalLine.wrappedSegments.assign(1, logicalLine.text);
            logicalLine.needsWrapping = true;
        }
        UpdateDisplayLines();
    }
    EnsureScrollBounds();
}


void TextBoxWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) return;
    Widget::HandleEvent(event);

    if (event.handled) return;

    // Mouse event should be in local coordinates
    if (event.eventType == UIEvent::Type::Mouse && event.mouse.type == MouseEventType::Scroll) {
        Rect localBounds = { 0,0, bounds.width, bounds.height };
        if (localBounds.Contains(event.mouse.position)) {
            ScrollDelta(-(event.mouse.scrollDelta / (WHEEL_DELTA / GetLineHeight())));
            event.handled = true;
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        if (event.keyboard.type == KeyEventType::Press) {
            Rect client = GetClientBounds();
            int pageScrollAmount = (client.height / GetLineHeight()) - 1;
            pageScrollAmount = std::max(1, pageScrollAmount);
            int lineScrollAmount = 1;

            if (event.keyboard.keyCode == KeyCode::DownArrow) { ScrollDelta(lineScrollAmount * GetLineHeight()); event.handled = true; }
            else if (event.keyboard.keyCode == KeyCode::UpArrow) { ScrollDelta(-lineScrollAmount * GetLineHeight()); event.handled = true; }
            else if (event.keyboard.keyCode == KeyCode::PageDown) { ScrollDelta(pageScrollAmount * GetLineHeight()); event.handled = true; }
            else if (event.keyboard.keyCode == KeyCode::PageUp) { ScrollDelta(-pageScrollAmount * GetLineHeight()); event.handled = true; }
        }
    }
}