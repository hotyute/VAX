#pragma once
#include "widget.h"

class ButtonWidget : public Widget {
public:
    std::string text;
    // Icon support can be added later

    ButtonWidget(const std::string& text = "Button");
    void SetText(const std::string& newText);

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;

private:
    bool isPressed = false;
};