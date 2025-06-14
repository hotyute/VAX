#pragma once
#include "ui_common.h"
#include "style.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>

// Forward declaration
class UIRenderer;
class UIManager; // For focus management access

class Widget {
public:
    Widget* parent = nullptr;
    std::vector<std::unique_ptr<Widget>> children; // Only for ContainerWidgets conceptually, but useful here for polymorphism

    Rect bounds;          // Actual screen coordinates and size, set by layout
    Size preferredSize;   // Desired size, calculated by Measure()
    Size minSize = {0,0};
    Size maxSize = {10000, 10000}; // Effectively infinity

    bool visible = true;
    bool enabled = true;
    bool focusable = false;
    bool hasFocus = false;
    bool dirty = true; // Needs redraw

    std::string id;
    Style style;

    // Event Callbacks
    std::function<void(Widget*, const UIEvent&)> OnEvent; // Generic event handler
    std::function<void(Widget*)> OnClick;
    std::function<void(Widget*, const KeyboardEvent&)> OnKeyPress;
    std::function<void(Widget*, const KeyboardEvent&)> OnKeyRelease;
    std::function<void(Widget*, const KeyboardEvent&)> OnCharInput;
    std::function<void(Widget*, const MouseEvent&)> OnMouseMove;
    std::function<void(Widget*, const MouseEvent&)> OnMouseDown;
    std::function<void(Widget*, const MouseEvent&)> OnMouseUp;
    std::function<void(Widget*, const MouseEvent&)> OnMouseEnter;
    std::function<void(Widget*, const MouseEvent&)> OnMouseLeave;
    std::function<void(Widget*, const MouseEvent&)> OnMouseScroll;


    Widget();
    virtual ~Widget() = default;

    virtual void HandleEvent(UIEvent& event);
    virtual void Update(float deltaTime); // For animations or time-based updates
    virtual void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f);
    virtual Size Measure(); // Calculate and return preferredSize
    virtual void Arrange(const Rect& finalRect); // Set own bounds and arrange children

    virtual void OnFocusGained();
    virtual void OnFocusLost();

    virtual void AddChild(std::unique_ptr<Widget> child); // Primarily for ContainerWidget
    Widget* GetChildById(const std::string& childId);

    Rect GetScreenBounds() const; // Calculates absolute screen coordinates
    Rect GetClientBounds() const; // Bounds minus padding

    void SetVisible(bool v);
    void SetEnabled(bool e);
    void RequestFocus();
    void MarkDirty(bool includeChildren = true);

protected:
    // Helper to dispatch event to children; returns true if a child handled it
    virtual bool DispatchEventToChildren(UIEvent& event);
};