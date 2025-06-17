#pragma once
#include "../widgets/window_widget.h" // Adjust path as needed
#include <vector>
#include <string>

// Forward declarations for UI widgets
class LabelWidget;
class ButtonWidget;
class RadioButtonWidget; // Using our new RadioButtonWidget
class InputFieldWidget;
class ContainerWidget; // For the edit section

// Internal data structure to hold data and UI pointers for each comms line
struct CommsLineElements {
    std::string id; // Unique identifier, e.g., "LINE_0", "LINE_1"
    std::string positionName;
    std::string frequency;
    bool isPrimary = false;
    bool tx_checked = false;
    bool rx_checked = false;
    bool hdst_checked = false;
    bool spkr_checked = false;

    // UI Element Pointers for this line
    RadioButtonWidget* primRadio = nullptr;
    ButtonWidget* nameButton = nullptr; // Will display "Position (Freq)"
    RadioButtonWidget* txRadio = nullptr;
    RadioButtonWidget* rxRadio = nullptr;
    RadioButtonWidget* hdstRadio = nullptr;
    RadioButtonWidget* spkrRadio = nullptr;

    ContainerWidget* uiRowContainer = nullptr; // The HBox holding this line's widgets
};

class CommsWindowWidget : public WindowWidget {
public:
    CommsWindowWidget();
    ~CommsWindowWidget();

    void LoadState(); // Loads state from application (e.g., global comms_line0/1 or USER->userdata)
    void SaveState(); // Saves state back to application

    void HandleEvent(UIEvent& event) override; // For Escape key to cancel edit mode etc.

    void OnShowOrBringToFront() override {
        WindowWidget::OnShowOrBringToFront(); // Call base if needed
        LoadState();
        MarkDirty(true);
    }


private:
    void SetupUI();
    ContainerWidget* CreateHeaderRow();
    void CreateCommsLineUIRow(int lineIndex, ContainerWidget* parentLayout);
    void UpdateCommsLineUIRow(int lineIndex); // Updates UI from CommsLineElements[lineIndex]

    void ToggleEditSection(int lineIndex = -1); // -1 to hide, or index of line to edit

    // Callbacks for UI interactions
    void OnPrimRadioClicked(Widget* sender, int lineIndex);
    void OnNameButtonClicked(Widget* sender, int lineIndex);
    void OnRadioToggled(Widget* sender, int lineIndex, int radioType); // radioType for TX, RX, HDST, SPKR

    void OnSaveEditClicked(Widget* sender);
    void OnClearEditClicked(Widget* sender);
    void OnCancelEditClicked(Widget* sender);

    // Constants for radio types passed to OnRadioToggled
    enum CommRadioType { TX, RX, HDST, SPKR };

    // Data for the comms lines
    std::vector<CommsLineElements> commsLines;
    static const int NUM_DISPLAYED_LINES = 2; // For comms_line0 and comms_line1

    // UI Elements for the main panel (static parts)
    // Header labels are created and added directly in SetupUI

    // UI Elements for the pop-up/expanded edit section
    ContainerWidget* editSectionContainer = nullptr;
    LabelWidget* editPositionLabel = nullptr;
    InputFieldWidget* editPositionInput = nullptr;
    LabelWidget* editFrequencyLabel = nullptr;
    InputFieldWidget* editFrequencyInput = nullptr;
    ButtonWidget* editSaveButton = nullptr;
    ButtonWidget* editClearButton = nullptr;
    ButtonWidget* editCancelButton = nullptr;

    bool isEditSectionVisible = false;
    int currentEditingLineIndex = -1; // Index into commsLines being edited
    int currentPrimaryLineIndex = -1; // Index into commsLines for current PRIM
};