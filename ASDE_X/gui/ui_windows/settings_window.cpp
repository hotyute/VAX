#include <algorithm> // Needed for std::clamp

#include "settings_window.h"
#include "../widgets/label_widget.h"
#include "../widgets/slider_widget.h"
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h"     // For marking windows dirty

// Initialize static members
float SettingsWindow::globalWindowOpacity = 0.8f; // Default opacity
std::function<void(float)> SettingsWindow::OnGlobalOpacityChanged;


SettingsWindow::SettingsWindow() : WindowWidget("Settings") {
    minSize = { 250, 120 };
    // preferredSize = {300, 150}; // Set if you want a larger default

    ContainerWidget* ca = GetContentArea();
    if (ca) {
        auto layout = std::make_unique<VerticalStackLayout>();
        layout->spacing = 5;
        layout->padding = { 10, 10, 10, 10 };
        ca->SetLayout(std::move(layout));
    }

    opacityLabel = new LabelWidget("Window Opacity:");
    contentArea->AddChild(std::unique_ptr<LabelWidget>(opacityLabel));

    opacitySlider = new SliderWidget(globalWindowOpacity, 0.1f, 1.0f); // Min opacity 0.1, Max 1.0
    opacitySlider->minSize = { 150, 20 }; // Give slider a decent min width
    opacitySlider->preferredSize = { 200, 20 };
    opacitySlider->OnValueChanged = std::bind(&SettingsWindow::OnOpacitySliderChanged, this,
        std::placeholders::_1, std::placeholders::_2);
    contentArea->AddChild(std::unique_ptr<SliderWidget>(opacitySlider));
}

SettingsWindow::~SettingsWindow() {
    // Pointers are managed by unique_ptr in ContainerWidget's children vector
}

void SettingsWindow::OnOpacitySliderChanged(SliderWidget* slider, float newValue) {
    SetGlobalWindowOpacity(newValue);
    if (OnGlobalOpacityChanged) {
        OnGlobalOpacityChanged(newValue);
    }
    // Mark all windows dirty so they redraw with new opacity
    UIManager::Instance().MarkAllWindowsDirty();
}

float SettingsWindow::GetGlobalWindowOpacity() {
    return globalWindowOpacity;
}

void SettingsWindow::SetGlobalWindowOpacity(float opacity) {
    globalWindowOpacity = std::clamp(opacity, 0.0f, 1.0f);
}