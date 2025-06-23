#include <algorithm> // Needed for std::clamp
#include <windows.h> // For SetWindowPos, HWND_TOPMOST, HWND_NOTOPMOST, SWP_NOMOVE, SWP_NOSIZE

#include "settings_window.h"
#include "../widgets/label_widget.h"
#include "../widgets/slider_widget.h"
#include "../widgets/checkbox_widget.h" // <<< ADDED
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h"     
#include "../widgets/font_keys.h"      // For FONT_KEY_UI_LABEL

#include "../../main.h" // For ::hWnd (main application window handle)
#include "../../usermanager.h" // For USER (to save/load setting)


// Initialize static members
float SettingsWindow::globalWindowOpacity = 0.8f;
bool SettingsWindow::globalAlwaysOnTop = false;    // <<< ADDED
std::function<void(float)> SettingsWindow::OnGlobalOpacityChanged;
std::function<void(bool)> SettingsWindow::OnGlobalAlwaysOnTopChanged; // <<< ADDED

SettingsWindow::SettingsWindow() : WindowWidget("Settings") {
    minSize = { 250, 150 }; // Increased height for new checkbox
    // preferredSize = {300, 180}; 

    ContainerWidget* ca = GetContentArea();
    if (ca) {
        auto layout = std::make_unique<VerticalStackLayout>();
        layout->spacing = 8; // Increased spacing a bit
        layout->padding = { 10, 10, 10, 10 };
        ca->SetLayout(std::move(layout));
    }

    // Opacity Controls
    auto opacityRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(opacityRow->layout.get())->spacing = 5;
    opacityLabel = new LabelWidget("Window Opacity:");
    opacityLabel->preferredSize = { 100, 20 }; opacityLabel->minSize = { 100, 20 };
    opacityLabel->style.fontKey = FONT_KEY_UI_LABEL;
    opacityRow->AddChild(std::unique_ptr<LabelWidget>(opacityLabel));

    opacitySlider = new SliderWidget(globalWindowOpacity, 0.1f, 1.0f);
    opacitySlider->minSize = { 100, 20 };
    opacitySlider->preferredSize = { 120, 20 };
    opacitySlider->OnValueChanged = std::bind(&SettingsWindow::OnOpacitySliderChanged, this,
        std::placeholders::_1, std::placeholders::_2);
    opacityRow->AddChild(std::unique_ptr<SliderWidget>(opacitySlider));
    contentArea->AddChild(std::move(opacityRow));


    // Always On Top Controls
    auto alwaysOnTopRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(alwaysOnTopRow->layout.get())->spacing = 5;
    alwaysOnTopLabel = new LabelWidget("Always On Top:");
    alwaysOnTopLabel->preferredSize = { 100, 20 }; alwaysOnTopLabel->minSize = { 100, 20 };
    alwaysOnTopLabel->style.fontKey = FONT_KEY_UI_LABEL;
    alwaysOnTopRow->AddChild(std::unique_ptr<LabelWidget>(alwaysOnTopLabel));

    alwaysOnTopCheckbox = new CheckBoxWidget(""); // Label is separate
    alwaysOnTopCheckbox->isChecked = globalAlwaysOnTop; // Initialize from global state
    alwaysOnTopCheckbox->minSize = { 20,20 }; // Just the box
    alwaysOnTopCheckbox->preferredSize = { 20,20 };
    alwaysOnTopCheckbox->OnCheckChanged = std::bind(&SettingsWindow::OnAlwaysOnTopToggled, this,
        std::placeholders::_1, std::placeholders::_2);
    alwaysOnTopRow->AddChild(std::unique_ptr<CheckBoxWidget>(alwaysOnTopCheckbox));
    contentArea->AddChild(std::move(alwaysOnTopRow));
}

SettingsWindow::~SettingsWindow() {
}

void SettingsWindow::OnShowOrBringToFront() {
    WindowWidget::OnShowOrBringToFront();
    // Load the current global states into the UI elements
    if (opacitySlider) {
        opacitySlider->SetValue(globalWindowOpacity, false); // false = don't fire event
    }
    if (alwaysOnTopCheckbox) {
        alwaysOnTopCheckbox->SetChecked(globalAlwaysOnTop, false); // false = don't fire event
    }
    MarkDirty(true);
}


void SettingsWindow::OnOpacitySliderChanged(SliderWidget* slider, float newValue) {
    SetGlobalWindowOpacity(newValue);
    if (OnGlobalOpacityChanged) {
        OnGlobalOpacityChanged(newValue);
    }
    UIManager::Instance().MarkAllWindowsDirty();
}

float SettingsWindow::GetGlobalWindowOpacity() {
    return globalWindowOpacity;
}

void SettingsWindow::SetGlobalWindowOpacity(float opacity) {
    globalWindowOpacity = std::clamp(opacity, 0.0f, 1.0f);
    if (USER) USER->userdata.window_opacity = globalWindowOpacity; // Save to user data
}


// --- Always On Top Methods ---
bool SettingsWindow::GetGlobalAlwaysOnTop() {
    return globalAlwaysOnTop;
}

void SettingsWindow::SetGlobalAlwaysOnTop(bool active) {
    if (globalAlwaysOnTop == active) return;

    globalAlwaysOnTop = active;
    if (::IsWindow(::hWnd)) { // Ensure main window handle is valid
        ::SetWindowPos(::hWnd,
            active ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
    }
    if (USER) { // Save to persistent user data if USER object exists
        USER->userdata.alwaysOnTop = globalAlwaysOnTop;
    }

    if (OnGlobalAlwaysOnTopChanged) {
        OnGlobalAlwaysOnTopChanged(active);
    }
}

void SettingsWindow::OnAlwaysOnTopToggled(CheckBoxWidget* checkbox, bool isChecked) {
    SetGlobalAlwaysOnTop(isChecked);
    // The SetGlobalAlwaysOnTop method now handles applying the change and saving.
}