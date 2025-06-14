#pragma once
#include "widget.h"
#include <functional> // For std::function

enum class ScrollBarOrientation {
    Vertical,
    Horizontal
};

class ScrollBarWidget : public Widget {
public:
    ScrollBarOrientation orientation;

    // Properties to define the scrollable content
    int contentSize = 1000; // Total size of the content being scrolled (e.g., pixels high or wide)
    int viewportSize = 100; // Size of the visible area of the content (e.g., height/width of the TextBox client area)
    int currentValue = 0;   // Current scroll position (top/left of the visible content within contentSize)
    int stepSize = 20;      // How much to scroll when clicking track (page up/down often viewportSize)
                            // Or for arrow buttons (often line height/char width)
    int arrowButtonSize = 16;
    bool showArrowButtons = true;

    std::function<void(ScrollBarWidget*, int /*newValue*/)> OnScroll;

    ScrollBarWidget(ScrollBarOrientation orientation = ScrollBarOrientation::Vertical);

    void SetParameters(int newContentSize, int newViewportSize, int newValue);
    void SetValue(int newValue, bool fireEvent = true);
    int GetValue() const { return currentValue; }

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;
    void Arrange(const Rect& finalRect) override;


private:
    Rect GetThumbRect() const; // Calculates the thumb's position and size
    void UpdateThumbFromValue();
    void UpdateValueFromThumbPosition(int thumbPixelPosition); // thumbPixelPosition is mouse Y for vertical, X for horizontal

    bool isDraggingThumb = false;
    Point dragStartMousePos; // Mouse position when dragging started
    int dragStartValue;    // Scrollbar value when dragging started

    Point dragThumbClickOffset; // Offset of the mouse click *within the thumb* when dragging started

    int thumbMinSize = 10; // Minimum pixel size for the thumb
    int trackThickness = 16; // Default thickness of the scrollbar track

    Rect GetUpArrowRect() const;   // For vertical scrollbar
    Rect GetDownArrowRect() const; // For vertical scrollbar
    Rect GetLeftArrowRect() const; // For horizontal scrollbar
    Rect GetRightArrowRect() const; // For horizontal scrollbar
    Rect GetTrackAreaRect() const; // The area available for the thumb, between arrow buttons
};