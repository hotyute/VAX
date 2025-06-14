#include "radio_group_widget.h"
#include "radio_button_widget.h" // Full include

RadioGroupWidget::RadioGroupWidget(std::unique_ptr<LayoutStrategy> layout)
    : ContainerWidget(std::move(layout)) {}

void RadioGroupWidget::AddRadioButton(std::unique_ptr<RadioButtonWidget> radioButton) {
    if (radioButton) {
        radioButton->group = this;
        ContainerWidget::AddChild(std::move(radioButton));
    }
}

void RadioGroupWidget::OnRadioButtonSelected(RadioButtonWidget* selected) {
    if (currentSelected == selected) return; // No change

    if (currentSelected) {
        currentSelected->isChecked = false; // Manually uncheck, don't call SetChecked to avoid loop
        currentSelected->MarkDirty(false);
    }
    currentSelected = selected;
    // 'selected' has already set its own isChecked to true via its SetChecked method
    // currentSelected->isChecked = true; // Already done by the button itself
    // currentSelected->MarkDirty();
    
    if (OnSelectionChanged) {
        OnSelectionChanged(this, currentSelected);
    }
    MarkDirty(); // Group might need redraw if it has own visuals
}

RadioButtonWidget* RadioGroupWidget::GetSelectedButton() const {
    return currentSelected;
}