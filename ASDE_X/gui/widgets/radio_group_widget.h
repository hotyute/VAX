#pragma once
#include "container_widget.h"
#include <functional>

class RadioButtonWidget; // Forward declaration

class RadioGroupWidget : public ContainerWidget {
public:
    std::function<void(RadioGroupWidget*, RadioButtonWidget* /*selectedButton*/)> OnSelectionChanged;

    RadioGroupWidget(std::unique_ptr<LayoutStrategy> layout);

    void AddRadioButton(std::unique_ptr<RadioButtonWidget> radioButton);
    void OnRadioButtonSelected(RadioButtonWidget* selected); // Called by RadioButtonWidget

    RadioButtonWidget* GetSelectedButton() const;

private:
    RadioButtonWidget* currentSelected = nullptr;
};