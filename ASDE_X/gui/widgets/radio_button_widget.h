#pragma once
#include "widget.h"
#include <string>

class RadioGroupWidget; // Forward declaration

class RadioButtonWidget : public Widget {
public:
    std::string label;
    bool isChecked = false;
    RadioGroupWidget* group = nullptr; // Pointer to its group

    RadioButtonWidget(const std::string& labelText = "");

    void SetChecked(bool checkedState); // This should interact with the group

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;
};