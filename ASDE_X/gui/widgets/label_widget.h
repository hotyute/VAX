#pragma once
#include "widget.h"

class LabelWidget : public Widget {
public:
    std::string text;
    // Add text alignment options if needed

    LabelWidget(const std::string& text = "");
    void SetText(const std::string& newText);
    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
};