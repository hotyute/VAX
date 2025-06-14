#define NOMINMAX
#include <windows.h>

#include "list_box_widget.h"
#include "ui_renderer.h"
#include "font_manager.h"
#include "style.h" // For default style

extern std::wstring s2ws(const std::string& s);


ListBoxWidget::ListBoxWidget() : Widget() {
    style = Style::DefaultInputField();
    style.hasBorder = true;
    focusable = true;

    if (!style.fontKey.empty()) {
        Size textSize = FontManager::Instance().GetTextExtent("Mg", style.fontKey);
        itemHeight = textSize.height + style.padding.top + style.padding.bottom;
        itemHeight = std::max(itemHeight, 16);
    }
    else {
        itemHeight = 20;
    }
    itemHeight = std::max(1, itemHeight);
}

void ListBoxWidget::AddItem(const std::string& item) {
    items.push_back(item);
    MarkDirty();
    if (parent) parent->MarkDirty();
}

void ListBoxWidget::ClearItems() {
    items.clear();
    selectedIndex = -1;
    hoverIndex = -1;
    MarkDirty();
    if (parent) parent->MarkDirty();
}

// list_box_widget.cpp
void ListBoxWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    // Base Widget::Draw handles the ListBox's own background/border,
    // modulated by inheritedAlpha.
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

    int currentY = clientScreenBounds.y - scrollOffsetY;

    for (size_t i = 0; i < items.size(); ++i) {
        if (currentY + itemHeight < clientScreenBounds.y) {
            currentY += itemHeight;
            continue;
        }
        if (currentY > clientScreenBounds.y + clientScreenBounds.height) {
            break;
        }

        Rect itemRectScreen = { clientScreenBounds.x, currentY, clientScreenBounds.width, itemHeight };

        // Start with the ListBox's base style for items
        Color itemBgColor = style.backgroundColor;
        Color itemTextColor = style.foregroundColor;

        // Apply selection/hover states
        if ((int)i == selectedIndex) {
            itemBgColor = { 0.2f, 0.4f, 0.8f, 1.0f }; // Selected item background (typically opaque)
            itemTextColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Selected item text (typically opaque)
        }
        else if ((int)i == hoverIndex && style.backgroundColor.a > 0.001f) { // Only apply hover if base has some alpha
            itemBgColor.r = std::min(1.0f, itemBgColor.r * 1.1f + 0.05f);
            itemBgColor.g = std::min(1.0f, itemBgColor.g * 1.1f + 0.05f);
            itemBgColor.b = std::min(1.0f, itemBgColor.b * 1.1f + 0.05f);
            // Hovered item text color usually remains the default style.foregroundColor
        }

        // Now, modulate these item-specific colors by the inheritedAlpha
        itemBgColor.a *= inheritedAlpha;
        itemTextColor.a *= inheritedAlpha;

        if (itemRectScreen.width > 0 && itemRectScreen.height > 0 && itemBgColor.a > 0.001f) {
            renderer.DrawRect(itemRectScreen, itemBgColor);
        }

        if (itemTextColor.a > 0.001f) {
            Size textSize = renderer.GetTextExtent(items[i], style.fontKey);
            Point textPos = {
                itemRectScreen.x + style.padding.left, // Use ListBox's style.padding for text within item
                itemRectScreen.y + (itemHeight - textSize.height) / 2
            };
            renderer.DrawText(items[i], textPos, style.fontKey, itemTextColor);
        }

        currentY += itemHeight;
    }

    renderer.PopClipRect();
    dirty = false;
}

Size ListBoxWidget::Measure() {
    int maxWidth = 0;
    if (!style.fontKey.empty()) {
        for (const auto& item : items) {
            Size itemSize = FontManager::Instance().GetTextExtent(item, style.fontKey);
            if (itemSize.width > maxWidth) {
                maxWidth = itemSize.width;
            }
        }
    }
    else {
        for (const auto& item : items) maxWidth = std::max(maxWidth, (int)item.length() * 8);
    }

    int totalContentHeight = (int)items.size() * itemHeight;
    if (items.empty()) {
        totalContentHeight = itemHeight;
    }

    preferredSize = {
        maxWidth + style.padding.left + style.padding.right + (style.hasBorder ? 2 * (int)style.borderWidth : 0),
        totalContentHeight + style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0)
    };

    preferredSize.width = std::max(minSize.width > 0 ? minSize.width : 50, preferredSize.width);
    preferredSize.height = std::max(minSize.height > 0 ? minSize.height : itemHeight, preferredSize.height);

    preferredSize.width = std::min(preferredSize.width, maxSize.width);
    preferredSize.height = std::min(preferredSize.height, maxSize.height);

    return preferredSize;
}

void ListBoxWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) return;
    // Widget::HandleEvent(event); // Base handles OnClick etc., which we might not want for ListBox directly

    if (event.handled) return;

    Rect localClientBounds = GetClientBounds(); // Relative to ListBox's (0,0) + padding

    if (event.eventType == UIEvent::Type::Mouse) {
        Point mousePosInWidget = event.mouse.position; // Mouse pos relative to ListBox's (0,0)

        // Mouse position relative to the client area (where items are drawn)
        Point mousePosInClient = {
            mousePosInWidget.x - localClientBounds.x,
            mousePosInWidget.y - localClientBounds.y
        };

        if (event.mouse.type == MouseEventType::Move) {
            if (mousePosInClient.x >= 0 && mousePosInClient.x < localClientBounds.width &&
                mousePosInClient.y >= 0 && mousePosInClient.y < localClientBounds.height)
            {
                // Item index calculation needs to account for scrollOffset
                int idx = (mousePosInClient.y + scrollOffsetY) / itemHeight;
                if (idx >= 0 && idx < (int)items.size()) {
                    if (hoverIndex != idx) {
                        hoverIndex = idx;
                        MarkDirty(false);
                    }
                }
                else {
                    if (hoverIndex != -1) { hoverIndex = -1; MarkDirty(false); }
                }
            }
            else {
                if (hoverIndex != -1) { hoverIndex = -1; MarkDirty(false); }
            }
            event.handled = true;
        }
        else if (event.mouse.type == MouseEventType::Press && event.mouse.button == MouseButton::Left) {
            if (mousePosInClient.x >= 0 && mousePosInClient.x < localClientBounds.width &&
                mousePosInClient.y >= 0 && mousePosInClient.y < localClientBounds.height)
            {
                int idx = (mousePosInClient.y + scrollOffsetY) / itemHeight; // Account for scroll
                if (idx >= 0 && idx < (int)items.size()) { // Clicked on a valid item
                    selectedIndex = idx;
                    hoverIndex = idx; // Update hover to selected
                    if (OnItemSelected) {
                        OnItemSelected(this, selectedIndex, items[selectedIndex]);
                    }
                    MarkDirty(false);
                    RequestFocus(); // ListBox should get focus on click
                }
                event.handled = true;
            }
        }
        else if (event.mouse.type == MouseEventType::Scroll) {
            // This event is relative to the ListBox widget itself.
            // The ScrollableListBoxWidget's HandleEvent should capture this if mouse is over ListBox.
        }
        else if (event.mouse.type == MouseEventType::Leave) {
            if (hoverIndex != -1) { hoverIndex = -1; MarkDirty(false); }
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        // Keyboard navigation should also account for scrolling and ensure selected item is visible
        if (event.keyboard.type == KeyEventType::Press) {
            int newIndex = selectedIndex;
            if (items.empty()) { event.handled = true; return; }

            if (event.keyboard.keyCode == KeyCode::UpArrow) {
                newIndex = (selectedIndex <= 0) ? 0 : selectedIndex - 1;
            }
            else if (event.keyboard.keyCode == KeyCode::DownArrow) {
                newIndex = (selectedIndex < 0) ? 0 : std::min((int)items.size() - 1, selectedIndex + 1);
            }
            else if (event.keyboard.keyCode == KeyCode::Home) {
                newIndex = 0;
            }
            else if (event.keyboard.keyCode == KeyCode::End) {
                newIndex = (int)items.size() - 1;
            }
            else if (event.keyboard.keyCode == KeyCode::PageUp) {
                int itemsPerPage = localClientBounds.height / itemHeight;
                newIndex = std::max(0, selectedIndex - itemsPerPage);
            }
            else if (event.keyboard.keyCode == KeyCode::PageDown) {
                int itemsPerPage = localClientBounds.height / itemHeight;
                newIndex = std::min((int)items.size() - 1, selectedIndex + itemsPerPage);
                if (selectedIndex < 0) newIndex = std::min((int)items.size() - 1, itemsPerPage - 1);
            }
            else if (event.keyboard.keyCode == KeyCode::Enter && selectedIndex != -1) {
                if (OnItemSelected) OnItemSelected(this, selectedIndex, items[selectedIndex]);
            }

            if (newIndex != selectedIndex || (selectedIndex == -1 && newIndex == 0 && !items.empty())) {
                selectedIndex = newIndex;
                hoverIndex = newIndex; // Keep hover consistent
                // Ensure selected item is visible
                int itemTopY = selectedIndex * itemHeight;
                int itemBottomY = itemTopY + itemHeight;
                if (itemTopY < scrollOffsetY) { // Item is above visible area
                    SetScrollOffset(itemTopY);
                }
                else if (itemBottomY > scrollOffsetY + localClientBounds.height) { // Item is below
                    SetScrollOffset(itemBottomY - localClientBounds.height);
                }
                MarkDirty(false);
            }
            event.handled = true;
        }
    }
    if (!event.handled) { // If not handled by specific listbox logic, pass to base
        Widget::HandleEvent(event);
    }
}

void ListBoxWidget::SetScrollOffset(int newOffset) {
    int maxScroll = GetContentHeight() - GetClientBounds().height;
    if (maxScroll < 0) maxScroll = 0;

    scrollOffsetY = std::max(0, std::min(newOffset, maxScroll));
    MarkDirty(false);
}

int ListBoxWidget::GetContentHeight() const {
    return (int)items.size() * itemHeight;
}