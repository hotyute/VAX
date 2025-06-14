#define NOMINMAX
#include <windows.h> // For OutputDebugString
#include <vector>    // For std::vector in GetScreenBounds
#include <algorithm> // For std::reverse in GetScreenBounds

#include "widget.h"
#include "ui_manager.h"
#include "ui_renderer.h"
#include "window_widget.h" // For dynamic_cast in GetScreenBounds

Widget::Widget() {
    // Default style could be applied here or in derived classes
}

void Widget::AddChild(std::unique_ptr<Widget> child) {
    if (child) {
        child->parent = this;
        children.push_back(std::move(child));
        MarkDirty();
    }
}

Widget* Widget::GetChildById(const std::string& childId) {
    if (this->id == childId) return this; // Check self first
    for (const auto& child : children) {
        if (child->id == childId) {
            return child.get();
        }
        Widget* found = child->GetChildById(childId); // Recursive call
        if (found) return found;
    }
    return nullptr;
}

// THIS IS THE CRITICAL FUNCTION FOR DRAWING AND HIT-TESTING (for UIManager)
Rect Widget::GetScreenBounds() const {
    Rect screenRect = this->bounds; // Start with my own bounds (x,y are relative to parent's client area)

    Widget* p = this->parent;
    while (p) {
        // Add parent's client area origin (which is parent's own screen x/y + its padding)
        screenRect.x += p->bounds.x; // Add parent's x (which is rel to its parent)
        screenRect.y += p->bounds.y; // Add parent's y

        // Add parent's padding, as this widget is inside it
        screenRect.x += p->style.padding.left;
        screenRect.y += p->style.padding.top;

        p = p->parent;
    }
    // After the loop, screenRect.x and screenRect.y are now absolute screen coordinates
    // because the loop terminates when p is nullptr (i.e., after processing the WindowWidget
    // whose bounds.x/y were already screen coordinates).
    return screenRect;
}


Rect Widget::GetClientBounds() const {
    // Client bounds are relative to this widget's own (0,0) after padding
    return {
        style.padding.left,
        style.padding.top,
        bounds.width - (style.padding.left + style.padding.right),
        bounds.height - (style.padding.top + style.padding.bottom)
    };
}

void Widget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) {
        event.handled = false; // Ensure not marked handled if widget is not active
        return;
    }

    // This base HandleEvent is for the widget itself to react to events
    // for which its specific coordinates are relevant (e.g. a click directly on it).
    // The event.mouse.position is expected to be LOCAL to this widget.

    bool handledBySpecificCallback = false;
    if (event.eventType == UIEvent::Type::Mouse) {
        // Create a local rect for hit-testing against (0,0) origin
        Rect localEventBounds = { 0, 0, bounds.width, bounds.height };

        switch (event.mouse.type) {
        case MouseEventType::Press:
            if (localEventBounds.Contains(event.mouse.position)) { // Check against local 0,0
                if (focusable && !hasFocus) RequestFocus(); // Request focus if clicked
                if (OnMouseDown) { OnMouseDown(this, event.mouse); handledBySpecificCallback = true; }
            }
            break;
        case MouseEventType::Release:
            if (localEventBounds.Contains(event.mouse.position)) { // Check if release is inside
                if (OnMouseUp) { OnMouseUp(this, event.mouse); handledBySpecificCallback = true; }
                if (OnClick) { // OnClick only if press also likely happened on this widget
                    OnClick(this); handledBySpecificCallback = true;
                }
            }
            else { // Released outside
                if (OnMouseUp) { OnMouseUp(this, event.mouse); handledBySpecificCallback = true; } // Still send mouse up
            }
            break;
        case MouseEventType::Move:
            if (localEventBounds.Contains(event.mouse.position)) {
                if (OnMouseMove) { OnMouseMove(this, event.mouse); handledBySpecificCallback = true; }
            }
            break;
        case MouseEventType::Scroll:
            if (localEventBounds.Contains(event.mouse.position)) {
                if (OnMouseScroll) { OnMouseScroll(this, event.mouse); handledBySpecificCallback = true; }
            }
            break;
        case MouseEventType::Enter: // Enter/Leave are usually handled by UIManager or parent container
            if (OnMouseEnter) { OnMouseEnter(this, event.mouse); handledBySpecificCallback = true; }
            break;
        case MouseEventType::Leave:
            if (OnMouseLeave) { OnMouseLeave(this, event.mouse); handledBySpecificCallback = true; }
            break;
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        switch (event.keyboard.type) {
        case KeyEventType::Press:
            if (OnKeyPress) { OnKeyPress(this, event.keyboard); handledBySpecificCallback = true; }
            break;
        case KeyEventType::Release:
            if (OnKeyRelease) { OnKeyRelease(this, event.keyboard); handledBySpecificCallback = true; }
            break;
        case KeyEventType::Char:
            if (OnCharInput) { OnCharInput(this, event.keyboard); handledBySpecificCallback = true; }
            break;
        }
    }

    if (handledBySpecificCallback) {
        event.handled = true;
        MarkDirty();
        return;
    }

    // Generic OnEvent callback if specific ones didn't handle it
    if (OnEvent) {
        OnEvent(this, event); // Event coords are still local to this widget
        if (event.handled) {
            MarkDirty();
            return;
        }
    }
    // If it reaches here, this widget (and its specific/generic callbacks) didn't handle the event.
    // The 'handled' flag in the passed 'event' remains as it was.
}

void Widget::Update(float deltaTime) {
    for (auto& child : children) {
        if (child && child->visible) {
            child->Update(deltaTime);
        }
    }
}

// widget.cpp
void Widget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Add parameter
    if (!visible) return;

    Rect screenBounds = GetScreenBounds();
    Style currentDrawingStyle = this->style;

    // Modulate this widget's own background and border alpha by the inherited alpha
    currentDrawingStyle.backgroundColor.a *= inheritedAlpha;
    currentDrawingStyle.borderColor.a *= inheritedAlpha;
    // Note: Foreground color for text is usually kept opaque for readability,
    // but you could modulate it too if desired:
    // currentDrawingStyle.foregroundColor.a *= inheritedAlpha;


    if (currentDrawingStyle.backgroundColor.a > 0.001f) { // Use a small epsilon for float comparison
        renderer.DrawRect(screenBounds, currentDrawingStyle.backgroundColor, true);
    }

    if (currentDrawingStyle.hasBorder && currentDrawingStyle.borderWidth > 0.0f && currentDrawingStyle.borderColor.a > 0.001f) {
        renderer.DrawRect(screenBounds, currentDrawingStyle.borderColor, false, currentDrawingStyle.borderWidth);
    }
    // Children are drawn by ContainerWidget::Draw, which will also receive inheritedAlpha
    // For simple widgets, drawing of text/glyphs will happen in their own Draw overrides.
    dirty = false; // This should be set by the top-level Draw initiator (WindowWidget or UIManager::Render)
}

Size Widget::Measure() {
    preferredSize = minSize;
    preferredSize.width = std::max(minSize.width, std::min(preferredSize.width, maxSize.width));
    preferredSize.height = std::max(minSize.height, std::min(preferredSize.height, maxSize.height));
    return preferredSize;
}

void Widget::Arrange(const Rect& finalRect) {
    this->bounds = finalRect; // finalRect is relative to parent's client area
    MarkDirty(); // Arrangement changed, needs redraw
}

void Widget::OnFocusGained() {
    hasFocus = true;
    MarkDirty(false); // Only this widget needs redraw for focus state
}

void Widget::OnFocusLost() {
    hasFocus = false;
    MarkDirty(false);
}

void Widget::SetVisible(bool v) {
    if (visible != v) {
        visible = v;
        if (parent) parent->MarkDirty(true); // Parent needs to re-layout/redraw
        else MarkDirty(true); // If top-level, mark self and children
    }
}
void Widget::SetEnabled(bool e) {
    if (enabled != e) {
        enabled = e;
        MarkDirty(false);
    }
}

void Widget::RequestFocus() {
    if (focusable && visible && enabled) {
        UIManager::Instance().SetFocus(this);
    }
}

void Widget::MarkDirty(bool includeChildren) {
    dirty = true;
    if (parent) {
        parent->MarkDirty(false); // Propagate dirty upwards, but don't re-propagate to children from parent
    }
    if (includeChildren) {
        for (auto& child : children) {
            if (child) child->MarkDirty(true);
        }
    }
}

// This base implementation is for non-container widgets.
// ContainerWidget will override it.
bool Widget::DispatchEventToChildren(UIEvent& event) {
    return false; // Non-containers don't have children to dispatch to in this manner
}