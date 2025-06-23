#pragma once
#include "../widgets/window_widget.h" // Adjust path as needed
#include <functional> // For std::function

class LabelWidget;
class SliderWidget;
class CheckBoxWidget; // <<< ADDED FORWARD DECLARATION

class SettingsWindow : public WindowWidget {
public:
    SettingsWindow();
    ~SettingsWindow();

    // Callback for when opacity changes
    static std::function<void(float)> OnGlobalOpacityChanged;
    static float GetGlobalWindowOpacity();
    static void SetGlobalWindowOpacity(float opacity);

    // Always On Top feature
    static std::function<void(bool)> OnGlobalAlwaysOnTopChanged; // Optional callback
    static bool GetGlobalAlwaysOnTop();
    static void SetGlobalAlwaysOnTop(bool active);


    void OnShowOrBringToFront() override; // <<< MODIFIED: Will load state

private:
    void OnOpacitySliderChanged(SliderWidget* slider, float newValue);
    void OnAlwaysOnTopToggled(CheckBoxWidget* checkbox, bool isChecked); // <<< ADDED

    LabelWidget* opacityLabel;
    SliderWidget* opacitySlider;

    LabelWidget* alwaysOnTopLabel;      // <<< ADDED
    CheckBoxWidget* alwaysOnTopCheckbox; // <<< ADDED

    static float globalWindowOpacity;
    static bool globalAlwaysOnTop;    // <<< ADDED
};