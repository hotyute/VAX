#pragma once
#include "../widgets/window_widget.h" // Adjust path as needed

class LabelWidget;
class SliderWidget;

class SettingsWindow : public WindowWidget {
public:
    SettingsWindow();
    ~SettingsWindow();

    // Callback for when opacity changes
    static std::function<void(float)> OnGlobalOpacityChanged;
    static float GetGlobalWindowOpacity();
    static void SetGlobalWindowOpacity(float opacity);


private:
    void OnOpacitySliderChanged(SliderWidget* slider, float newValue);

    LabelWidget* opacityLabel;
    SliderWidget* opacitySlider;

    static float globalWindowOpacity; // Shared across all windows of this type or managed globally
};