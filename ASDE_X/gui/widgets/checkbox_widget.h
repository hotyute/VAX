#pragma once
#include "widget.h"
#include <string>
#include <functional>

class CheckBoxWidget : public Widget {
public:
    std::string label;
    bool isChecked = false;

    std::function<void(CheckBoxWidget*, bool /*isChecked*/)> OnCheckChanged;

    CheckBoxWidget(const std::string& labelText = "");

    void SetChecked(bool checkedState, bool fireEvent = true);
    bool GetChecked() const { return isChecked; }

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;
};