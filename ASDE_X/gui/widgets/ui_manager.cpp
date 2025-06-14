#define NOMINMAX // If not already there or in a pch.h
#include <windows.h>
#include <windowsx.h> // For GET_X_LPARAM, GET_Y_LPARAM

#include "ui_manager.h"
#include "window_widget.h" // For WindowWidget
#include "ui_renderer.h"   // For UIRenderer
#include "../../aircraft.h"
#include <algorithm>    // For std::remove, std::find_if

UIManager& UIManager::Instance() {
    static UIManager instance;
    return instance;
}

void UIManager::Initialize(std::unique_ptr<UIRenderer> r, int sw, int sh) {
    renderer = std::move(r);
    screenWidth = sw;
    screenHeight = sh;
}

void UIManager::Shutdown() {
    windows.clear();
    renderer.reset();
}

void UIManager::AddWindow(std::unique_ptr<WindowWidget> window) {
    if (window) {
        window->Arrange({window->bounds.x == 0 && window->bounds.y == 0 ? 50 : window->bounds.x, // Default position if not set
                         window->bounds.x == 0 && window->bounds.y == 0 ? 50 : window->bounds.y,
                         window->Measure().width, window->Measure().height});
        windows.push_back(std::move(window)); // Add to top
    }
}

void UIManager::RemoveWindow(WindowWidget* window) {
    if (!window) return;

    // Check if the window being removed is related to current interaction widgets
    if (focusedWidget) {
        Widget* current = focusedWidget;
        while (current) {
            if (current == window) {
                SetFocus(nullptr); // Will call OnFocusLost on the old focusedWidget
                break;
            }
            current = current->parent;
        }
    }
    if (mouseHoverWidget) {
        Widget* current = mouseHoverWidget;
        while (current) {
            if (current == window) {
                mouseHoverWidget = nullptr; // Clear it directly
                break;
            }
            current = current->parent;
        }
    }
    if (capturedWidget == window) { // Or if capturedWidget is a child of 'window'
        ReleaseCapture(); // This will set capturedWidget = nullptr
    }
    else if (capturedWidget) {
        Widget* current = capturedWidget;
        while (current) {
            if (current == window) {
                ReleaseCapture();
                break;
            }
            current = current->parent;
        }
    }


    windows.erase(std::remove_if(windows.begin(), windows.end(),
        [window](const std::unique_ptr<WindowWidget>& p) {
            return p.get() == window;
        }),
        windows.end());
    // The unique_ptr destructor will handle deleting the window and its children
}

void UIManager::BringWindowToFront(WindowWidget* windowToMove) {
    if (!windowToMove) return;
    if (!windows.empty() && windows.back().get() == windowToMove) {
        // It's already the front-most window.
        // If it doesn't have focus AND the current UIManager::focusedWidget is not a child of it,
        // then request focus. Otherwise, if a child has focus, leave it.
        bool childHasFocus = false;
        if (focusedWidget && focusedWidget != windowToMove) {
            Widget* p = focusedWidget->parent;
            while (p) {
                if (p == windowToMove) {
                    childHasFocus = true;
                    break;
                }
                p = p->parent;
            }
        }
        if (!windowToMove->hasFocus && !childHasFocus && windowToMove->focusable) {
            // OutputDebugStringA(("BringWindowToFront: Window was already front, requesting focus for window itself: " + windowToMove->id + "\n").c_str());
            windowToMove->RequestFocus(); // This will call SetFocus(windowToMove)
        }
        return;
    }
    // ... (rest of your existing find and move logic) ...
    std::unique_ptr<WindowWidget> tempOwner = nullptr;
    // ... (your existing find logic) ...
    auto it = std::find_if(windows.begin(), windows.end(),
        [&](const std::unique_ptr<WindowWidget>& p) { return p.get() == windowToMove; });

    if (it != windows.end()) {
        tempOwner = std::move(*it);
        windows.erase(it);
    } // else: warning/error, window not managed?

    if (tempOwner) {
        windows.push_back(std::move(tempOwner));
        WindowWidget* frontWindow = windows.back().get(); // now stable pointer

        // --- MODIFIED FOCUS LOGIC ---
        // Only request focus on the window IF the current focusedWidget
        // is not already this window or one of its children.
        bool focusIsAlreadyInThisWindowOrIsThisWindow = false;
        if (focusedWidget) {
            if (focusedWidget == frontWindow) {
                focusIsAlreadyInThisWindowOrIsThisWindow = true;
            }
            else {
                Widget* p = focusedWidget->parent;
                while (p) {
                    if (p == frontWindow) {
                        focusIsAlreadyInThisWindowOrIsThisWindow = true;
                        break;
                    }
                    p = p->parent;
                }
            }
        }

        if (!focusIsAlreadyInThisWindowOrIsThisWindow && frontWindow && frontWindow->focusable) {
            // OutputDebugStringA(("BringWindowToFront: Window moved to front, requesting focus FOR THE WINDOW: " + frontWindow->id + "\n").c_str());
            frontWindow->RequestFocus(); // This will call SetFocus(frontWindow)
        }
        else {
            // OutputDebugStringA(("BringWindowToFront: Window moved to front, but focus remains on: " + (focusedWidget ? focusedWidget->id : "null") + "\n").c_str());
        }
    }
}

WindowWidget* UIManager::GetWindowById(const std::string& id) {
    for (const auto& win_ptr : windows) {
        if (win_ptr && win_ptr->id == id) {
            return win_ptr.get();
        }
    }
    return nullptr;
}

// In ui_manager.cpp
// Make sure to include the ComboBoxWidget header
#include "combo_box_widget.h" // Or whatever path it is
#include "list_box_widget.h"  // For ListBoxWidget type

void UIManager::ProcessSystemEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    lastEventHandledByUI_INTERNAL = false;

    UIEvent uiEvent;
    Point screenPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // Populate common event data
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
        uiEvent.eventType = UIEvent::Type::Mouse;
        uiEvent.mouse.position = screenPos; // Screen coordinates for UIManager level
        uiEvent.mouse.shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        uiEvent.mouse.ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        uiEvent.mouse.altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
    }
    else if ((message >= WM_KEYFIRST && message <= WM_KEYLAST) || message == WM_CHAR || message == WM_SYSCHAR) {
        uiEvent.eventType = UIEvent::Type::Keyboard;
        uiEvent.keyboard.shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        uiEvent.keyboard.ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        uiEvent.keyboard.altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
    }
    else {
        // For messages like WM_SIZE, WM_SETFOCUS, WM_KILLFOCUS, specific handling below.
    }


    switch (message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN: // Consider if right-click should also close dropdowns or interact
    case WM_MBUTTONDOWN: {
        uiEvent.mouse.type = MouseEventType::Press;
        if (message == WM_LBUTTONDOWN) uiEvent.mouse.button = MouseButton::Left;
        else if (message == WM_RBUTTONDOWN) uiEvent.mouse.button = MouseButton::Right;
        else uiEvent.mouse.button = MouseButton::Middle;

        if (message == WM_LBUTTONDOWN) leftMouseDown = true;
        lastMousePosition = screenPos;

        // --- MODIFIED click-outside-dropdown logic ---
        ComboBoxWidget* currentActiveDropdownOwner = ComboBoxWidget::GetActiveDropdownOwner_Static();
        WindowWidget* dropdownWindowAssociatedWithActiveComboBox = nullptr; // Store the dropdown window itself

        if (currentActiveDropdownOwner) {
            ListBoxWidget* openList = currentActiveDropdownOwner->GetCurrentOpenDropdownList();
            if (openList && openList->parent && // Parent is ScrollableListBox
                openList->parent->parent && // Grandparent is ContentArea
                openList->parent->parent->parent) { // Great-grandparent is WindowWidget
                dropdownWindowAssociatedWithActiveComboBox = dynamic_cast<WindowWidget*>(openList->parent->parent->parent);
            }
        }

        bool clickIsInsideActiveDropdownMechanism = false;
        if (currentActiveDropdownOwner) {
            // Check 1: Is the click on the ComboBox button itself?
            if (currentActiveDropdownOwner->GetScreenBounds().Contains(screenPos)) {
                clickIsInsideActiveDropdownMechanism = true;
            }
            // Check 2: Is the click inside the bounds of the dropdown window?
            else if (dropdownWindowAssociatedWithActiveComboBox &&
                dropdownWindowAssociatedWithActiveComboBox->GetScreenBounds().Contains(screenPos)) {
                clickIsInsideActiveDropdownMechanism = true;
            }
        }

        if (currentActiveDropdownOwner && !clickIsInsideActiveDropdownMechanism) {
            // OutputDebugStringA("UIManager: Click truly outside active dropdown mechanism. Closing dropdown.\n");
            ComboBoxWidget::CloseActiveDropdown_Static(); // This calls ToggleDropdown on the owner
            // The event is *not* handled here by the dropdown closing.
            // The click should still be processed by whatever was actually clicked.
        }
        // --- END click-outside-dropdown logic MODIFICATION ---


        WindowWidget* topWindowUnderCursor = nullptr;

        if (capturedWidget) {
            uiEvent.targetWidget = capturedWidget;
            // Convert screenPos to be local to capturedWidget's parent window for HandleEvent
            // (or ensure capturedWidget's HandleEvent can take screen coordinates if needed)
            // This part is tricky; typically captured widget gets raw translated coords.
            // For now, assuming HandleEvent can manage.
            capturedWidget->HandleEvent(uiEvent); // Event coords are screenPos for this direct call
        }
        else {
            for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
                if ((*it)->visible && (*it)->GetScreenBounds().Contains(screenPos)) {
                    topWindowUnderCursor = it->get();
                    break;
                }
            }

            if (topWindowUnderCursor) {
                // If the window clicked is not the active dropdown window AND not the owner combobox itself
                // AND it's not already the top-most, bring it to front.
                // (The ComboBox toggle itself handles bringing dropdown to front)
                bool isPartOfActiveDropdown = (topWindowUnderCursor == dropdownWindowAssociatedWithActiveComboBox) ||
                    (currentActiveDropdownOwner && topWindowUnderCursor == currentActiveDropdownOwner->parent); // Check if topWindow is the ComboBox's parent window


                if (!windows.empty() && windows.back().get() != topWindowUnderCursor) {
                    // If the click that closed a dropdown also landed on a window that needs bringing to front
                    BringWindowToFront(topWindowUnderCursor);
                }
                else if (windows.back().get() == topWindowUnderCursor && !topWindowUnderCursor->hasFocus) {
                    // If it was already front but didn't have focus (and no child did), request focus.
                    bool childHasFocus = false;
                    if (focusedWidget && focusedWidget != topWindowUnderCursor) {
                        Widget* p = focusedWidget->parent;
                        while (p) { if (p == topWindowUnderCursor) { childHasFocus = true; break; } p = p->parent; }
                    }
                    if (!childHasFocus) {
                        // Only request focus if it's not the dropdown window,
                        // as the dropdown window's internal components (listbox) should get focus.
                        if (topWindowUnderCursor != dropdownWindowAssociatedWithActiveComboBox) {
                            topWindowUnderCursor->RequestFocus();
                        }
                    }
                }

                // Dispatch the event to the window that was actually clicked.
                // UIManager passes SCREEN coordinates to the top-level window's HandleEvent.
                // The window's HandleEvent is responsible for making them local.
                uiEvent.targetWidget = topWindowUnderCursor; // Target is the window itself
                topWindowUnderCursor->HandleEvent(uiEvent); // Event coords are screenPos
            }
        }
        if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        break;
    }

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP: {
        uiEvent.mouse.type = MouseEventType::Release;
        if (message == WM_LBUTTONUP) { uiEvent.mouse.button = MouseButton::Left; leftMouseDown = false; }
        else if (message == WM_RBUTTONUP) uiEvent.mouse.button = MouseButton::Right;
        else uiEvent.mouse.button = MouseButton::Middle;

        if (capturedWidget) {
            uiEvent.targetWidget = capturedWidget;
            capturedWidget->HandleEvent(uiEvent);
        }
        else {
            WindowWidget* topWindow = nullptr;
            for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
                if ((*it)->visible && (*it)->GetScreenBounds().Contains(screenPos)) {
                    topWindow = it->get();
                    break;
                }
            }
            if (topWindow) {
                uiEvent.targetWidget = topWindow;
                topWindow->HandleEvent(uiEvent);
            }
        }
        if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        break;
    }

    case WM_MOUSEMOVE: {
        uiEvent.mouse.type = MouseEventType::Move;
        Widget* newHoverTarget = nullptr;
        WindowWidget* topHitWindow = nullptr; // The window the mouse is currently over
        uiEvent.mouse.screen_position = screenPos; // Store raw screen position ALWAYS

        if (capturedWidget) {
            newHoverTarget = capturedWidget;
            uiEvent.targetWidget = newHoverTarget;

            newHoverTarget->HandleEvent(uiEvent); // Captured widget gets the move event
        }
        else {
            // Find the window the mouse is over
            for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
                if ((*it)->visible && (*it)->GetScreenBounds().Contains(screenPos)) {
                    topHitWindow = it->get();
                    // Now find the specific widget within that window
                    newHoverTarget = WidgetAtScreenPoint(screenPos, topHitWindow);
                    break;
                }
            }

            // Handle Enter/Leave events
            if (mouseHoverWidget != newHoverTarget) {
                if (mouseHoverWidget) { // Leaving the old widget
                    UIEvent leaveEvent(uiEvent); // Copy base event data (like modifiers)
                    leaveEvent.mouse.type = MouseEventType::Leave;
                    // mouse.position in leaveEvent should be where it left FROM (lastMousePosition)
                    // but for simplicity, current screenPos is often fine for the widget to know it's no longer hovered.
                    leaveEvent.mouse.position = screenPos; // Or lastMousePosition;
                    leaveEvent.targetWidget = mouseHoverWidget;

                    // Dispatch leave to the window containing mouseHoverWidget, or mouseHoverWidget itself
                    WindowWidget* oldHoverWindow = nullptr;
                    Widget* temp = mouseHoverWidget;
                    while (temp) { if ((oldHoverWindow = dynamic_cast<WindowWidget*>(temp))) break; temp = temp->parent; }
                    if (oldHoverWindow) oldHoverWindow->HandleEvent(leaveEvent);
                    else if (mouseHoverWidget) mouseHoverWidget->HandleEvent(leaveEvent);
                }
                if (newHoverTarget) { // Entering the new widget
                    UIEvent enterEvent(uiEvent);
                    enterEvent.mouse.type = MouseEventType::Enter;
                    enterEvent.mouse.position = screenPos;
                    enterEvent.targetWidget = newHoverTarget;

                    // Dispatch enter to the window containing newHoverTarget, or newHoverTarget itself
                    WindowWidget* newActualHoverWindow = nullptr; // this is topHitWindow if newHoverTarget is in it
                    Widget* temp = newHoverTarget;
                    while (temp) { if ((newActualHoverWindow = dynamic_cast<WindowWidget*>(temp))) break; temp = temp->parent; }
                    if (newActualHoverWindow) newActualHoverWindow->HandleEvent(enterEvent);
                    else if (newHoverTarget) newHoverTarget->HandleEvent(enterEvent);
                }
                mouseHoverWidget = newHoverTarget;
            }

            // Dispatch the actual move event to the window the mouse is currently over
            if (topHitWindow && !uiEvent.handled) { // If Enter/Leave didn't handle it
                uiEvent.targetWidget = topHitWindow; // Target is the window
                topHitWindow->HandleEvent(uiEvent);  // Window will dispatch to children if needed
            }
        }
        lastMousePosition = screenPos;
        if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        break;
    }

    case WM_MOUSEWHEEL: {
        uiEvent.mouse.type = MouseEventType::Scroll;
        POINT wheelPtRaw = { screenPos.x, screenPos.y };
        ScreenToClient(hwnd, &wheelPtRaw);
        Point wheelPt = { wheelPtRaw.x, wheelPtRaw.y };
        uiEvent.mouse.scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        Widget* targetScrollWidget = nullptr;
        if (capturedWidget) {
            targetScrollWidget = capturedWidget;
        }
        else {
            // Prefer focused widget for scroll, then hovered widget, then widget under mouse
            if (focusedWidget && focusedWidget->GetScreenBounds().Contains(wheelPt)) {
                targetScrollWidget = focusedWidget;
            }
            else if (mouseHoverWidget && mouseHoverWidget->GetScreenBounds().Contains(screenPos)) {
                targetScrollWidget = mouseHoverWidget;
            }
            else { // Fallback to widget directly under mouse
                for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
                    if ((*it)->visible && (*it)->GetScreenBounds().Contains(screenPos)) {
                        targetScrollWidget = WidgetAtScreenPoint(screenPos, (*it).get());
                        break;
                    }
                }
            }
        }

        if (targetScrollWidget) {
            uiEvent.targetWidget = targetScrollWidget;
            WindowWidget* containingWindow = nullptr;
            Widget* temp = targetScrollWidget;
            while (temp) { if ((containingWindow = dynamic_cast<WindowWidget*>(temp))) break; temp = temp->parent; }

            if (containingWindow) containingWindow->HandleEvent(uiEvent); // Let window dispatch
            else if (dynamic_cast<WindowWidget*>(targetScrollWidget)) { // If target itself is a window
                static_cast<WindowWidget*>(targetScrollWidget)->HandleEvent(uiEvent);
            }
            // If it's a non-window widget somehow directly targeted (should not happen with current logic)
            // else targetScrollWidget->HandleEvent(uiEvent); 
        }
        if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        break;
    }

                      // --- KEYBOARD EVENTS ---
    case WM_KEYDOWN: case WM_SYSKEYDOWN: {
        if (focusedWidget) {
            uiEvent.keyboard.type = KeyEventType::Press;
            uiEvent.keyboard.keyCode = VKToKeyCode(wParam, lParam);
            uiEvent.targetWidget = focusedWidget;
            focusedWidget->HandleEvent(uiEvent);
            if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        }
        break;
    }
    case WM_KEYUP: case WM_SYSKEYUP: {
        if (focusedWidget) {
            uiEvent.keyboard.type = KeyEventType::Release;
            uiEvent.keyboard.keyCode = VKToKeyCode(wParam, lParam);
            uiEvent.targetWidget = focusedWidget;
            focusedWidget->HandleEvent(uiEvent);
            if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        }
        break;
    }
    case WM_CHAR: case WM_SYSCHAR: {
        if (focusedWidget && wParam > 0) {
            uiEvent.keyboard.type = KeyEventType::Char;
            uiEvent.keyboard.character = static_cast<wchar_t>(wParam);
            uiEvent.targetWidget = focusedWidget;
            focusedWidget->HandleEvent(uiEvent);
            if (uiEvent.handled) this->lastEventHandledByUI_INTERNAL = true;
        }
        break;
    }

                // --- WINDOW MANAGEMENT AND FOCUS EVENTS ---
    case WM_SIZE: {
        screenWidth = LOWORD(lParam);
        screenHeight = HIWORD(lParam);
        // OutputDebugStringA(("UIManager: WM_SIZE received. New screenWidth: " + std::to_string(screenWidth) + ", screenHeight: " + std::to_string(screenHeight) + "\n").c_str());
        for (auto& win : windows) {
            if (win->visible) {
                // Basic resize: keep position, adjust size up to new screen bounds
                // More sophisticated policies (e.g. centering, anchoring) could be added.
                Rect oldBounds = win->bounds;
                Size prefSize = win->Measure(); // Re-measure based on new potential space

                Rect newBounds;
                newBounds.x = oldBounds.x;
                newBounds.y = oldBounds.y;
                newBounds.width = std::min(prefSize.width, screenWidth - oldBounds.x);
                newBounds.height = std::min(prefSize.height, screenHeight - oldBounds.y);

                // Ensure it doesn't go off screen
                newBounds.width = std::max(0, newBounds.width);
                newBounds.height = std::max(0, newBounds.height);
                newBounds.x = std::max(0, std::min(newBounds.x, screenWidth - newBounds.width));
                newBounds.y = std::max(0, std::min(newBounds.y, screenHeight - newBounds.height));

                win->Arrange(newBounds); // Arrange with new bounds
                win->MarkDirty(true);    // Mark dirty for redraw
            }
        }
        // Tell renderer about the new screen size for its projection setup
        if (renderer) renderer->BeginFrame(screenWidth, screenHeight); // This might be too early, Render() does this.
        // But if any Measure/Arrange needs renderer context with new size.

        this->lastEventHandledByUI_INTERNAL = true; // UIManager handles resize for its windows
        break;
    }
    case WM_SETFOCUS: // Main application window gained focus
        // OutputDebugStringA("UIManager: WM_SETFOCUS received by main application window.\n");
        if (focusedWidget) {
            focusedWidget->MarkDirty(); // Redraw with focus indication
        }
        else {
            // If no UI widget has logical focus, try to focus the topmost window
            if (!windows.empty()) {
                for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
                    if ((*it)->visible && (*it)->focusable) {
                        (*it)->RequestFocus();
                        break;
                    }
                }
            }
        }
        this->lastEventHandledByUI_INTERNAL = true;
        break;
    case WM_KILLFOCUS: // Main application window lost focus
        // OutputDebugStringA("UIManager: WM_KILLFOCUS received by main application window.\n");
        // When app loses focus, UI widgets still "have" focus internally.
        // We might want to visually remove focus indicators.
        if (focusedWidget) {
            // focusedWidget->OnFocusLost(); // This might be too aggressive if app is just temporarily losing focus
            focusedWidget->MarkDirty(); // Redraw to remove focus highlight
        }
        // Do not clear focusedWidget here, it represents the internal UI focus state.
        this->lastEventHandledByUI_INTERNAL = true;
        break;
    }

    // This was at the end of the original. Moved it inside each case
    // if (uiEvent.handled) {
    //    this->lastEventHandledByUI_INTERNAL = true;
    // }
}

FlightPlanWindow* UIManager::GetFlightPlanWindowForAircraft(Aircraft* aircraft) {
    if (!aircraft) return nullptr;
    std::string targetCallsign = aircraft->getCallsign(); // Or however you get it
    std::string targetId = "FPWindow_" + targetCallsign; // Assuming consistent ID scheme
    WindowWidget* win = GetWindowById(targetId);
    return dynamic_cast<FlightPlanWindow*>(win);
}


void UIManager::Update(float deltaTime) {
    // Update windows from bottom to top (or any order)
    for (auto& window : windows) {
        if (window->visible) {
            window->Update(deltaTime);
        }
    }
}

void UIManager::Render() {
    if (!renderer) return;
    renderer->BeginFrame(screenWidth, screenHeight);

    for (auto& window : windows) {
        if (window->visible) {
            if (window->dirty) {
                window->Measure();
                window->Arrange(window->bounds);
            }
            window->Draw(*renderer, 1.0f); // Pass initial full opacity
        }
    }
    renderer->EndFrame();
}

void UIManager::SetFocus(Widget* widget) {
    if (focusedWidget == widget) return;

    Widget* oldFocusedWidget = focusedWidget; // Store old focus
    focusedWidget = widget;                   // Tentatively set new focus

    if (oldFocusedWidget) {
        oldFocusedWidget->OnFocusLost();
    }

    if (focusedWidget) {
        focusedWidget->OnFocusGained();

        // If the new focused widget is in a window that's not on top, bring it to front
        Widget* topLevelContainer = focusedWidget;
        WindowWidget* containingWindow = nullptr;
        while (topLevelContainer) {
            if ((containingWindow = dynamic_cast<WindowWidget*>(topLevelContainer))) {
                break;
            }
            topLevelContainer = topLevelContainer->parent;
        }

        if (containingWindow) {
            // Pass a flag or check if the window is already the target of BringWindowToFront
            // to avoid it re-requesting focus on itself if the actual target is a child.
            // For simplicity, we can check if the window is already on top.
            // If it's not the last window in the 'windows' vector, then bring it to front.
            if (!windows.empty() && windows.back().get() != containingWindow) {
                BringWindowToFront(containingWindow);
                // CRITICAL: After BringWindowToFront, the UIManager's 'focusedWidget' might have been
                // changed by BringWindowToFront if it called RequestFocus on the window.
                // We need to re-assert the originally intended widget.
                // However, a better fix is in BringWindowToFront.
            }
        }
    }
}

Widget* UIManager::WidgetAtScreenPoint(const Point& screenPos, Widget* context) {
    if (!context || !context->visible) return nullptr;

    // If context is a container, check its children first (in reverse drawing order)
    if (dynamic_cast<ContainerWidget*>(context)) {
        Rect contextScreenBounds = context->GetScreenBounds(); // Absolute screen bounds of the container
        Point contextClientAreaScreenOrigin = {
            contextScreenBounds.x + context->style.padding.left,
            contextScreenBounds.y + context->style.padding.top
        };

        for (auto it = context->children.rbegin(); it != context->children.rend(); ++it) {
            Widget* child = it->get();
            if (child && child->visible) {
                // child->bounds are relative to the parent container's client area (0,0 for the layout)
                Rect childAbsoluteScreenBounds = {
                    contextClientAreaScreenOrigin.x + child->bounds.x, // Correct: child's relative X + parent's client area screen X
                    contextClientAreaScreenOrigin.y + child->bounds.y, // Correct: child's relative Y + parent's client area screen Y
                    child->bounds.width,
                    child->bounds.height
                };

                if (childAbsoluteScreenBounds.Contains(screenPos)) { // Hit-test child using absolute screen coordinates
                    Widget* foundInGrandChild = WidgetAtScreenPoint(screenPos, child); // Recurse with original screenPos
                    return foundInGrandChild ? foundInGrandChild : child;
                }
            }
        }
    }

    // If no child handled it (or not a container), or if point is not in any child,
    // check if the point is within the context itself using its absolute screen bounds.
    if (context->GetScreenBounds().Contains(screenPos)) {
        return context;
    }

    return nullptr;
}

void UIManager::SetCapture(Widget* widget) {
    capturedWidget = widget;
    ::SetCapture(GetActiveWindow()); // Win32 mouse capture
}

void UIManager::ReleaseCapture() {
    capturedWidget = nullptr;
    ::ReleaseCapture(); // Win32 mouse capture
}

void UIManager::MarkAllWindowsDirty() {
    for (auto& window : windows) {
        if (window) {
            window->MarkDirty(true); // Mark self and children
        }
    }
}