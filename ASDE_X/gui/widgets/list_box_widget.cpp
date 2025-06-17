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
    if (!visible || !enabled) {
        event.handled = false; // Ensure not handled if widget is inactive
        return;
    }

    // If already handled by a more specific derived class or a previously checked sibling, bail.
    if (event.handled) {
        return;
    }

    Rect localBounds = { 0, 0, bounds.width, bounds.height }; // Bounds of the ListBox itself
    Rect localClientBounds = GetClientBounds(); // Area inside padding where items are drawn

    if (event.eventType == UIEvent::Type::Mouse) {
        // event.mouse.position is ALREADY LOCAL to this ListBoxWidget's (0,0)
        // because UIManager/ContainerWidget would have translated it.
        Point mousePosInWidget = event.mouse.position;



        // Calculate mouse position relative to the client area (where items are actually drawn)
        Point mousePosInClient = {
            mousePosInWidget.x - localClientBounds.x,
            mousePosInWidget.y - localClientBounds.y
        };

        // Check if mouse is within the overall bounds of the ListBox widget first
        if (!localBounds.Contains(mousePosInWidget)) {
            // If mouse isn't even over the widget, but it somehow got the event (e.g., capture),
            // we might still want to handle specific things like a Leave event if it was previously hovered.
            if (event.mouse.type == MouseEventType::Leave) {
                if (hoverIndex != -1) {
                    hoverIndex = -1;
                    MarkDirty(false);
                }
            }
            // For other events, if not within bounds, usually don't handle.
            // However, UIManager's dispatch logic should prevent non-capture events from reaching here
            // if the mouse isn't over.
            // Widget::HandleEvent(event); // Call base for generic non-hit events if any
            return;
        }

        // At this point, the mouse IS over the ListBoxWidget.
        // Now check if it's specifically within the client area for item interactions.
        bool mouseOverClientArea = localClientBounds.Contains(mousePosInWidget);


        switch (event.mouse.type) {
        case MouseEventType::Move:
            if (mouseOverClientArea) {
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
            else { // Mouse is over padding/border of the ListBox, not items
                if (hoverIndex != -1) { hoverIndex = -1; MarkDirty(false); }
            }
            // ListBox often consumes Move events over it to manage hover, even if no specific callback.
            event.handled = true;
            break;

        case MouseEventType::Press:
            if (mouseOverClientArea) { // Click must be on the item area
                int idx = (mousePosInClient.y + scrollOffsetY) / itemHeight;
                if (idx >= 0 && idx < (int)items.size()) { // Clicked on a valid item
                    if (event.mouse.button == MouseButton::Left) {
                        if (selectedIndex != idx) {
                            selectedIndex = idx;
                            // hoverIndex = idx; // Already handled by move or will be updated
                            if (OnItemSelected) {
                                OnItemSelected(this, selectedIndex, items[selectedIndex]);
                            }
                            MarkDirty(false);
                        }
                        else { // Clicked on already selected item
                            if (OnItemSelected) { // Some might want to re-trigger selection
                                OnItemSelected(this, selectedIndex, items[selectedIndex]);
                            }
                        }
                        RequestFocus();
                        event.handled = true;
                    }
                    else if (event.mouse.button == MouseButton::Right) {
                        // Select the item on right-click as well (common behavior)
                        if (selectedIndex != idx) {
                            selectedIndex = idx;
                            // hoverIndex = idx;
                            // Optionally call OnItemSelected for right-click selection consistency:
                            // if (OnItemSelected) {
                            //    OnItemSelected(this, selectedIndex, items[selectedIndex]);
                            // }
                            MarkDirty(false);
                        }
                        RequestFocus();

                        if (OnRightClickItem) {
                            // event.mouse.screen_position should have been populated by UIManager
                            OnRightClickItem(this, idx, items[idx], event.mouse.raw_position);
                        }
                        event.handled = true;
                    }
                }
            }
            else { // Click on padding/border
                RequestFocus(); // Focus the ListBox even if click is not on an item
                event.handled = true; // Consume click on non-item area if ListBox is focusable
            }
            break;

        case MouseEventType::Release:
            // Standard OnClick from base Widget::HandleEvent might be called if not handled here.
            // If you want specific release logic inside an item:
            // if (mouseOverClientArea && event.mouse.button == MouseButton::Left) {
            //    int idx = (mousePosInClient.y + scrollOffsetY) / itemHeight;
            //    if (idx == selectedIndex && idx != -1) {
            //        // Potentially trigger an action if release is on the selected item
            //    }
            // }
            // For now, let base handle generic clicks or specific mouse up.
            break;

        case MouseEventType::Scroll:
            if (mouseOverClientArea || localBounds.Contains(mousePosInWidget)) { // Scroll if mouse is anywhere over the ListBox
                int oldOffset = scrollOffsetY;
                int scrollAmount = (event.mouse.scrollDelta / WHEEL_DELTA) * itemHeight; // Scroll by item height units
                SetScrollOffset(scrollOffsetY - scrollAmount);
                if (scrollOffsetY != oldOffset) {
                    // If ListBox is part of ScrollableListBox, the parent needs to know
                    // to update its scrollbar. This might be done via an OnScroll callback
                    // from ListBox or by parent polling.
                    // For now, ListBox handles its own scroll state.
                    event.handled = true;
                }
            }
            break;

        case MouseEventType::Leave:
            if (hoverIndex != -1) {
                hoverIndex = -1;
                MarkDirty(false);
            }
            // event.handled = true; // Usually, leave events are informational
            break;

        default:
            break;
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        if (event.keyboard.type == KeyEventType::Press) {
            int newIndex = selectedIndex;
            if (items.empty() && event.keyboard.keyCode != KeyCode::Escape) { // Allow Escape even if empty
                event.handled = true;
                return;
            }

            switch (event.keyboard.keyCode) {
            case KeyCode::UpArrow:
                newIndex = (selectedIndex <= 0) ? 0 : selectedIndex - 1;
                if (items.empty()) newIndex = -1;
                break;
            case KeyCode::DownArrow:
                newIndex = (selectedIndex < 0 && !items.empty()) ? 0 : std::min((int)items.size() - 1, selectedIndex + 1);
                if (items.empty()) newIndex = -1;
                break;
            case KeyCode::Home:
                newIndex = items.empty() ? -1 : 0;
                break;
            case KeyCode::End:
                newIndex = items.empty() ? -1 : (int)items.size() - 1;
                break;
            case KeyCode::PageUp:
                if (!items.empty()) {
                    int itemsPerPage = localClientBounds.height > 0 ? (localClientBounds.height / itemHeight) : 1;
                    itemsPerPage = std::max(1, itemsPerPage);
                    newIndex = (selectedIndex < 0) ? 0 : std::max(0, selectedIndex - itemsPerPage);
                }
                else {
                    newIndex = -1;
                }
                break;
            case KeyCode::PageDown:
                if (!items.empty()) {
                    int itemsPerPage = localClientBounds.height > 0 ? (localClientBounds.height / itemHeight) : 1;
                    itemsPerPage = std::max(1, itemsPerPage);
                    newIndex = (selectedIndex < 0) ? (std::min((int)items.size() - 1, itemsPerPage - 1)) : std::min((int)items.size() - 1, selectedIndex + itemsPerPage);
                }
                else {
                    newIndex = -1;
                }
                break;
            case KeyCode::Enter:
            case KeyCode::Space: // Often Space also selects in lists
                if (selectedIndex != -1 && OnItemSelected) {
                    OnItemSelected(this, selectedIndex, items[selectedIndex]);
                }
                break;
            default: // Unhandled key press for ListBox
                event.handled = false; // Explicitly mark unhandled if not processed
                break; // Important to prevent event.handled = true below for these keys
            }

            // If a key was handled above (other than default), event.handled should be true by now
            // or will be set true after this block.
            if (newIndex != selectedIndex && !items.empty()) {
                selectedIndex = newIndex;
                hoverIndex = newIndex;

                int itemTopY = selectedIndex * itemHeight;
                int itemBottomY = itemTopY + itemHeight;
                if (localClientBounds.height > 0) { // Ensure client bounds are valid for scrolling
                    if (itemTopY < scrollOffsetY) {
                        SetScrollOffset(itemTopY);
                    }
                    else if (itemBottomY > scrollOffsetY + localClientBounds.height) {
                        SetScrollOffset(itemBottomY - localClientBounds.height);
                    }
                }
                MarkDirty(false);
            }
            // If any keyboard navigation happened, or Enter/Space was pressed, mark handled.
            if (event.keyboard.keyCode == KeyCode::UpArrow || event.keyboard.keyCode == KeyCode::DownArrow ||
                event.keyboard.keyCode == KeyCode::Home || event.keyboard.keyCode == KeyCode::End ||
                event.keyboard.keyCode == KeyCode::PageUp || event.keyboard.keyCode == KeyCode::PageDown ||
                event.keyboard.keyCode == KeyCode::Enter || event.keyboard.keyCode == KeyCode::Space) {
                event.handled = true;
            }
        }
    }

    // If the event was not handled by specific ListBox logic, pass it to the base Widget
    if (!event.handled) {
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