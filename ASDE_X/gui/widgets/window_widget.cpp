// --- START OF FILE window_widget.cpp ---

#define NOMINMAX
#include <windows.h>

#include "window_widget.h"
#include "../ui_windows/settings_window.h"
#include "ui_renderer.h"
#include "ui_manager.h" // For removing window
#include "layout_strategy.h" // For default content area layout


WindowWidget::WindowWidget(const std::string& title) : ContainerWidget(), title(title) {
    this->style = Style::DefaultWindow();
    this->focusable = true; // Windows can be focused

    auto defaultContentLayout = std::make_unique<VerticalStackLayout>();
    defaultContentLayout->padding = { 5,5,5,5 };
    contentArea = new ContainerWidget(std::move(defaultContentLayout)); // Raw new, owned by this WindowWidget via AddChild
    ContainerWidget::AddChild(std::unique_ptr<Widget>(contentArea)); // Add contentArea as a child of WindowWidget
}


Rect WindowWidget::GetTitleBarRect() const {
    if (!hasTitleBar) return { bounds.x, bounds.y, 0, 0 }; // Return zero-sized rect if no title bar
    // The rect is relative to the window's own origin for internal logic, 
    // but drawing will use the absolute screen bounds.
    return { 0, 0, bounds.width, titleBarHeight };
}

Rect WindowWidget::GetCloseButtonRect() const {
    if (!closable || !hasTitleBar) return { 0,0,0,0 };
    int buttonSize = titleBarHeight - 8; // Make it a bit smaller than title bar height
    return { bounds.width - buttonSize - 4, 4, buttonSize, buttonSize };
}

void WindowWidget::Draw(UIRenderer& renderer, float inheritedAlpha) {
    if (!visible) return;

    Rect screenBounds = GetScreenBounds(); // Absolute screen coordinates of the window

    // --- Determine overall effective alpha for this window and its direct content ---
    float currentWindowEffectiveAlpha = inheritedAlpha; // Start with alpha from UIManager (usually 1.0 for top windows)
    bool applyGlobalOpacitySetting = true;

    if (dynamic_cast<const SettingsWindow*>(this) || this->id.find("_dropdown_popup") != std::string::npos) {
        applyGlobalOpacitySetting = false;
    }

    if (applyGlobalOpacitySetting) {
        currentWindowEffectiveAlpha *= SettingsWindow::GetGlobalWindowOpacity();
    }
    // currentWindowEffectiveAlpha now holds the target opacity for this window's frame
    // and will be the 'inheritedAlpha' for its direct child 'contentArea'.

    // --- Prepare style for drawing this window's frame ---
    Style windowFrameDrawStyle = this->style; // Start with a copy of this window's original style
    windowFrameDrawStyle.backgroundColor.a *= currentWindowEffectiveAlpha;
    // Optionally, make the window border also use this effective alpha:
    // windowFrameDrawStyle.borderColor.a *= currentWindowEffectiveAlpha;


    // 1. Draw Window Background & Border (using windowFrameDrawStyle)
    if (windowFrameDrawStyle.backgroundColor.a > 0.001f) { // Use a small epsilon
        renderer.DrawRect(screenBounds, windowFrameDrawStyle.backgroundColor, true);
    }
    if (windowFrameDrawStyle.hasBorder && windowFrameDrawStyle.borderWidth > 0.0f && windowFrameDrawStyle.borderColor.a > 0.001f) {
        renderer.DrawRect(screenBounds, windowFrameDrawStyle.borderColor, false, windowFrameDrawStyle.borderWidth);
    }

    // 2. Draw Title Bar (if any)
    if (hasTitleBar && titleBarHeight > 0) {
        Rect titleScreenRect = { screenBounds.x, screenBounds.y, screenBounds.width, titleBarHeight };

        Color titleBgColor = this->style.borderColor; // Base color from original style for contrast

        // Title bar opacity can be handled differently:
        // It could be fully opaque, match currentWindowEffectiveAlpha, or be slightly more opaque.
        float titleBarEffectiveAlpha = this->style.borderColor.a; // Start with its own alpha
        if (applyGlobalOpacitySetting) {
            // Example: make it follow global opacity, but slightly less transparent
            titleBarEffectiveAlpha *= std::min(1.0f, SettingsWindow::GetGlobalWindowOpacity() + 0.2f);
            // Or simply: titleBarEffectiveAlpha *= currentWindowEffectiveAlpha; (if it was just modulated by inherited)
        }
        titleBgColor.a = titleBarEffectiveAlpha;


        if (titleBgColor.a > 0.001f) {
            renderer.DrawRect(titleScreenRect, titleBgColor);
        }

        // Draw title text (usually opaque for readability, over the title bar BG)
        // Its color alpha might also be modulated if desired, but typically not.
        Color titleTextColor = this->style.foregroundColor;
        // titleTextColor.a *= currentWindowEffectiveAlpha; // If you want title text to fade too
        Size titleTextSize = renderer.GetTextExtent(title, this->style.fontKey);
        int textY = titleScreenRect.y + (titleBarHeight - titleTextSize.height) / 2;
        if (titleTextColor.a > 0.001f) {
            renderer.DrawText(title, { titleScreenRect.x + 5, textY }, this->style.fontKey, titleTextColor);
        }


        // Draw close button (typically opaque)
        if (closable) {
            Rect closeRelativeRect = GetCloseButtonRect();
            Rect closeScreenRect = {
                screenBounds.x + closeRelativeRect.x,
                screenBounds.y + closeRelativeRect.y,
                closeRelativeRect.width,
                closeRelativeRect.height
            };
            renderer.DrawRect(closeScreenRect, { 0.7f, 0.2f, 0.2f, 1.0f }); // Opaque close button
            Point p1 = { closeScreenRect.x + 4, closeScreenRect.y + 4 };
            Point p2 = { closeScreenRect.x + closeScreenRect.width - 4, closeScreenRect.y + closeScreenRect.height - 4 };
            renderer.DrawLine(p1, p2, { 1, 1, 1 });
            p1 = { closeScreenRect.x + closeScreenRect.width - 4, closeScreenRect.y + 4 };
            p2 = { closeScreenRect.x + 4, closeScreenRect.y + closeScreenRect.height - 4 };
            renderer.DrawLine(p1, p2, { 1, 1, 1 });
        }
    }

    // 3. Draw Content Area and its children
    Rect contentClientAreaScreen;
    contentClientAreaScreen.x = screenBounds.x + this->style.padding.left;
    contentClientAreaScreen.y = screenBounds.y + this->style.padding.top + (hasTitleBar ? titleBarHeight : 0);
    contentClientAreaScreen.width = screenBounds.width - (this->style.padding.left + this->style.padding.right);
    contentClientAreaScreen.height = screenBounds.height - (this->style.padding.top + this->style.padding.bottom + (hasTitleBar ? titleBarHeight : 0));

    if (contentClientAreaScreen.width < 0) contentClientAreaScreen.width = 0;
    if (contentClientAreaScreen.height < 0) contentClientAreaScreen.height = 0;

    bool pushedClip = false;
    if (contentClientAreaScreen.width > 0 && contentClientAreaScreen.height > 0) {
        renderer.PushClipRect(contentClientAreaScreen);
        pushedClip = true;
    }

    if (contentArea && contentArea->visible) {
        // Pass the currentWindowEffectiveAlpha down to the contentArea.
        // The contentArea (and its children, via ContainerWidget::Draw)
        // will use this to modulate their own drawing.
        contentArea->Draw(renderer, currentWindowEffectiveAlpha);
    }

    if (pushedClip) {
        renderer.PopClipRect();
    }

    this->dirty = false; // This window (frame and content) is now drawn
}


void WindowWidget::HandleEvent(UIEvent& event_from_manager) {
    if (!visible || !enabled) return;

    bool eventHandledByChromeOrDrag = false;
    Point screenMousePos = event_from_manager.mouse.position;

    // Convert screen mouse position to be local to the window's bounds (origin 0,0)
    Point windowLocalMousePos = screenMousePos;
    windowLocalMousePos.x -= bounds.x;
    windowLocalMousePos.y -= bounds.y;

    if (event_from_manager.eventType == UIEvent::Type::Mouse) {
        Rect titleBarLocalRect = GetTitleBarRect(); // Relative to window: {0, 0, ...}
        Rect closeButtonLocalRect = GetCloseButtonRect(); // Relative to window: {width-size, ...}

        if (event_from_manager.mouse.type == MouseEventType::Press && event_from_manager.mouse.button == MouseButton::Left) {
            if (focusable) {
                UIManager::Instance().BringWindowToFront(this); // UIManager will also handle focus request
            }

            if (hasTitleBar && closable && closeButtonLocalRect.Contains(windowLocalMousePos)) {
                UIManager::Instance().RemoveWindow(this);
                eventHandledByChromeOrDrag = true;
            }
            else if (hasTitleBar && draggable && titleBarLocalRect.Contains(windowLocalMousePos)) {
                isDragging = true;
                dragStartMousePos = screenMousePos;
                dragStartWindowPos = { bounds.x, bounds.y };
                UIManager::Instance().SetCapture(this);
                eventHandledByChromeOrDrag = true;
            }
        }
        else if (isDragging && event_from_manager.mouse.type == MouseEventType::Move) {
            int dx = screenMousePos.x - dragStartMousePos.x;
            int dy = screenMousePos.y - dragStartMousePos.y;
            bounds.x = dragStartWindowPos.x + dx;
            bounds.y = dragStartWindowPos.y + dy;
            MarkDirty(true);
            eventHandledByChromeOrDrag = true;
        }
        else if (isDragging && event_from_manager.mouse.type == MouseEventType::Release && event_from_manager.mouse.button == MouseButton::Left) {
            isDragging = false;
            UIManager::Instance().ReleaseCapture();
            Arrange(this->bounds);
            MarkDirty(true);
            eventHandledByChromeOrDrag = true;
        }
    }

    if (eventHandledByChromeOrDrag) {
        event_from_manager.handled = true;
        return;
    }

    // Create a new event with coordinates made local to the window's content area for dispatching
    if (contentArea && contentArea->visible && contentArea->enabled) {
        if (contentArea->bounds.Contains(windowLocalMousePos)) { // Mouse is within content area
            UIEvent contentEvent = event_from_manager;
            contentEvent.mouse.position.x = windowLocalMousePos.x - contentArea->bounds.x;
            contentEvent.mouse.position.y = windowLocalMousePos.y - contentArea->bounds.y;
            contentArea->HandleEvent(contentEvent);
            if (contentEvent.handled) {
                event_from_manager.handled = true;
            }
        }
    }

    if (!event_from_manager.handled) {
        UIEvent baseEvent = event_from_manager;
        baseEvent.mouse.position = windowLocalMousePos;
        Widget::HandleEvent(baseEvent);
        if (baseEvent.handled) {
            event_from_manager.handled = true;
        }
    }
}


Size WindowWidget::Measure() {
    Size contentSize = { 0,0 };
    if (contentArea) {
        contentSize = contentArea->Measure();
    }

    preferredSize.width = std::max(minSize.width, contentSize.width + style.padding.left + style.padding.right);
    preferredSize.height = std::max(minSize.height, contentSize.height + style.padding.top + style.padding.bottom + (hasTitleBar ? titleBarHeight : 0));
    return preferredSize;
}

void WindowWidget::Arrange(const Rect& finalRect_screen_coords) {

    Widget::Arrange(finalRect_screen_coords);

    if (contentArea) {
        Rect contentAreaRelativeRect;
        contentAreaRelativeRect.x = style.padding.left;
        contentAreaRelativeRect.y = (hasTitleBar ? titleBarHeight : 0) + style.padding.top;
        contentAreaRelativeRect.width = this->bounds.width - (style.padding.left + style.padding.right);
        contentAreaRelativeRect.height = this->bounds.height - (hasTitleBar ? titleBarHeight : 0) - (style.padding.top + style.padding.bottom);
        if (contentAreaRelativeRect.width < 0) contentAreaRelativeRect.width = 0;
        if (contentAreaRelativeRect.height < 0) contentAreaRelativeRect.height = 0;

        contentArea->Arrange(contentAreaRelativeRect);
    }
    MarkDirty(true);
}