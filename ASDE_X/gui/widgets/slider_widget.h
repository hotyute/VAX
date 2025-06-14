#pragma once
#include "widget.h"
#include <functional> // For std::function

class SliderWidget : public Widget {
public:
    std::function<void(SliderWidget*, float)> OnValueChanged;

    SliderWidget(float initialValue = 0.5f, float minValue = 0.0f, float maxValue = 1.0f);

    void SetValue(float newValue, bool fireEvent = true);
    float GetValue() const { return currentValue; }

    void SetMinValue(float minVal);
    float GetMinValue() const { return minValue; }
    void SetMaxValue(float maxVal);
    float GetMaxValue() const { return maxValue; }

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;
    // Arrange is default from Widget

private:
    void UpdateValueFromMousePosition(const Point& localMousePos);
    Rect GetThumbRect() const; // Calculates thumb position based on currentValue

    float currentValue;
    float minValue;
    float maxValue;

    bool isDraggingThumb = false;
    int trackThickness = 8;    // Pixel thickness of the slider track
    int thumbWidth = 12;       // Pixel width of the thumb (for horizontal)
    int thumbHeight = 20;      // Pixel height of the thumb (for vertical)
    // For horizontal, thumbHeight is trackHeight, thumbWidth is specified.
    // For vertical, thumbWidth is trackWidth, thumbHeight is specified.
};